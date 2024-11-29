#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_EVENTS_PER_PAGE 3

// Enum para identificar o tipo de evento
typedef enum { UFC_EVENT, FIGHT_NIGHT_EVENT } EventType;

// Estrutura unificada para eventos
struct genericEvent {
    EventType type;
    struct date {
        int dia, mes, ano, hora;
    } dt;
    char local[30];
    union {
        struct {
            char lutador1[11]; // +1 para o caractere nulo
            char lutador2[11];
        } ufc;
        char descricao[31];
    } details;

};
// Controle de custos
float estimativaGanhos = 0;
    union{
        // UFC tem pay-per-view
        struct{
            float custoIngressoUFC; 
            float ppv;
        }ufcpay;

        float custoIngressoFGHT;
    } arrecadao;

// Estrutura para armazenar dados da visualização filtrada
typedef struct {
    struct genericEvent *events;
    int numEvents;
    int currentPage;
    int totalPages;
    GtkWidget *events_view;
} FilteredEventsData;

// Variáveis globais
struct genericEvent *allEvents = NULL;
int numAllEvents = 0;

// Índice de paginação
int currentPage = 0;
int totalPages = 0;

// Widgets globais (Widgets == elementos de tela)
GtkWidget *window;
GtkWidget *main_menu;
GtkWidget *events_view;

// Prototipação de funções
void load_events();
void save_events();
void show_main_menu();
void on_menu_option_selected(GtkWidget *widget, gpointer data);
void show_events_view();
void on_next_page(GtkWidget *widget, gpointer data);
void on_prev_page(GtkWidget *widget, gpointer data);
void on_back_to_menu(GtkWidget *widget, gpointer data);
void on_window_destroy();
void show_error_message(const char *message);
void show_info_message(const char *message);
void show_add_event_form();
void on_event_type_changed(GtkComboBox *widget, gpointer data);
void show_events_by_date();
void show_events_by_location();
void show_delete_event();
int compare_events_by_date(const void *a, const void *b);
time_t event_to_timestamp(struct genericEvent *event);
gboolean validate_numeric_input(const gchar *input);
void show_filtered_events_view(struct genericEvent *events, int numEvents);
void on_filtered_next_page(GtkWidget *widget, gpointer user_data);
void on_filtered_prev_page(GtkWidget *widget, gpointer user_data);
void on_filtered_back_to_menu(GtkWidget *widget, gpointer user_data);
void refresh_filtered_events_view(FilteredEventsData *data);

int main(int argc, char *argv[]) { //Não é necessário...
    gtk_init(&argc, &argv); // ...mas permite mudar tema de tela, padrão de exibição, etc. Se não passar (ou passar errado), usa-se o padrão.
    /*
        E olha que legal: a gtk suporta o uso das CSS.
        Pode-se definir um arquivo "azul-bebe.css" e passá-lo como parâmetro. É um ganho de controle surreal do layout.
    */


    // Carregar eventos
    load_events();

    // Criar janela principal
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL); // GTK_WINDOW_TOPLEVEL = 0 (enum type)
    /*
        Funcionamento da gtk_window_new: 
        Protótipo da função:
        GtkWidget *gtk_window_new(GtkWindowType type); lembrando, em gtk, tudo é Widget (componente de tela)
        O typedef enum só tem GTK_WINDOW_TOPLEVEL ou GTK_WINDOW_POPUP, o primeiro é janela com título e o segundo sem título (bobo mesmo);
        gtk_window_new se responsabiliza por criar a janela principal, botão de maximizar, minimizar e fechar;
        O importante aqui é que a gtk cria os próprios ponteiros e cuida de descartá-los toda vez que um Widget é destruído (ex: fechar a janela);
    
    */
   
    gtk_window_set_title(GTK_WINDOW(window), "Gerenciador de Eventos UFC"); 
    /*
        O código aqui já é bem autoexplicativo e não estamos interessados nas implementações internas da gtk (como ela cria a janela, etc, etc);
        O relevante aqui é o macro GTK_WINDOW (que define um casting). Como dito, a gtk irá cuidar dos próprios ponteiros. E os ponteiros gtk são particulares:
        são do tipo GtkWdiget*, GTK_WINDOW apenas converte "window" (que é um ponteiro regular de C) para um ponteiro Gtk
        Depois disso a responsabilidade é da GTK.
    */
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 600);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), NULL); // Event Handler
    // g_signal_connect é similar ao ActionListener do Java. Este em específico está conectado à janela principal e pode receber o sinal de "delete-event",
    // conforme o parâmetro "destroy" informa. Interessante notar que isso quebra, em parte, a programação estruturada típica de C, pois a programação baseada em eventos é assíncrona.

    // Mostrar menu principal
    show_main_menu();

    gtk_widget_show_all(window);
    gtk_main(); // Inicia o loop de eventos, obrigatório e padrão.

    // Salvar eventos antes de sair
    save_events();

    // Liberar memória
    free(allEvents);

    return 0;
}

// Função para comparar eventos por data
int compare_events_by_date(const void *a, const void *b) {
    struct genericEvent *eventA = (struct genericEvent *)a;
    struct genericEvent *eventB = (struct genericEvent *)b;

    time_t timestampA = event_to_timestamp(eventA); 
    time_t timestampB = event_to_timestamp(eventB);

    if (timestampA < timestampB) return -1;
    else if (timestampA > timestampB) return 1;
    else return 0;

}

// Função para converter um evento em timestamp, desacopla o genericEvent acima
time_t event_to_timestamp(struct genericEvent *event) {
    struct tm event_time = {0}; // tm tem várias definições, de segundos até horário de verão, aqui só usa-se 4 delas e os inicializamos em 0, {0} é sugar syntax
    event_time.tm_mday = event->dt.dia;
    event_time.tm_mon = event->dt.mes - 1; // pra tm jan = 0, mas para nós jan = 1
    event_time.tm_year = event->dt.ano - 1900; //tmyear => nº de anos desde 1900 
    event_time.tm_hour = event->dt.hora;
    return mktime(&event_time); // mktime converte para segundos os dados da struct, segundos desde 1900
}

// Função para validar entradas numéricas
gboolean validate_numeric_input(const gchar *input) {
    for (int i = 0; i < strlen(input); i++) {
        if (!g_ascii_isdigit(input[i])) {
            return FALSE;
        }
    }
    return TRUE;
}

// Função para carregar eventos dos arquivos (IMPERATIVO)
void load_events() {
    allEvents = NULL;
    numAllEvents = 0;

    // Carregar eventos UFC
    FILE *ufcFile = fopen("UFC.txt", "r");
    if (ufcFile != NULL) {
        char line[256];
        while (fgets(line, sizeof(line), ufcFile)) {
            line[strcspn(line, "\n")] = '\0';
            char *token;
            struct genericEvent newEvent;
            newEvent.type = UFC_EVENT; // TIPO

            token = strtok(line, ","); // strtok guarda o ponteiro local por meio de uma variável estática
            if (token == NULL) continue;
            newEvent.dt.dia = atoi(token); //dia OBS: se atoi falha, retorna 0

            token = strtok(NULL, ","); // se p1 = NULL, strtok usa o membro estático
            if (token == NULL) continue;
            newEvent.dt.mes = atoi(token); // Mês

            token = strtok(NULL, ",");
            if (token == NULL) continue;
            newEvent.dt.ano = atoi(token); // Ano

            token = strtok(NULL, ",");
            if (token == NULL) continue;
            newEvent.dt.hora = atoi(token); // Hora

            token = strtok(NULL, ",");
            if (token == NULL) continue;
            strncpy(newEvent.local, token, 29); // Local
            newEvent.local[29] = '\0'; //Importante: strncpy não garante '\0' se a string passar do tamanho, isso é uma medida de segurança

            token = strtok(NULL, ","); // Agora vamos acessar nossa Union interna, depois sua subestrutura
            if (token == NULL) continue;
            strncpy(newEvent.details.ufc.lutador1, token, 10); // Lutador 1
            newEvent.details.ufc.lutador1[10] = '\0';

            token = strtok(NULL, ",");
            if (token == NULL) continue;
            strncpy(newEvent.details.ufc.lutador2, token, 10); // Lutador 2
            newEvent.details.ufc.lutador2[10] = '\0';
            // 
            struct genericEvent *newEvents = realloc(allEvents, (numAllEvents + 1) * sizeof(struct genericEvent));
            if (newEvents == NULL) {
                printf("Erro de alocação de memória\n");
                fclose(ufcFile);
                return;
            }
            allEvents = newEvents; // Se a operação foi possível, allEvents aponta para o começo do vetor heterogêneo

            allEvents[numAllEvents] = newEvent;
            numAllEvents++;
        }
        fclose(ufcFile);

    } else {
        // Arquivo não existe, criar vazio
        ufcFile = fopen("UFC.txt", "w");
        fclose(ufcFile);
    }

    // Carregar eventos Fight Night
    FILE *fnFile = fopen("FightNight.txt", "r");
    if (fnFile != NULL) {
        char line[256];
        while (fgets(line, sizeof(line), fnFile)) {
            line[strcspn(line, "\n")] = '\0';
            char *token;
            struct genericEvent newEvent;
            newEvent.type = FIGHT_NIGHT_EVENT;

            token = strtok(line, ",");
            if (token == NULL) continue;
            newEvent.dt.dia = atoi(token);

            token = strtok(NULL, ",");
            if (token == NULL) continue;
            newEvent.dt.mes = atoi(token);

            token = strtok(NULL, ",");
            if (token == NULL) continue;
            newEvent.dt.ano = atoi(token);

            token = strtok(NULL, ",");
            if (token == NULL) continue;
            newEvent.dt.hora = atoi(token);

            token = strtok(NULL, ",");
            if (token == NULL) continue;
            strncpy(newEvent.local, token, 29);
            newEvent.local[29] = '\0';

            token = strtok(NULL, ",");
            if (token == NULL) continue;
            strncpy(newEvent.details.descricao, token, 30);
            newEvent.details.descricao[30] = '\0';

            struct genericEvent *newEvents = realloc(allEvents, (numAllEvents + 1) * sizeof(struct genericEvent));
            if (newEvents == NULL) {
                printf("Erro de alocação de memória\n");
                fclose(fnFile);
                return;
            }
            allEvents = newEvents;

            allEvents[numAllEvents] = newEvent;
            numAllEvents++;
        }
        fclose(fnFile);
    } else {
        // Arquivo não existe, criar vazio
        fnFile = fopen("FightNight.txt", "w");
        fclose(fnFile);
    }

    // Ordenar os eventos por data (ponteiroComeçoArray, numElemArray, tamanhoCadaElem, funcDeComparacao);
    qsort(allEvents, numAllEvents, sizeof(struct genericEvent), compare_events_by_date);
}

// Função para exibir o menu principal
void show_main_menu() {
    if (events_view != NULL) {
        gtk_widget_destroy(events_view);
        /*
            Essa é a primeira aparição da gtk_widget_destroy, que faz parte da GTK. 
            É a função mais importante, no contexto da disciplina, que está sendo usada, pois está associada
            ao manuseio da memória.
            A gtk_widget_destroy emite o sinal "destroy". Desse modo, todos os retornos de função (callbacks) são avisados
            de que seu widget está prestes a ser destruído. Isso permite que nós façamos alguma intervenção por conta, por exemplo.
            Além disso, a GTK, por parte dela, remove o widget da sua hierarquia (brevemente, o topo da hierarquia é sempre o ponteiro/objeto "window",
            uma vez que é em "window" que todos os Widgets são conectados. Em Java, chamavámos isso de JFrame. Se a window é destruída a aplicação deve encerrar GtkWindow -> GtkBox -> GtkButton)
            de widgets e libera a memória associada a ele (isso inclui membros inferiores da hierarquia). Torna as referências ao widget são invalidadas (NULL).
            Neste momento, é a primeira vez que o "OBJETO" events_view é chamado para algo. No momento, ele não contém absolutamente nada, então destruí-lo não afeta o programa.
        
        */
        events_view = NULL; // events_view ainda aponta p/ o objeto anterior, como vamos reusá-lo, estaremos sempre setando para NULL após uma destruição.
    }

    if (main_menu != NULL) { // Ideia análoga a descrita acima.
        gtk_widget_destroy(main_menu);
        main_menu = NULL;
    }

    main_menu = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10); // gtk_box_new(alinhamentoWidgets, paddingeEmPixels), retorna o endereço. Equivale ao JPanel.

    GtkWidget *title = gtk_label_new("Gerenciador de Eventos UFC"); // Título do container criado
    gtk_box_pack_start(GTK_BOX(main_menu), title, FALSE, FALSE, 10); // O título é adicionado ao container main_menu

    // Opções do menu
    const char *options[] = {
        "Mostrar todos os eventos",
        "Adicionar um novo evento",
        "Mostrar eventos por data",
        "Mostrar eventos por local",
        "Deletar um evento",
        "Sair"
    };

    for (int i = 0; i < 6; i++) {
        GtkWidget *button = gtk_button_new_with_label(options[i]); // Cria botão e passa o nome
        gtk_box_pack_start(GTK_BOX(main_menu), button, FALSE, FALSE, 0); // Adiciona ao container mestre (main_menu)
        g_signal_connect(button, "clicked", G_CALLBACK(on_menu_option_selected), GINT_TO_POINTER(i)); // Associa um EventHandler a cada botão, o qual captará sinais se emitidos deles, no caso, de clique. GIN converte o inteiro para void* (porque precisa p/ função, n pq o código precisa)
    }

    gtk_container_add(GTK_CONTAINER(window), main_menu); // Quando o container main_menu é totalmente criado, adicionamo-lo à tela, outrossim adicionamos cada botão ao main_menu.
    gtk_widget_show_all(window); // Cada box criado, como no Swing do Java, é inicialmente transparante. Dizemos, aqui, para mostrá-la.
}

// Função para salvar eventos nos arquivos
void save_events() {
    FILE *outUfcFile = fopen("UFC.txt", "w");
    FILE *outFnFile = fopen("FightNight.txt", "w");

    if (outUfcFile == NULL || outFnFile == NULL) {
        printf("Erro ao abrir arquivos para escrita\n");
        return;
    }

    for (int i = 0; i < numAllEvents; i++) { // Pega cada tipo estruturado do array dinâmico principal
        struct genericEvent *event = &allEvents[i];
        if (event->type == UFC_EVENT) { 
            fprintf(outUfcFile, "%d,%d,%d,%d,%s,%s,%s\n",
                    event->dt.dia, event->dt.mes, event->dt.ano, event->dt.hora,
                    event->local,
                    event->details.ufc.lutador1,
                    event->details.ufc.lutador2);
        } else if (event->type == FIGHT_NIGHT_EVENT) {
            fprintf(outFnFile, "%d,%d,%d,%d,%s,%s\n",
                    event->dt.dia, event->dt.mes, event->dt.ano, event->dt.hora,
                    event->local,
                    event->details.descricao);
        }
    }

    fclose(outUfcFile);
    fclose(outFnFile);
}

// Callback para as opções do menu
void on_menu_option_selected(GtkWidget *widget, gpointer data) {
    int choice = GPOINTER_TO_INT(data); // desfaz o void*, voltando a ser int
    switch (choice) {
        case 0:
            currentPage = 0;
            show_events_view();
            break;
        case 1:
            show_add_event_form();
            break;
        case 2:
            show_events_by_date();
            break;
        case 3:
            show_events_by_location();
            break;
        case 4:
            show_delete_event();
            break;
        case 5:
            gtk_main_quit();
            break;
    }
}

// Função para exibir os eventos (máximo 3 por página)
void show_events_view() { //Daqui em diante boa parte das funções já foi explicada uma vez, logo, não vou tornar a comentá-las.
    if (main_menu != NULL) {
        gtk_widget_destroy(main_menu);
        main_menu = NULL;
    }

    if (events_view != NULL) {
        gtk_widget_destroy(events_view);
    }

    events_view = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    GtkWidget *title = gtk_label_new("Lista de Eventos");
    gtk_box_pack_start(GTK_BOX(events_view), title, FALSE, FALSE, 0);

    GtkWidget *event_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(events_view), event_box, TRUE, TRUE, 0);

    // Calcula o total de páginas (Daria pra ganhar eficiência ao se calcular isso por fora, aqui, toda vez que o Widget é destruído se refaz a conta, mas o ganho é negligenciável)
    int totalEvents = numAllEvents;
    totalPages = (totalEvents + MAX_EVENTS_PER_PAGE - 1) / MAX_EVENTS_PER_PAGE; // Aplicado fator de correção

    // Exibe os eventos da página atual
    int startIndex = currentPage * MAX_EVENTS_PER_PAGE; //0, 3, 6...
    int endIndex = startIndex + MAX_EVENTS_PER_PAGE;//3, 6,...,
    if (endIndex > totalEvents) endIndex = totalEvents;

    for (int i = startIndex; i < endIndex; i++) {
        GtkWidget *event_label;
        struct genericEvent *event = &allEvents[i]; //Não copiamos os dados, apenas apontamos para eles.
        char event_info[256]; //buffer de tamanho seguro

        if (event->type == UFC_EVENT) {
            sprintf(event_info, "UFC - %02d/%02d/%04d %02d:00\nLocal: %s\nLuta: %s vs %s",
                    event->dt.dia, event->dt.mes, event->dt.ano, event->dt.hora,
                    event->local, event->details.ufc.lutador1, event->details.ufc.lutador2);
        } else if (event->type == FIGHT_NIGHT_EVENT) {
            sprintf(event_info, "Fight Night - %02d/%02d/%04d %02d:00\nLocal: %s\nDescrição: %s",
                    event->dt.dia, event->dt.mes, event->dt.ano, event->dt.hora,
                    event->local, event->details.descricao);
        }

        event_label = gtk_label_new(event_info); // Coletada e formatada a string, entregamo-la ao Widget do nosso label
        gtk_box_pack_start(GTK_BOX(event_box), event_label, FALSE, FALSE, 0); // E mandamos ela ser adicionada à tela. 
    }

    // Botões de navegação
    GtkWidget *navigation_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10); // box dos botões
    gtk_box_pack_start(GTK_BOX(events_view), navigation_box, FALSE, FALSE, 0); // box dos botões add ao box

    if (currentPage > 0) {// Se for possível voltar
        GtkWidget *prev_button = gtk_button_new_with_label("<- Anterior"); // cria-se O botão voltar
        gtk_box_pack_start(GTK_BOX(navigation_box), prev_button, TRUE, TRUE, 0); // O botão é add a sua caixa
        g_signal_connect(prev_button, "clicked", G_CALLBACK(on_prev_page), NULL);// Um event Handler captura o sinal se ele for pressionado se sim, a sua função, on_prev_page, meramente manda destruir tudo o que foi construído até então e refazer, só que currentPage++.
    }

    if (currentPage < totalPages - 1) { // Ideia análoga a de cima
        GtkWidget *next_button = gtk_button_new_with_label("Próximo ->");
        gtk_box_pack_start(GTK_BOX(navigation_box), next_button, TRUE, TRUE, 0);
        g_signal_connect(next_button, "clicked", G_CALLBACK(on_next_page), NULL);
    }

    // Botão para voltar ao menu principal
    GtkWidget *back_button = gtk_button_new_with_label("Voltar ao Menu");
    gtk_box_pack_start(GTK_BOX(events_view), back_button, FALSE, FALSE, 0);
    g_signal_connect(back_button, "clicked", G_CALLBACK(on_back_to_menu), NULL); // Ideia análoga ao descrito acima

    gtk_container_add(GTK_CONTAINER(window), events_view); // Adiciona-se a página criada à janela principal;
    gtk_widget_show_all(window); // Manda-se mostrar a janela principal
}

// Callback para o botão "Próximo ->"
void on_next_page(GtkWidget *widget, gpointer data) {
    if (currentPage < totalPages - 1) {
        currentPage++;
        gtk_widget_destroy(events_view); // Destrói-se a janela anterior
        events_view = NULL; // Esse detalhe aqui causou alguns problemas
        show_events_view(); 
    }
}

// Callback para o botão "<- Anterior"
void on_prev_page(GtkWidget *widget, gpointer data) {
    if (currentPage > 0) {
        currentPage--; // Ao clicar, retornamos uma página, logo currentPage reduz
        gtk_widget_destroy(events_view); // Destrói-se a janela anterior
        events_view = NULL;
        show_events_view(); 
    }
}

// Callback para o botão "Voltar ao Menu"
void on_back_to_menu(GtkWidget *widget, gpointer data) {
    currentPage = 0; // reseta currentPage
    gtk_widget_destroy(events_view); // Destrói-se tudo
    events_view = NULL; 
    show_main_menu(); //Retorna-se ao menu principal
}

// Callback para fechar a janela (primeiro gtk_connection)
void on_window_destroy() {
    gtk_main_quit(); // Destrói tudo
}

// Função para exibir mensagens de erro
void show_error_message(const char *message) {
    // Bloqueia a janela principal e abre uma janela de pop-up
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                               GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_ERROR,
                                               GTK_BUTTONS_CLOSE,
                                               "%s", message);
    gtk_dialog_run(GTK_DIALOG(dialog));//executa
    gtk_widget_destroy(dialog);//elimina
}

// Função para exibir mensagens informativas
void show_info_message(const char *message) {
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                               GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_MESSAGE_INFO,
                                               GTK_BUTTONS_CLOSE,
                                               "%s", message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// Função para adicionar um novo evento
void show_add_event_form() {
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *entry_day, *entry_month, *entry_year, *entry_hour;
    GtkWidget *entry_local, *entry_lutador1, *entry_lutador2, *entry_descricao;
    GtkWidget *combo_event_type;

    // Criar uma janela de diálogo modal
    dialog = gtk_dialog_new_with_buttons("Adicionar Novo Evento", GTK_WINDOW(window),
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         "_Salvar", GTK_RESPONSE_OK,
                                         "_Cancelar", GTK_RESPONSE_CANCEL,
                                         NULL);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    // Criar um grid para organizar os widgets
    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(content_area), grid);

    // Tipo de Evento
    label = gtk_label_new("Tipo de Evento:");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);

    combo_event_type = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_event_type), "UFC");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_event_type), "Fight Night");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_event_type), 0);
    gtk_grid_attach(GTK_GRID(grid), combo_event_type, 1, 0, 1, 1);

    // Dia
    label = gtk_label_new("Dia:");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);

    entry_day = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry_day), 2);
    gtk_grid_attach(GTK_GRID(grid), entry_day, 1, 1, 1, 1);

    // Mês
    label = gtk_label_new("Mês:");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 2, 1, 1);

    entry_month = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry_month), 2);
    gtk_grid_attach(GTK_GRID(grid), entry_month, 1, 2, 1, 1);

    // Ano
    label = gtk_label_new("Ano:");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 3, 1, 1);

    entry_year = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry_year), 4);
    gtk_grid_attach(GTK_GRID(grid), entry_year, 1, 3, 1, 1);

    // Hora
    label = gtk_label_new("Hora:");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 4, 1, 1);

    entry_hour = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry_hour), 2);
    gtk_grid_attach(GTK_GRID(grid), entry_hour, 1, 4, 1, 1);

    // Local
    label = gtk_label_new("Local:");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 5, 1, 1);

    entry_local = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry_local), 29);
    gtk_grid_attach(GTK_GRID(grid), entry_local, 1, 5, 1, 1);

    // Lutador 1
    label = gtk_label_new("Lutador 1:");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 6, 1, 1);

    entry_lutador1 = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry_lutador1), 10);
    gtk_grid_attach(GTK_GRID(grid), entry_lutador1, 1, 6, 1, 1);

    // Lutador 2
    label = gtk_label_new("Lutador 2:");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 7, 1, 1);

    entry_lutador2 = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry_lutador2), 10);
    gtk_grid_attach(GTK_GRID(grid), entry_lutador2, 1, 7, 1, 1);

    // Descrição (para Fight Night)
    label = gtk_label_new("Descrição:");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 8, 1, 1);

    entry_descricao = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry_descricao), 30);
    gtk_grid_attach(GTK_GRID(grid), entry_descricao, 1, 8, 1, 1);

    // Inicialmente, mostrar campos de acordo com o tipo selecionado
    on_event_type_changed(GTK_COMBO_BOX(combo_event_type), NULL);

    // Conectar o sinal de mudança do combo para mostrar/esconder campos
    g_signal_connect(combo_event_type, "changed", G_CALLBACK(on_event_type_changed), NULL);

    gtk_widget_show_all(dialog);

    // Executar o diálogo e capturar a resposta
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));

    if (result == GTK_RESPONSE_OK) {
        // Obter os valores das entradas
        const gchar *type = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo_event_type));
        const gchar *day_text = gtk_entry_get_text(GTK_ENTRY(entry_day));
        const gchar *month_text = gtk_entry_get_text(GTK_ENTRY(entry_month));
        const gchar *year_text = gtk_entry_get_text(GTK_ENTRY(entry_year));
        const gchar *hour_text = gtk_entry_get_text(GTK_ENTRY(entry_hour));
        const gchar *local_text = gtk_entry_get_text(GTK_ENTRY(entry_local));

        // Validar entradas numéricas
        if (!validate_numeric_input(day_text) || !validate_numeric_input(month_text) ||
            !validate_numeric_input(year_text) || !validate_numeric_input(hour_text)) {
            show_error_message("Por favor, insira valores numéricos válidos em dia, mês, ano e hora.");
            gtk_widget_destroy(dialog);
            return;
        }
        // Converter as entradas
        int day = atoi(day_text);
        int month = atoi(month_text);
        int year = atoi(year_text);
        int hour = atoi(hour_text);

        for(int i = 0; i < numAllEvents; i++){
            struct genericEvent* event = &allEvents[i];
            if(event->dt.dia == day && event->dt.mes == month && event->dt.ano == year){
                // Condição: se day E month E year for IGUAL a algum evento já existente:
                if(event->type == (strcmp(type, "UFC") == 0 ? UFC_EVENT : FIGHT_NIGHT_EVENT)){
                    // SE evenType event == allEvents[i], então show_error_message("Evento já cadastrado!");
                    show_error_message("Evento já cadastrado!");
                    gtk_widget_destroy(dialog);
                    dialog = NULL;
                    return;
                }
            }
        }

        // Obter a data atual
        time_t t = time(NULL);
        struct tm *current_time = localtime(&t);

        // Converter a data do evento para time_t
        struct tm event_time = {0};
        event_time.tm_mday = day;
        event_time.tm_mon = month - 1;
        event_time.tm_year = year - 1900;
        event_time.tm_hour = hour;
        time_t event_timestamp = mktime(&event_time);

        if (day <= 0 || day > 31 || month <= 0 || month > 12 || year < current_time->tm_year + 1900 ||
            hour < 0 || hour > 23 || strlen(local_text) == 0 || event_timestamp == -1 || year > 2025) {
            show_error_message("Por favor, insira valores válidos.");
        } else if (difftime(event_timestamp, t) < 7 * 24 * 3600) {
            show_error_message("O evento deve ser marcado com pelo menos 7 dias de antecedência.");
        } else {
            // Criar o evento e adicionar ao array
            struct genericEvent newEvent;
            newEvent.dt.dia = day;
            newEvent.dt.mes = month;
            newEvent.dt.ano = year;
            newEvent.dt.hora = hour;
            strncpy(newEvent.local, local_text, 29);
            newEvent.local[29] = '\0';

            if (strcmp(type, "UFC") == 0) {
                const gchar *lutador1_text = gtk_entry_get_text(GTK_ENTRY(entry_lutador1));
                const gchar *lutador2_text = gtk_entry_get_text(GTK_ENTRY(entry_lutador2));

                if (strlen(lutador1_text) == 0 || strlen(lutador2_text) == 0) {
                    show_error_message("Por favor, insira os nomes dos lutadores.");
                } else {
                    newEvent.type = UFC_EVENT;
                    strncpy(newEvent.details.ufc.lutador1, lutador1_text, 10);
                    newEvent.details.ufc.lutador1[10] = '\0';
                    strncpy(newEvent.details.ufc.lutador2, lutador2_text, 10);
                    newEvent.details.ufc.lutador2[10] = '\0';

                    struct genericEvent *newEvents = realloc(allEvents, (numAllEvents + 1) * sizeof(struct genericEvent));
                    if (newEvents == NULL) {
                        show_error_message("Erro de alocação de memória.");
                    } else {
                        allEvents = newEvents;
                        allEvents[numAllEvents] = newEvent;
                        numAllEvents++;

                        // Reordenar os eventos
                        qsort(allEvents, numAllEvents, sizeof(struct genericEvent), compare_events_by_date);

                        save_events();
                        show_info_message("Evento adicionado com sucesso.");
                    }
                }
            } else {
                const gchar *descricao_text = gtk_entry_get_text(GTK_ENTRY(entry_descricao));

                if (strlen(descricao_text) == 0) {
                    show_error_message("Por favor, insira uma descrição.");
                } else {
                    newEvent.type = FIGHT_NIGHT_EVENT;
                    strncpy(newEvent.details.descricao, descricao_text, 30);
                    newEvent.details.descricao[30] = '\0';

                    struct genericEvent *newEvents = realloc(allEvents, (numAllEvents + 1) * sizeof(struct genericEvent));
                    if (newEvents == NULL) {
                        show_error_message("Erro de alocação de memória.");
                    } else {
                        allEvents = newEvents;
                        allEvents[numAllEvents] = newEvent;
                        numAllEvents++;

                        // Reordenar os eventos
                        qsort(allEvents, numAllEvents, sizeof(struct genericEvent), compare_events_by_date);

                        save_events();
                        show_info_message("Evento adicionado com sucesso.");
                    }
                }
            }
        }
    }

    gtk_widget_destroy(dialog);
}

// Callback para mudança do tipo de evento
void on_event_type_changed(GtkComboBox *widget, gpointer data) {
    const gchar *type = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));

    // Obter os widgets necessários
    GtkWidget *dialog = gtk_widget_get_toplevel(GTK_WIDGET(widget));
    GtkWidget *grid = gtk_bin_get_child(GTK_BIN(gtk_dialog_get_content_area(GTK_DIALOG(dialog))));
    GtkWidget *entry_lutador1 = gtk_grid_get_child_at(GTK_GRID(grid), 1, 6);
    GtkWidget *entry_lutador2 = gtk_grid_get_child_at(GTK_GRID(grid), 1, 7);
    GtkWidget *label_lutador1 = gtk_grid_get_child_at(GTK_GRID(grid), 0, 6);
    GtkWidget *label_lutador2 = gtk_grid_get_child_at(GTK_GRID(grid), 0, 7);
    GtkWidget *entry_descricao = gtk_grid_get_child_at(GTK_GRID(grid), 1, 8);
    GtkWidget *label_descricao = gtk_grid_get_child_at(GTK_GRID(grid), 0, 8);

    if (strcmp(type, "UFC") == 0) {
        // Mostrar campos de lutadores
        gtk_widget_show(entry_lutador1);
        gtk_widget_show(entry_lutador2);
        gtk_widget_show(label_lutador1);
        gtk_widget_show(label_lutador2);
        // Esconder descrição
        gtk_widget_hide(entry_descricao);
        gtk_widget_hide(label_descricao);
    } else {
        // Esconder campos de lutadores
        gtk_widget_hide(entry_lutador1);
        gtk_widget_hide(entry_lutador2);
        gtk_widget_hide(label_lutador1);
        gtk_widget_hide(label_lutador2);
        // Mostrar descrição
        gtk_widget_show(entry_descricao);
        gtk_widget_show(label_descricao);
    }
}

// Função para mostrar eventos por data
void show_events_by_date() {
    // Criar diálogo para entrada da data
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Mostrar Eventos por Data",
                                                    GTK_WINDOW(window),
                                                    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    "_OK", GTK_RESPONSE_OK,
                                                    "_Cancelar", GTK_RESPONSE_CANCEL,
                                                    NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(content_area), grid);

    GtkWidget *label = gtk_label_new("Mês:");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    GtkWidget *entry_month = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry_month), 2);
    gtk_grid_attach(GTK_GRID(grid), entry_month, 1, 0, 1, 1);

    label = gtk_label_new("Ano:");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    GtkWidget *entry_year = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry_year), 4);
    gtk_grid_attach(GTK_GRID(grid), entry_year, 1, 1, 1, 1);

    gtk_widget_show_all(dialog);

    gint result = gtk_dialog_run(GTK_DIALOG(dialog));

    if (result == GTK_RESPONSE_OK) {
        const gchar *month_text = gtk_entry_get_text(GTK_ENTRY(entry_month));
        const gchar *year_text = gtk_entry_get_text(GTK_ENTRY(entry_year));

        // Validar entradas numéricas
        if (!validate_numeric_input(month_text) || !validate_numeric_input(year_text)) {
            show_error_message("Por favor, insira valores numéricos válidos em mês e ano.");
            gtk_widget_destroy(dialog);
            return;
        }

        int month = atoi(month_text);
        int year = atoi(year_text);

        if (month <= 0 || month > 12 || year <= 0) {
            show_error_message("Por favor, insira um mês e ano válidos.");
        } else {
            // Filtrar eventos por data
            struct genericEvent *filteredEvents = NULL;
            int numFilteredEvents = 0;

            for (int i = 0; i < numAllEvents; i++) {
                if (allEvents[i].dt.mes == month &&
                    allEvents[i].dt.ano == year) {
                    struct genericEvent *newEvents = realloc(filteredEvents, (numFilteredEvents + 1) * sizeof(struct genericEvent));
                    if (newEvents == NULL) {
                        show_error_message("Erro de alocação de memória.");
                        free(filteredEvents);
                        gtk_widget_destroy(dialog);
                        return;
                    }
                    filteredEvents = newEvents;
                    filteredEvents[numFilteredEvents] = allEvents[i];
                    numFilteredEvents++;
                }
            }

            if (numFilteredEvents == 0) {
                show_info_message("Nenhum evento encontrado para esta data.");
                free(filteredEvents);
            } else {
                // Exibir eventos filtrados com paginação
                gtk_widget_destroy(dialog);
                show_filtered_events_view(filteredEvents, numFilteredEvents);
                return;
            }
        }
    }

    gtk_widget_destroy(dialog);
}

// Função para mostrar eventos por local
void show_events_by_location() {
    // Criar diálogo para entrada do local
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Mostrar Eventos por Local",
                                                    GTK_WINDOW(window),
                                                    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    "_OK", GTK_RESPONSE_OK,
                                                    "_Cancelar", GTK_RESPONSE_CANCEL,
                                                    NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(content_area), grid);

    GtkWidget *label = gtk_label_new("Local:");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    GtkWidget *entry_local = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry_local), 29);
    gtk_grid_attach(GTK_GRID(grid), entry_local, 1, 0, 1, 1);

    gtk_widget_show_all(dialog);

    gint result = gtk_dialog_run(GTK_DIALOG(dialog));

    if (result == GTK_RESPONSE_OK) {
        const gchar *local_text = gtk_entry_get_text(GTK_ENTRY(entry_local));

        if (strlen(local_text) == 0) {
            show_error_message("Por favor, insira um local válido.");
        } else {
            // Filtrar eventos por local
            struct genericEvent *filteredEvents = NULL;
            int numFilteredEvents = 0;

            for (int i = 0; i < numAllEvents; i++) {
                if (strcasecmp(allEvents[i].local, local_text) == 0) {
                    struct genericEvent *newEvents = realloc(filteredEvents, (numFilteredEvents + 1) * sizeof(struct genericEvent));
                    if (newEvents == NULL) {
                        show_error_message("Erro de alocação de memória.");
                        free(filteredEvents);
                        gtk_widget_destroy(dialog);
                        return;
                    }
                    filteredEvents = newEvents;
                    filteredEvents[numFilteredEvents] = allEvents[i];
                    numFilteredEvents++;
                }
            }

            if (numFilteredEvents == 0) {
                show_info_message("Nenhum evento encontrado para este local.");
                free(filteredEvents);
            } else {
                // Exibir eventos filtrados com paginação
                gtk_widget_destroy(dialog);
                show_filtered_events_view(filteredEvents, numFilteredEvents);
                return;
            }
        }
    }

    gtk_widget_destroy(dialog);
}

// Função para mostrar eventos filtrados com paginação
void show_filtered_events_view(struct genericEvent *events, int numEvents) {
    if (main_menu != NULL) {
        gtk_widget_destroy(main_menu);
        main_menu = NULL;
    }

    if (events_view != NULL) {
        gtk_widget_destroy(events_view);
    }

    // Alocar memória para os dados da visualização filtrada
    FilteredEventsData *data = malloc(sizeof(FilteredEventsData));
    if (data == NULL) {
        show_error_message("Erro de alocação de memória.");
        return;
    }

    data->events = events;
    data->numEvents = numEvents;
    data->currentPage = 0;
    data->totalPages = (numEvents + MAX_EVENTS_PER_PAGE - 1) / MAX_EVENTS_PER_PAGE;

    data->events_view = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    // Armazenar events_view globalmente para futuras destruições
    events_view = data->events_view;

    gtk_container_add(GTK_CONTAINER(window), data->events_view);

    // Inicializar a visualização
    refresh_filtered_events_view(data);
}

// Função para atualizar a visualização filtrada
void refresh_filtered_events_view(FilteredEventsData *data) {
    // Remover todos os filhos de events_view
    GList *children = gtk_container_get_children(GTK_CONTAINER(data->events_view));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);

    GtkWidget *title = gtk_label_new("Eventos Filtrados");
    gtk_box_pack_start(GTK_BOX(data->events_view), title, FALSE, FALSE, 0);

    GtkWidget *event_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(data->events_view), event_box, TRUE, TRUE, 0);

    // Exibe os eventos da página atual
    int startIndex = data->currentPage * MAX_EVENTS_PER_PAGE;
    int endIndex = startIndex + MAX_EVENTS_PER_PAGE;
    if (endIndex > data->numEvents) endIndex = data->numEvents;

    for (int i = startIndex; i < endIndex; i++) {
        GtkWidget *event_label;
        struct genericEvent *event = &data->events[i];
        char event_info[256];

        if (event->type == UFC_EVENT) {
            sprintf(event_info, "UFC - %02d/%02d/%04d %02d:00\nLocal: %s\nLuta: %s vs %s",
                    event->dt.dia, event->dt.mes, event->dt.ano, event->dt.hora,
                    event->local, event->details.ufc.lutador1, event->details.ufc.lutador2);
        } else if (event->type == FIGHT_NIGHT_EVENT) {
            sprintf(event_info, "Fight Night - %02d/%02d/%04d %02d:00\nLocal: %s\nDescrição: %s",
                    event->dt.dia, event->dt.mes, event->dt.ano, event->dt.hora,
                    event->local, event->details.descricao);
        }

        event_label = gtk_label_new(event_info);
        gtk_box_pack_start(GTK_BOX(event_box), event_label, FALSE, FALSE, 0);
    }

    // Botões de navegação
    GtkWidget *navigation_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(data->events_view), navigation_box, FALSE, FALSE, 0);

    if (data->currentPage > 0) {
        GtkWidget *prev_button = gtk_button_new_with_label("<- Anterior");
        gtk_box_pack_start(GTK_BOX(navigation_box), prev_button, TRUE, TRUE, 0);
        g_signal_connect(prev_button, "clicked", G_CALLBACK(on_filtered_prev_page), data);
    }

    if (data->currentPage < data->totalPages - 1) {
        GtkWidget *next_button = gtk_button_new_with_label("Próximo ->");
        gtk_box_pack_start(GTK_BOX(navigation_box), next_button, TRUE, TRUE, 0);
        g_signal_connect(next_button, "clicked", G_CALLBACK(on_filtered_next_page), data);
    }

    // Botão para voltar ao menu principal
    GtkWidget *back_button = gtk_button_new_with_label("Voltar ao Menu");
    gtk_box_pack_start(GTK_BOX(data->events_view), back_button, FALSE, FALSE, 0);
    g_signal_connect(back_button, "clicked", G_CALLBACK(on_filtered_back_to_menu), data);

    gtk_widget_show_all(data->events_view);
}

// Callback para o botão "Próximo ->" na visualização filtrada
void on_filtered_next_page(GtkWidget *widget, gpointer user_data) {
    FilteredEventsData *data = (FilteredEventsData *)user_data;

    if (data->currentPage < data->totalPages - 1) {
        data->currentPage++;
        refresh_filtered_events_view(data);
    }
}

// Callback para o botão "<- Anterior" na visualização filtrada
void on_filtered_prev_page(GtkWidget *widget, gpointer user_data) {
    FilteredEventsData *data = (FilteredEventsData *)user_data;

    if (data->currentPage > 0) {
        data->currentPage--;
        refresh_filtered_events_view(data);
    }
}

// Callback para o botão "Voltar ao Menu" na visualização filtrada
void on_filtered_back_to_menu(GtkWidget *widget, gpointer user_data) {
    FilteredEventsData *data = (FilteredEventsData *)user_data;

    gtk_widget_destroy(data->events_view);
    data->events_view = NULL;

    // Liberar memória alocada para os eventos filtrados
    free(data->events);
    // Liberar a estrutura de dados
    free(data);

    show_main_menu();
}

// Função para deletar um evento
void show_delete_event() {
    // Criar uma lista de eventos futuros
    struct genericEvent *futureEvents = NULL;
    int numFutureEvents = 0;
    time_t t = time(NULL);

    for (int i = 0; i < numAllEvents; i++) {
        if (difftime(event_to_timestamp(&allEvents[i]), t) > 0) {
            struct genericEvent *newEvents = realloc(futureEvents, (numFutureEvents + 1) * sizeof(struct genericEvent));
            if (newEvents == NULL) {
                show_error_message("Erro de alocação de memória.");
                free(futureEvents);
                return;
            }
            futureEvents = newEvents;
            futureEvents[numFutureEvents] = allEvents[i];
            numFutureEvents++;
        }
    }

    if (numFutureEvents == 0) {
        show_info_message("Não há eventos futuros para deletar.");
        return;
    }

    // Criar diálogo com lista de eventos
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Deletar Evento",
                                                    GTK_WINDOW(window),
                                                    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    "_Deletar", GTK_RESPONSE_OK,
                                                    "_Cancelar", GTK_RESPONSE_CANCEL,
                                                    NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *combo_events = gtk_combo_box_text_new();
    gtk_container_add(GTK_CONTAINER(content_area), combo_events);

    char event_info[256];
    for (int i = 0; i < numFutureEvents; i++) {
        struct genericEvent *event = &futureEvents[i];
        if (event->type == UFC_EVENT) {
            sprintf(event_info, "UFC - %02d/%02d/%04d %02d:00 - %s vs %s",
                    event->dt.dia, event->dt.mes, event->dt.ano, event->dt.hora,
                    event->details.ufc.lutador1, event->details.ufc.lutador2);
        } else if (event->type == FIGHT_NIGHT_EVENT) {
            sprintf(event_info, "Fight Night - %02d/%02d/%04d %02d:00 - %s",
                    event->dt.dia, event->dt.mes, event->dt.ano, event->dt.hora,
                    event->details.descricao);
        }
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_events), event_info);
    }

    gtk_widget_show_all(dialog);

    gint result = gtk_dialog_run(GTK_DIALOG(dialog));

    if (result == GTK_RESPONSE_OK) {
        int selected_index = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_events));
        if (selected_index >= 0) {
            // Remover o evento selecionado de allEvents
            struct genericEvent selected_event = futureEvents[selected_index];

            int found = -1;
            for (int i = 0; i < numAllEvents; i++) {
                if (compare_events_by_date(&allEvents[i], &selected_event) == 0 &&
                    allEvents[i].type == selected_event.type) {
                    found = i;
                    break;
                }
            }

            if (found >= 0) {
                for (int i = found; i < numAllEvents - 1; i++) {
                    allEvents[i] = allEvents[i + 1];
                }
                numAllEvents--;
                allEvents = realloc(allEvents, numAllEvents * sizeof(struct genericEvent));
                save_events();
                show_info_message("Evento deletado com sucesso.");
            } else {
                show_error_message("Erro ao deletar o evento.");
            }
        } else {
            show_error_message("Nenhum evento selecionado.");
        }
    }

    free(futureEvents);
    gtk_widget_destroy(dialog);
}
