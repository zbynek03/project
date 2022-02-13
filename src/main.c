#include "stm8s.h"
#include "milis.h"
#include "stm8_hd44780.h"
#include "delay.h"
#include <stdio.h>

#define _ISOC99_SOURCE
#define _GNU_SOURCE

#define PULSE_LEN 2             // délka spouštěcího (trigger) pulzu pro ultrazvuk
#define MEASURMENT_PERIOD 100   // perioda měření ultrazvukem (měla by být víc jak (maximální_dosah*2)/rychlost_zvuku)

uint16_t capture = 1;               // tady bude aktuální výsledek měření (času)
uint8_t capture_flag = 0;	//tady se ukládá zda bylo provedeno měření času

uint16_t vzdalenost = 0;

uint8_t minuly_stav=1; // zde si budeme ukládat minulý stav tlačítka (1=tlačítko stisknuté, 0=tlačítko uvolněné)
uint8_t aktualni_stav=1;  // zde si budeme ukládat aktuální stav tlačítka (1=tlačítko stisknuté, 0=tlačítko uvolněné)
// u funkce GPIO_ReadInputPin, hodnota RESET znamená log.0 a tedy stisknuté tlačítko

static uint8_t stage = 0;   // stavový automat
static uint16_t time = 0;   // pro časování pomocí milis
//static nejde dát do funkce => build-STM8S208/main.asm:1621: Error: <r> relocation error

void setup(void){
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);// taktování na 16MHz
    lcd_init(); //inicializace LCD
}

void GPIO_setup(void){
    GPIO_Init(GPIOC, GPIO_PIN_7, GPIO_MODE_OUT_PP_LOW_SLOW); //trig
    //GPIO_Init(GPIOD, GPIO_PIN_4, GPIO_MODE_IN_FL_NO_IT); // echo
    GPIO_Init(GPIOC,GPIO_PIN_5,GPIO_MODE_OUT_PP_LOW_SLOW); // nastavíme PC5 jako výstup typu push-pull (LEDka)
    GPIO_Init(GPIOE, GPIO_PIN_4,GPIO_MODE_IN_FL_NO_IT); // nastavíme PE4 jako vstup (tlačítko)
		
}
void TIM1_setup(void)
{
       TIM1_TimeBaseInit(15, TIM1_COUNTERMODE_UP, 0xffff, 0);      // timer necháme volně běžet (do maximálního stropu) s časovou základnou 1MHz (1us)
// Konfigurujeme parametry capture kanálu 1 - komplikované, nelze popsat v krátkém komentáři
    TIM1_ICInit(TIM1_CHANNEL_1, TIM1_ICPOLARITY_RISING,
                TIM1_ICSELECTION_DIRECTTI, TIM1_ICPSC_DIV1, 0);
// Konfigurujeme parametry capture kanálu 2 - komplikované, nelze popsat v krátkém komentáři
    TIM1_ICInit(TIM1_CHANNEL_2, TIM1_ICPOLARITY_FALLING,
                TIM1_ICSELECTION_INDIRECTTI, TIM1_ICPSC_DIV1, 0);
    TIM1_SelectInputTrigger(TIM1_TS_TI1FP1);    // Zdroj signálu pro Clock/Trigger controller 
    TIM1_SelectSlaveMode(TIM1_SLAVEMODE_RESET); // Clock/Trigger má po příchodu signálu provést RESET timeru
    TIM1_ClearFlag(TIM1_FLAG_CC2);      // pro jistotu vyčistíme vlajku signalizující záchyt a změření echo pulzu
    TIM1_Cmd(ENABLE);   
}

void process_measurment(void){
    switch (stage) {
    case 0: 
                          // čekáme než uplyne  MEASURMENT_PERIOD abychom odstartovali měření
        if (milis() - time > MEASURMENT_PERIOD * 4) {
            
            time = milis();
            
            GPIO_WriteHigh(GPIOC, GPIO_PIN_7);  // zahájíme trigger pulz
            
            stage = 1;          // a budeme čekat až uplyne čas trigger pulzu;
            
        }
        break;
    case 1: 
                            // čekáme než uplyne PULSE_LEN (generuje trigger pulse)
        if (milis() - time > PULSE_LEN * 4) {

            GPIO_WriteLow(GPIOC, GPIO_PIN_7);
   // ukončíme trigger pulz
            stage = 2;          // a přejdeme do fáze kdy očekáváme echo
        }
        break;
    case 2:


           // čekáme jestli dostaneme odezvu (čekáme na echo)
        if (TIM1_GetFlagStatus(TIM1_FLAG_CC2) != RESET) {

                   // hlídáme zda timer hlásí změření pulzu
            capture = TIM1_GetCapture2();
                   // uložíme výsledek měření
            capture_flag = 1;   // dáme vědět zbytku programu že máme nový platný výsledek

            stage = 0;          // a začneme znovu od začátku
        } else if (milis() - time > MEASURMENT_PERIOD * 4) {

      // pokud timer nezachytil pulz po dlouhou dobu, tak echo nepřijde
            stage = 0;          // a začneme znovu od začátku
        }
        break;
    default:                   // pokud se cokoli pokazí
        stage = 0;              // začneme znovu od začátku
    }
}

void main(){
    init_milis();
    setup();
    GPIO_setup();
    TIM1_setup();
    lcd_gotoxy(0,1);
    lcd_puts("vz=");
    char text[32];

    while (1){
        //if(GPIO_ReadInputPin(GPIOE,GPIO_PIN_4)==RESET){
        process_measurment(); //zajištuje měření
        if(capture_flag == 1){ //jakmile je nová hodnota tak se vypíše nový výsledek a flag se nastaví na 0
            vzdalenost = capture;
            //lcd_clear();
            lcd_gotoxy(0,1);
            sprintf(text,"vzd=%3ucm",vzdalenost/58);
            lcd_puts(text);
            capture_flag = 0;
            }
        //}
    }					
}
#include "__assert__.h"