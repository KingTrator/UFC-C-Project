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
            char x;
            scanf("%c", &x); // Just to avoid immediate reload. I'll think on somen' better later
            /*
                Create option to leave here as well
                instead of reloading the menu
            */
        } 
    }
}