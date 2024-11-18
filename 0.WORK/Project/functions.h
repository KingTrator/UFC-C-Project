#ifdef FUNCTIONS_H
#define FUNCTIONS_H
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>

#define RED "\033[1;31m"  
#define RESET "\033[0m"    

void cool_print();
int interactive_menu();
short get_data();
short daysInMonth[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};




#endif