#include "functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
short wrong = 1;// Usado para todos os loops nos quais o usuário pode errar (e tentar de novo)
short on;  // Acompanha a escolha do usuário dentro das opções do menu
struct Date{
    int day;
    int month;
    int year;
};

/*
    Tive uma ideia de como podemos atender ao critério "procurar data específcia"
    e "procurar por string exata" sem sofrer:
    O nome dos eventos pode ser salvo nos arquivos no formato
    dia/mês/ano-Evento
    Assim, na hora de buscar o nome do arquivo, se for por string específica
    basta ler o nome dele até achar "-". E depois ver se o que se segue é a string EXATA.
    Caso seja por data, basta ler os valores de data até "-" (ou delimitar o buscardo para 10 caracteres)
*/

int main(){

        FILE *file;
        printf("Preparando o ambiente");sleep(1);printf(".");sleep(1);printf(".");sleep(1);printf(".\n");
        file = fopen("EventosUFC", "r");
        if(file == NULL){
            file = fopen("EventosUFC", "w");
            if(file == NULL){
                printf("Problema na criacao de arquivo!");
                exit(1);
            }
            printf("Arquivo criado.\n");
            fclose(file);
        }
        file = fopen("EventosFightNight", "r");
        if(file == NULL){
            file = fopen("EventosFightNight", "w");
            if(file == NULL){
                printf("Problema na criacao de arquivo!");
                exit(1);
            }
            printf("Arquivo criado.\n");
            fclose(file);
        }
        sleep(1); system("cls");
        printf("Ok! Tudo certo!"); sleep(2);


        short answer = interactive_menu();
        on = answer;


        if(answer == 1){




            cool_print();
            printf("\nVoce escolheu cadastrar um novo evento\n");
            cool_print();
            struct Date eventDate;

            while(wrong){
                wrong = getDate(&eventDate);
            } wrong = 1;

            short hour = 0;
            while(hour < 1 || hour > 24){ 
                printf("\nOk! Qual o horario do evento? (ENTRE 1 e 24)\n");
                printf("Hora: ");
                scanf("%d", &hour);
                puts("");
            }

            short a = 10;
            while(a < 1 || a > 2){ 
                printf("O Evento eh UFC (Evento Grande) (1) ou Fight Night (Evento pequeno)? (2)\n");
                printf("Tipo: ");
                scanf("%d", &a);
                
            }
            

            if (a == 1) {
            char* eventUFC = (char*)malloc(8 * sizeof(char)); 
            char* eventNumber = (char*)malloc(4 * sizeof(char));
            short eventNumberinINT = -1;

            strcpy(eventUFC, "UFC ");

            // Validation Loop
            do {
                printf("Numero do evento (311 a 323): ");
                if (scanf("%hd", &eventNumberinINT) != 1 || eventNumberinINT < 311 || eventNumberinINT > 323) { // Há no máximo 12 UFCs por ano
                    printf("Erro: Digite apenas um numero com ate 3 digitos!\n");
                } else {
                    wrong = 0; 
                }
            } while (wrong); wrong = 1; 

            snprintf(eventNumber, 4, "%03d", eventNumberinINT); 
            strcat(eventUFC, eventNumber);

            printf("Evento: %s\n", eventUFC);

            /*
                Quando o código aumentar, será preciso criar uma lógica para checar se o evento já foi cadastrado.
                Essa lógica só serve para eventos do tipo UFC, pois as Fight Night podem acontecer em datas exatamente iguais,
                só não podem ocorrer no mesmo local.
            
            */
           // Supondo que o Evento ainda não foi cadastrado:
            cool_print();
            char* fstFighter = (char*)malloc(100 * sizeof(char));
            char* sndFighter = (char*)malloc(100 * sizeof(char));
            char* mainEvent = (char*)malloc(200 * sizeof(char));

            printf("Nome do MAIN event\n");

            do {
                printf("Lutador 1: ");
                scanf(" %99[^\n]", fstFighter); puts("");
                 
                int valid = 1;
      
                for (int i = 0; i < strlen(fstFighter); i++) {
                    if (fstFighter[i] != ' ' && (fstFighter[i] < 'A' || fstFighter[i] > 'Z') && (fstFighter[i] < 'a' || fstFighter[i] > 'z')) {
                        valid = 0; // Nome inválido
                        i = strlen(fstFighter);// Finaliza o loop
                    }
                }

                if (!valid || strlen(fstFighter) < 3) {
                    printf("Erro: Apenas caracteres alfabeticos (ASCII) sao permitidos. Eh preciso pelo menos 3 caracteres. Tente novamente.\n");
                    wrong = 1;
                } else {
                    wrong = 0; // Nome válido
                }
            } while (wrong);
            wrong = 1; // Reseta a flag

            do {
                printf("Lutador 2: ");
                scanf(" %99[^\n]", sndFighter); puts("");
                int valid = 1; 
                if(strlen(sndFighter) < 3) {
                        printf("\nDigite ao menos 3 caracteres\n");
                        valid = 0;
                    }
                for (int i = 0; i < strlen(sndFighter); i++) {
                    if (sndFighter[i] != ' ' && (sndFighter[i] < 'A' || sndFighter[i] > 'Z') && (sndFighter[i] < 'a' || sndFighter[i] > 'z')) {
                        valid = 0; // Nome inválido
                        i = strlen(sndFighter); // Finaliza o loop
                    }
                }

                if (!valid || strlen(fstFighter) < 3) {
                    printf("Erro: Apenas caracteres alfabeticos (ASCII) sao permitidos. Eh preciso pelo menos 3 caracteres. Tente novamente.\n");
                    wrong = 1;
                } else {
                    wrong = 0; // Nome válido
                }
            } while (wrong);

            wrong = 1; // Reseta a flag

            // Concatenar os nomes dos lutadores em mainEvent
            snprintf(mainEvent, 200, "%s x %s", fstFighter, sndFighter);

            // Reduzir nomes se mainEvent > 30
            if (strlen(mainEvent) > 30) {
                snprintf(mainEvent, 200, "%.3s x %.3s", fstFighter, sndFighter);
            } 
            // Reajustamos o tamanho de MainEvent para que tenha apenas o necessário
            mainEvent = (char*)realloc(mainEvent, strlen(mainEvent) * sizeof(char)+1);

            // Garante que o Main Event estará em caixa alta.
            for(int i = 0; i < strlen(mainEvent); i++) mainEvent[i] = toupper(mainEvent[i]); 
            

            printf("Main Event: %s\n", mainEvent);

            // DEFINIR LOCAL
            // Definido o local, criar o arquivo, dentro de EventosUFC na forma:
            // dia/mês/ano-UFCXXX, ou seja date.data/date.month/date.year concatena com EventUFC

            

            // Liberação de memória dos lutadores
            free(fstFighter);
            free(sndFighter);
            fstFighter = NULL;
            sndFighter = NULL;


            // Liberação de memória restante
            free(eventNumber);
            free(eventUFC);
            eventNumber = NULL;
            eventUFC = NULL;
        }

        } 
    }
