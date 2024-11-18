// functions.c
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include "functions.h"
#include <time.h>
#define RED "\033[1;31m"  
#define RESET "\033[0m"    

struct Data{
    int day;
    int month;
    int year;
} data;
short daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};


short isValidDate(struct Data* data) {
    time_t t = time(NULL);
    struct tm today = *localtime(&t); // Get current date

    // Adjust to match our struct format
    short currentYear = today.tm_year + 1900;
    short currentMonth = today.tm_mon + 1;
    short currentDay = today.tm_mday;

    // Check valid year
    if (data->year < currentYear || data->year > 2025) {
        printf("\n Ano INVALIDO (2024 ou 2025)!\n");
        return 1;
    }

    // Check valid month
    if (data->month < 1 || data->month > 12) {
        printf("\n Mes INVALIDO (1 a 12)\n");
        return 1;
    }

    // Check valid day
    if (data->day < 1 || data->day > daysInMonth[data->month - 1]) {
        printf("\n dia INVALIDO! (1 a 30/31), fev: 1 a 28\n");
        return 1;
    }

    // Additional check: Must be today or later
    if (data->year == currentYear && (data->month < currentMonth || 
       (data->month == currentMonth && data->day < currentDay+1))) {
        printf("\nNao faz sentido cadastrar no passado!\n");
        printf("Apenas de AMANHA para o futuro\n");
        return 1;
    }

    return 0;
}

short getData(){
    printf("\n Dia: ");
    scanf("%d", &data.day);
    printf("\n Mes: ");
    scanf("%d", &data.month);
    printf("\n Ano: ");
    scanf("%d", &data.year);
    puts("");
    return isValidDate(&data);

}

void cool_print(){
    puts("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-");
}

int interactive_menu() {
    const char *options[] = {
        "SAIR",
        "Cadastrar novo evento",
        "Ver todos os eventos",
        "Ver todos os eventos com data",
        "Procurar evento por descricao (STRING EXATA)",
        "Remover evento"
    };
    const short num_options = sizeof(options) / sizeof(options[0]);
    short selected = 0;
    short ch;
    short displaying = 1; 
    short tooFast = 0;
    while (displaying) {
        if(tooFast >= 2) Sleep(100);
        tooFast %= 2;
        system("cls");  // Clear the screen, slow operation, might cause screen flickering - I'll try to solve this
        /*
            I have to improve the FPS experience to make this menu smoother Sleep alone isn't working
            So I have concluded this:
            First, swap Sleep with Clock() from time.h
            Then I'll have to take manual control over getch() buffer, to avoid ghost effect.
            By creating a circular buffer, in which I'll store only the last size(options)/size(options[0]) escape sequences sent
            I'll hold an illusion of IN TIME RESPONSIVINESS and SMOOTH SCREEN
        */


        // Display the menu options
        for (int i = 0; i < num_options; i++) {
            if(i == selected){
                printf("-> ");
            } else {
                printf("   ");
            }

            if(i == 0){
                printf(RED "%s" RESET "\n", options[i]);
            } else {
                printf("%s\n", options[i]);
            }
        }

        // Get user input
        ch = getch();
        if (ch == 0 || ch == 224) {
            // Arrow keys are returned as two characters: 0 or 224 followed by the code
            ch = getch();
            switch (ch) {
                case 72: // Up arrow
                    selected--;
                    if (selected < 0) {
                        selected = num_options - 1;
                    }
                    break;
                case 80: // Down arrow
                    selected++;
                    if (selected >= num_options) {
                        selected = 0;
                    }
                    break;
            }
        } else if (ch == '\r') {  // Enter key
            // Handle the selected option
            switch (selected) {
                case 0:
                    return 0;
                    break;
                case 1:
                    return 1;
                    break;
                case 2:
                    return 2; //TODOS, PASSADOS OU NOVOS.
                    break;
                case 3:
                    return 3;
                    break;
                case 4:
                    return 4; //STRING UFC 300, 301, ETC
                    break;
                case 5:
                    return 5;
            }

        }
    }
}

