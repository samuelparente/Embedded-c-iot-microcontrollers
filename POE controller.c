/* 
 * File:   newmain.c
 * Author: samuel
 * SOLAR POE CONTROLLER
 * Created on 4 de Abril de 2019, 18:54
 */



#define _XTAL_FREQ 16000000

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <string.h>
#include <stdint.h>

// CONFIG1L
#pragma config RETEN = OFF      // VREG Sleep Enable bit (Ultra low-power regulator is Disabled (Controlled by REGSLP bit))
#pragma config INTOSCSEL = HIGH // LF-INTOSC Low-power Enable bit (LF-INTOSC in High-power mode during Sleep)
#pragma config SOSCSEL = DIG    // SOSC Power Selection and mode Configuration bits (Digital (SCLKI) mode)
#pragma config XINST = OFF      // Extended Instruction Set (Disabled)

// CONFIG1H
#pragma config FOSC = HS2       // Oscillator (HS oscillator (High power, 16 MHz - 25 MHz))
#pragma config PLLCFG = OFF     // PLL x4 Enable bit (Disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor (Disabled)
#pragma config IESO = OFF       // Internal External Oscillator Switch Over Mode (Disabled)

// CONFIG2L
#pragma config PWRTEN = ON      // Power Up Timer (Enabled)
#pragma config BOREN = OFF      // Brown Out Detect (Disabled in hardware, SBOREN disabled)
#pragma config BORV = 3         // Brown-out Reset Voltage bits (1.8V)
#pragma config BORPWR = ZPBORMV // BORMV Power level (ZPBORMV instead of BORMV is selected)

// CONFIG2H
#pragma config WDTEN = OFF      // Watchdog Timer (WDT disabled in hardware; SWDTEN bit disabled)
#pragma config WDTPS = 1048576  // Watchdog Postscaler (1:1048576)

// CONFIG3H
#pragma config CANMX = PORTB    // ECAN Mux bit (ECAN TX and RX pins are located on RB2 and RB3, respectively)
#pragma config MSSPMSK = MSK7   // MSSP address masking (7 Bit address masking mode)
#pragma config MCLRE = OFF      // Master Clear Enable (MCLR Disabled, RE3 Enabled)

// CONFIG4L
#pragma config STVREN = ON      // Stack Overflow Reset (Enabled)
#pragma config BBSIZ = BB2K     // Boot Block Size (2K word Boot Block size)

// CONFIG5L
#pragma config CP0 = OFF        // Code Protect 00800-01FFF (Disabled)
#pragma config CP1 = OFF        // Code Protect 02000-03FFF (Disabled)
#pragma config CP2 = OFF        // Code Protect 04000-05FFF (Disabled)
#pragma config CP3 = OFF        // Code Protect 06000-07FFF (Disabled)

// CONFIG5H
#pragma config CPB = OFF        // Code Protect Boot (Disabled)
#pragma config CPD = OFF        // Data EE Read Protect (Disabled)

// CONFIG6L
#pragma config WRT0 = OFF       // Table Write Protect 00800-01FFF (Disabled)
#pragma config WRT1 = OFF       // Table Write Protect 02000-03FFF (Disabled)
#pragma config WRT2 = OFF       // Table Write Protect 04000-05FFF (Disabled)
#pragma config WRT3 = OFF       // Table Write Protect 06000-07FFF (Disabled)

// CONFIG6H
#pragma config WRTC = OFF       // Config. Write Protect (Disabled)
#pragma config WRTB = OFF       // Table Write Protect Boot (Disabled)
#pragma config WRTD = OFF       // Data EE Write Protect (Disabled)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protect 00800-01FFF (Disabled)
#pragma config EBTR1 = OFF      // Table Read Protect 02000-03FFF (Disabled)
#pragma config EBTR2 = OFF      // Table Read Protect 04000-05FFF (Disabled)
#pragma config EBTR3 = OFF      // Table Read Protect 06000-07FFF (Disabled)

// CONFIG7H
#pragma config EBTRB = OFF      // Table Read Protect Boot (Disabled)


// ENTRADAS E SAIDAS
#define button_sel_a LATDbits.LATD0
#define button_sel_b LATDbits.LATD1
#define button_sel_c LATDbits.LATD2
#define button_sel_d LATDbits.LATD3
#define button_press PORTDbits.RD4
#define leds_data LATBbits.LATB5
#define leds_clock LATBbits.LATB3
#define leds_strobe LATBbits.LATB2
#define v_switch_clock LATDbits.LATD6
#define v_switch_data LATDbits.LATD5
#define v_switch_strobe LATDbits.LATD7
#define ch_en_clock LATCbits.LATC1
#define ch_en_data LATCbits.LATC0
#define ch_en_strobe LATCbits.LATC2
#define buzzer LATBbits.LATB4
#define reset_button PORTCbits.RC4
#define fan LATCbits.LATC5
#define temp_sense_1 0x01
#define temp_sense_2 0x05
#define temp_sense_3 0x09
#define temp_sense_4 0x0d
#define v_sense_painel 0x11
#define i_sense_painel 0x15
#define v_sense_bateria 0x19
#define i_sense_bateria 0x1d

//ENDERECOS EEPROM
#define first_start_address 0
#define canal_1 1
#define canal_16 16

//VARIAVEIS GLOBAIS
char buffer[128];

//DECLARACOES PROTOTIPOS DE FUNCOES
void config_io(void);
void config_ad(void);
int ler_botoes(void);
void liga_leds(void);
void desliga_leds(void);
void clock_leds(void);
void strobe_leds(void);
void pisca_leds(void);
void escreve_eeprom(int dados, int endereco);
int ler_eeprom(int endereco);
void actualiza_leds(void);
void first_time_init(void);
void actualiza_poe(void);
void ler_reset_sistema(void);
void actualiza_tensoes(void);
void clock_v_switch(void);
void strobe_v_switch(void);
void channels_update(void);
void channels_clock(void);
void channels_strobe(void);
void channels_off(void);
void config_ad(void);
void ler_temperatura(void);


//FUNCOES
void config_io(){
    CCP2CON=0x00;
    CCP3CON=0x00;
    CCP4CON=0x00;
    CCP5CON=0x00;
    
    ANCON0=0x00;
    ANCON1=0x00;
    
    TRISA=0xff;
    TRISB=0x00;
    TRISC=0x00;
    TRISD=0x00;
    TRISE=0x00;
    TRISDbits.TRISD4=1;
    TRISCbits.TRISC4=1;
}

void config_ad(){

    //12 bits-vref 5V=4096
    ANCON0bits.ANSEL0=0x01; //sensor temperatura 1
    ANCON0bits.ANSEL1=0x01; //sensor temperatura 2
    ANCON0bits.ANSEL2=0x01; //sensor temperatura 3
    ANCON0bits.ANSEL3=0x01; //sensor temperatura 4
    ADCON1=0x00;
    ADCON2=0b10001101;
}

void ler_temperatura(){
//4096--- 1mV resoluçao
    float temperatura1,temperatura2,temperatura3,temperatura4;
    int count;
    int count_12bits;
    ADCON0=0x00;
   
    //sensor temperatura 1
    __delay_ms(1);
    
    ADCON0=temp_sense_1;
    ADCON0bits.GO=0x01;
    while(ADCON0bits.DONE==1){}
  
    count=ADRESH;
    count_12bits=((count<<8)|ADRESL);
    
    temperatura1=(count_12bits*5.0)/4095;
    
    //sensor temperatura 2
    __delay_ms(1);
    
    ADCON0=temp_sense_2;
    ADCON0bits.GO=0x01;
    while(ADCON0bits.DONE==1){}
  
    count=ADRESH;
    count_12bits=((count<<8)|ADRESL);
    
    temperatura2=(count_12bits*5.0)/4095;
    
    //sensor temperatura 3
    __delay_ms(1);
    
    ADCON0=temp_sense_3;
    ADCON0bits.GO=0x01;
    while(ADCON0bits.DONE==1){}
  
    count=ADRESH;
    count_12bits=((count<<8)|ADRESL);
    
    temperatura3=(count_12bits*5.0)/4095;
    
    //sensor temperatura 4
    __delay_ms(1);
    
    ADCON0=temp_sense_4;
    ADCON0bits.GO=0x01;
    while(ADCON0bits.DONE==1){}
  
    count=ADRESH;
    count_12bits=((count<<8)|ADRESL);
    
    temperatura4=(count_12bits*5.0)/4095;
    
    if((temperatura1>=0.4)||(temperatura2>=0.4)||(temperatura3>=0.4)||(temperatura4>=0.4)){
        fan=0x01;
        
    }
    else{
       fan=0x00;
        
    }

}
int ler_botoes(){
   
    int botao;
    int i;
    
    button_sel_a=button_sel_b=button_sel_c=button_sel_d=0x00;
    
    for(i=0;i<16;i++){
        
        LATD=i;
        __delay_ms(25);
        
        if(button_press==0){
            botao=i;
            
            while(button_press==0){}
           
            goto exit_button;
        }
        
        
         else{
             botao=0xff;
         }
        
    }
 
    exit_button:
    
    return (botao);
}

void liga_leds(){

    int i;
    leds_data=0x01;
           
           for(i=0;i<32;i++){
            
               leds_clock=0x01;
               __delay_us(1);
               leds_clock=0x00;
               
           }
           
           strobe_leds();
}

void desliga_leds(){

    int i;
    leds_data=0x00;
           
           for(i=0;i<32;i++){
            
               leds_clock=0x01;
               __delay_us(1);
               leds_clock=0x00;
               
           }
           
           strobe_leds();
}


void clock_leds(){

    leds_clock=0x00;
    __delay_us(10);
    leds_clock=0x01;
    __delay_us(10);
    leds_clock=0x00;

}

void strobe_leds(){
  
  leds_strobe=0x00;
  __delay_us(10);  
  leds_strobe=0x01;
  __delay_us(10);
  leds_strobe=0x00;
  
}

void pisca_leds(){

    liga_leds();
    __delay_ms(250);
    desliga_leds();
    __delay_ms(250);
        
}

void clock_v_switch(){

    v_switch_clock=0x00;
    __delay_us(10);
    v_switch_clock=0x01;
    __delay_us(10);
    v_switch_clock=0x00;

}

void strobe_v_switch(){
  
  v_switch_strobe=0x00;
  __delay_us(10);  
  v_switch_strobe=0x01;
  __delay_us(10);
  v_switch_strobe=0x00;
  
}

void channels_update(){

    int i;
    //Bytes MSB
    
    for(i=0;i<17;i++){
        
        if(ler_eeprom(canal_16-i)==0){
            
            ch_en_data=0x00;
            channels_clock();
        }
        
        else if(ler_eeprom(canal_16-i)==48){
        
            ch_en_data=0x01;
            channels_clock();
        }
        
        else if(ler_eeprom(canal_16-i)==24){
        
            ch_en_data=0x01;
            channels_clock();
        }
        
        
    }
    
    channels_strobe();

}

void channels_off(){

     int i;
     ch_en_data=0x00;
           
           for(i=0;i<16;i++){
            
            channels_clock();
               
           }
           
     channels_strobe();
    
}

void channels_clock(){

    ch_en_clock=0x00;
    __delay_us(10);
    ch_en_clock=0x01;
    __delay_us(10);
    ch_en_clock=0x00;

}

void channels_strobe(){

  ch_en_strobe=0x00;
  __delay_us(10);  
  ch_en_strobe=0x01;
  __delay_us(10);
  ch_en_strobe=0x00;
}

void escreve_eeprom(int dados, int endereco){
        
        INTCONbits.GIE=0x00;
        EEADR=endereco;
        endereco=endereco>>8;
        EEADRH=endereco;
        EEDATA=dados;
        EECON1bits.EEPGD=0X00;
        EECON1bits.CFGS=0X00;
        EECON1bits.WREN=0X01;
        EECON2=0X55;
        EECON2=0XAA;
        EECON1bits.WR=1;

    while(EECON1bits.WR==1){

 }

 EECON1bits.WREN=0x00;

}

int ler_eeprom(int endereco){
    
   int valor_armazenado;

        EEADR=endereco;
        endereco=endereco>>8;
        EEADRH=endereco;
        EECON1bits.EEPGD=0x00;
        EECON1bits.CFGS=0x00;
        EECON1bits.RD=0x01;
        valor_armazenado=EEDATA;

    return valor_armazenado;
    __delay_ms(25);

}

void first_time_init(){
       
    int temp,i;
    
       pisca_leds(); 
       
       if(ler_botoes()!=0xff){
           
           liga_leds();
           __delay_ms(3000);
           desliga_leds();
           __delay_ms(3000);
           
           //LIGA LED 1
           
           leds_data=0x01;
           leds_clock=0x01;
           __delay_us(1);
           leds_clock=0x00;
           strobe_leds();
           
           escreve_eeprom(0x0d,first_start_address);
           
           i=0;
           
           for(temp=0;temp<1023;temp++){
               
               leds_data=0x01;
               
               escreve_eeprom(0x00,0x01+temp);
               
               i++;
               
               if(i==32){
                   i=0;
                   leds_clock=0x01;
                   __delay_us(1);
                   leds_clock=0x00;
                   leds_strobe=0x00;  
                   leds_strobe=0x01;
                   __delay_us(1);
                   leds_strobe=0x00;
               
               }
        
           }
       }

    __delay_ms(3000);
    desliga_leds();
}


void actualiza_leds(){

    int i;
    //Bytes MSB: primeiro 48V e depois 24V
   
    //MSB LEDS 48V
    for(i=0;i<17;i++){
        
        if(ler_eeprom(canal_16-i)==0){
            
            leds_data=0x00;
            clock_leds();
        }
        
        else if(ler_eeprom(canal_16-i)==48){
        
            leds_data=0x01;
            clock_leds();
        }
        
        else if(ler_eeprom(canal_16-i)==24){
        
            leds_data=0x00;
            clock_leds();
        }
        
        
    }
  
     //MSB LEDS 24V
    for(i=0;i<17;i++){
        
        if(ler_eeprom(canal_16-i)==0){
            
            leds_data=0x00;
            clock_leds();
        }
        
        else if(ler_eeprom(canal_16-i)==48){
        
            leds_data=0x00;
            clock_leds();
        }
        
        else if(ler_eeprom(canal_16-i)==24){
        
            leds_data=0x01;
            clock_leds();
        }
        
        
    }
    
    strobe_leds();

}


void actualiza_tensoes(){

    int i;
    //Bytes MSB
   
    //MSB TENSOES 
    for(i=0;i<17;i++){
        
        if(ler_eeprom(canal_16-i)==0){
            
            v_switch_data=0x00;
            clock_v_switch();
        }
        
        else if(ler_eeprom(canal_16-i)==48){
        
            v_switch_data=0x01;
            clock_v_switch();
        }
        
        else if(ler_eeprom(canal_16-i)==24){
        
            v_switch_data=0x00;
            clock_v_switch();
        }
        
        
    }
    
    strobe_v_switch();

}


void actualiza_poe(){
    
    int canal_select;
    canal_select=ler_botoes();

    if(canal_select!=255){
 
     
        if(ler_eeprom(canal_1+canal_select)==0){
             
            escreve_eeprom(24,canal_1+canal_select);
    
        }
     
        else if(ler_eeprom(canal_1+canal_select)==24){
                 
            escreve_eeprom(48,canal_1+canal_select);
            
        }
        
        else if(ler_eeprom(canal_1+canal_select)==48){
        
            escreve_eeprom(0,canal_1+canal_select);
            
        }
        
        else {__delay_us(10);}
        
        actualiza_leds();
        actualiza_tensoes();
        channels_update();
        
    }
 
    
    else{
        __delay_us(10);}
     
    
}

void ler_reset_sistema(){

  if(reset_button==0){
      
      __delay_ms(3000);
      
      if(reset_button==0){
          
          buzzer=0x01;
          
          while(reset_button==0){}
          
          escreve_eeprom(0x00,first_start_address);
          __delay_ms(3000);
          asm("RESET");
      }
      
      else{}
     
              
  }
}


void __interrupt() interrupts(void){


}


void main (){
    
    int i;
  
    config_io();
    LATA=0x00;
    LATB=0x00;
    LATC=0x00;
    LATD=0x00;
    buzzer=0x00;
    
    channels_off();
    
    config_ad();
    
    desliga_leds();
    __delay_ms(2000);
    buzzer=0x01;
    liga_leds();
    __delay_ms(2000);
    buzzer=0x00;
    desliga_leds();
    __delay_ms(1000);
    
loop_start:

    i=ler_eeprom(first_start_address);
    
    if(i!=0x0d){
    
        first_time_init();
        goto loop_start; 
    }
    
    else{
        
         __delay_ms(1);      
    
    }

   actualiza_leds();
   actualiza_tensoes();
   channels_update();
   
    while(1){
      
      ler_temperatura();
      actualiza_poe();
      ler_reset_sistema();

    }

}

