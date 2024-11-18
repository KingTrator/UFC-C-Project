#include "functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

int main(){

    int on = 1;
    while(on){
        int answer = interactive_menu();
        on = answer;

        if(answer == 1){
            cool_print();
            printf("\nVoce escolheu cadastrar um novo evento\n");
            cool_print();

            short wrong = 1;
            while(wrong){
                wrong = getData();
            }
            short hora = 0;
            while(hora < 1 || hora > 24){ // UFC might happen at any time, since it's international
                printf("\nOk! Qual o horario do evento? (ENTRE 1 e 24)\n");
                printf("Hora: ");
                scanf("%d", &hora);
                puts("");
            }

            char a = 10;
            while(a != 1 || a != 2){
                printf("O Evento eh UFC (Evento Grande) (1) ou Fight Night (Evento pequeno)? (2)\n");
                printf("Tipo: ");
                scanf("%c", &a);
            }

        } 
    }
}