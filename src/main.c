#include "stm8s.h"
#include "delay.h"
#include "milis.h"
#include "stm8_hd44780.h"

#define LED_Port GPIOC
#define LED_Pin GPIO_PIN_5

unsigned char bl_state;
unsigned char data_value;
unsigned int present_value = 0x0000;
unsigned int previous_value = 0x0001;
unsigned int time1 = 0x0010;
unsigned int difference = 1000;

void GPIO_setup(void);
void lcd_print(unsigned char x_pos, unsigned char y_pos, unsigned int value);

void main(){

	CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);      // 16MHz z interního RC oscilátoru
	init_milis(); 
    GPIO_setup();
    lcd_init(); 
    lcd_clear();
    lcd_home();
	lcd_print(0,0,difference);

    while(1){
							if(GPIO_ReadInputPin(GPIOE,GPIO_PIN_4) == RESET){
								lcd_print(8,1,milis()/2);
							}
							if( milis() != time1){ 
								lcd_print(8,0,milis()/2);								
								time1 = milis();			
							}
       };
}

void GPIO_setup(void){
        GPIO_DeInit(GPIOC);
        GPIO_Init(GPIOC,((GPIO_Pin_TypeDef)(GPIO_PIN_1 | GPIO_PIN_2)),GPIO_MODE_IN_PU_NO_IT);
        GPIO_DeInit(GPIOD);
        GPIO_Init(LED_Port, LED_Pin, GPIO_MODE_OUT_PP_LOW_SLOW);
	    GPIO_Init(GPIOE,GPIO_PIN_4,GPIO_MODE_IN_FL_NO_IT);
}

void lcd_print(unsigned char x_pos, unsigned char y_pos, unsigned int value){
       char tmp[5] = {0x20, 0x20, 0x20, 0x20, '\0'} ;
       tmp[0] = ((value / 1000) + 0x30);
       tmp[1] = (((value / 100) % 10) + 0x30);
       tmp[2] = (((value / 10) % 10) + 0x30);
       tmp[3] = ((value % 10) + 0x30);
       lcd_gotoxy(x_pos, y_pos);
       lcd_puts(tmp);  
}
#include "__assert__.h"