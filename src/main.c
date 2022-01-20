#include "stm8s.h"
#include "milis.h"
#include "stm8_hd44780.h"
#include "spse_stm8.h" 
#include "delay.h"
#include <stdio.h>

#define _ISOC99_SOURCE
#define _GNU_SOURCE

void setup(void){
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);// taktování na 16MHz
    lcd_init(); //inicializace LCD
}

void GPIO_setup(void){
    GPIO_Init(GPIOC, GPIO_PIN_5, GPIO_MODE_OUT_PP_LOW_SLOW); //trig
    GPIO_Init(GPIOC, GPIO_PIN_1, GPIO_MODE_IN_FL_NO_IT); // echo
		
}

void main(){
    setup();

    lcd_gotoxy(0,0);
    lcd_puts("cum");

    while (1) {

        char text[32];

    }
}
#include "__assert__.h"