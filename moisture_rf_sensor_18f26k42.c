/*
Sensor humidade por microondas RF 2.4Ghz
Projetado e desenvolvido por Samuel Parente
2021
*/

//variaveis

//configs do adf4351- 2.4Ghz freq out
/*int register0[4]={0b00000000,0b00110000,0b00000000,0b00000000};
int register1[4]={0b00001000,0b00000000,0b10000000,0b00010001};
int register2[4]={0b00000000,0b00000000,0b01001110,0b01000010};
int register3[4]={0b00000000,0b00000000,0b00000100,0b10110011};
int register4[4]={0b00000000,0b10001100,0b10000010,0b00111100};
int register5[4]={0b00000000,0b01011000,0b00000000,0b00000101};*/

//NEW_
int register0[4]={0b00000000,0b00110010,0b00000000,0b00000000};
int register1[4]={0b00001000,0b00000000,0b10000000,0b00010001};
int register2[4]={0b00000000,0b00000000,0b01001110,0b01000010};
int register3[4]={0b00000000,0b00000000,0b00000100,0b10110011};
int register4[4]={0b00000000,0b10001100,0b10000011,0b01101100};
int register5[4]={0b00000000,0b01011000,0b00000000,0b00000101};



unsigned char buffer_dados[128];


//nomes de pinos de e/s
sbit LED_A at LATA.B2;
sbit LED_B at LATA.B3;

sbit LOCK_DETECT at LATB.B4;

sbit adf4351_ce at LATB.B0;
sbit adf4351_le at LATB.B1;
sbit adf4351_ld at PORTB.B4;
sbit amplitude_in at PORTA.B0;
sbit phase_in at PORTA.B1;

//spi adf4351

sbit SoftSpi_SDI at RB5_bit;
sbit SoftSpi_SDO at RB2_bit;
sbit SoftSpi_CLK at RB3_bit;

sbit SoftSpi_SDI_Direction at TRISB5_bit;
sbit SoftSpi_SDO_Direction at TRISB2_bit;
sbit SoftSpi_CLK_Direction at TRISB3_bit;

//spi ad5681

sbit data_dac at LATC.B7;
sbit clock_dac at LATC.B6;
sbit sync_dac at LATC.B5;
sbit reset_dac at LATC.B4;


//prototipos de funções
void config_comp(void);
void config_adc(void);
void config_io(void);
void config_adf4351(void);
void config_dac(void);
void output_dac(int v_level);
void data_out(char byte_out);
int read_ch(int channel);
void ler_canais(void);
void escreve_usb(unsigned char buffer_envio[]);
void blink(void);

//funções

void config_comp(){

//desliga comparadores
CM1CON0=0x00;
CM2CON0=0x00;

}

void config_adc(){

ADCON0=0x00;
ANSELA=0x00;
ANSELB=0x00;
ANSELC=0x00;

ANSELA=0b00010011;

ADCON0.B2 = 1; //right justify
ADCON0.B4 = 1; //ADCRC Clock

ADCON1=0x00;
ADCON2=0x00;
ADCON3=0x00;
ADREF=0x00;


}



int read_ch(int channel){

  char res_high,res_low;
  int resultado;

  ADPCH=channel;
  ADCON0.B7 = 1; //liga adc
  delay_ms(25);

  ADCON0.B0 = 1; //GO bit (bit ON precisa estar ativo)

  while(ADCON0.B0==1)

        //espera fim conversã0
        {asm nop
        }

   ADCON0.B7 = 0; //desliga ADC

   res_high=ADRESH;
   res_low=ADRESL;
   resultado = (res_high << 8) | res_low ;

   return resultado;


}

void config_io(){

TRISA=0x00;
TRISB=0x00;
TRISC=0x00;

//entradas
TRISB.B4=0x01;  //lock detect adf4351
TRISA.B0=0x01; //amplitude in ad8302
TRISA.B1=0x01; // phase in ad8302
TRISA.B4=0x01; // ajuste in

//estado inicial
LATA=0x00;
LATB=0x00;
LATC=0x00;

}




void config_adf4351(){
int i=0x00;
//00000000 01000000 00000000 00000000   r0
//00001000 00000000 10000000 00010001   r1
//00000000 00000000 01001110 01000010   r2
//00000000 00000000 00000100 10110011   r3
//00000000 11101100 10000010 00101100   r4
//00000000 01011000 00000000 00000101   r5

//adf4351_ce=0x01;
//adf4351_le=0x00;

adf4351_ce=0x00;
adf4351_le=0x00;

Soft_SPI_Init();
delay_ms(10);

//r0
  for (i=0;i<4;i++){
  Soft_SPI_Write(register0[i]);
  }

  delay_ms(5);
  adf4351_le=0x01;
  delay_ms(5);
  adf4351_le=0x00;
  delay_ms(5);

//r1
  for (i=0;i<4;i++){
  Soft_SPI_Write(register1[i]);
  }

  delay_ms(5);
  adf4351_le=0x01;
  delay_ms(5);
  adf4351_le=0x00;
  delay_ms(5);
  
//r2
  for (i=0;i<4;i++){
  Soft_SPI_Write(register2[i]);
  }

  delay_ms(5);
  adf4351_le=0x01;
  delay_ms(5);
  adf4351_le=0x00;
  delay_ms(5);
  
//r3
  for (i=0;i<4;i++){
  Soft_SPI_Write(register3[i]);
  }

  delay_ms(5);
  adf4351_le=0x01;
  delay_ms(5);
  adf4351_le=0x00;
  delay_ms(5);
  
//r4
  for (i=0;i<4;i++){
  Soft_SPI_Write(register4[i]);
  }

  delay_ms(5);
  adf4351_le=0x01;
  delay_ms(5);
  adf4351_le=0x00;
  delay_ms(5);
  
//r5
  for (i=0;i<4;i++){
  Soft_SPI_Write(register5[i]);
  }

  delay_ms(5);
  adf4351_le=0x01;
  delay_ms(5);
  adf4351_le=0x00;
  delay_ms(5);

}

void config_dac(){

//control bytes
char byte3_control=0b01000000;
char byte2_control=0b10000000;
char byte1_control=0b00000000;


sync_dac=0x01;
delay_ms(25);
reset_dac=0x01;
delay_ms(25);

sync_dac=0x00;
delay_ms(25);

data_out(byte3_control);
data_out(byte2_control);
data_out(byte1_control);

delay_ms(25);
sync_dac=0x01;
delay_ms(25);

}


void output_dac(int v_level){

//vout=gain x vref x (valor decimal/4096)
//0 0 1 1 DB15 DB14 DB13 DB12 DB11 DB10 DB9…DB4
//0011xxxx xxxxxxxx

char high_byte,low_byte;

low_byte=v_level;
v_level=v_level>>8;
high_byte=v_level;
high_byte.B7=0x00;
high_byte.B6=0x00;
high_byte.B5=0x01;
high_byte.B4=0x01;
 
sync_dac=0x00;
delay_ms(1);

data_out(high_byte);
data_out(low_byte);

delay_ms(1);
sync_dac=0x01;
delay_ms(5);

}

void data_out(char byte_out){
char i;

for(i=0;i<8;i++){

data_dac=byte_out.B7;
clock_dac =0x01;
delay_ms(1);
clock_dac =0x00;
delay_ms(1);
clock_dac =0x01;
delay_ms(1);

byte_out=byte_out<<1;

}



}

void escreve_usb(unsigned char buffer_envio[]){
unsigned int i;

for(i=0;i<strlen(buffer_envio);i++){

        soft_uart_write(buffer_envio[i]);
    }


}

void ler_canais(){

int phase_raw;
float phase_v;
float phase;

int amplitude_raw;
float amplitude_v;
float amplitude;

int ajuste_raw;
float ajuste_v;
float ajuste;

unsigned int temp;
unsigned int error;


if (LOCK_DETECT==1){
escreve_usb("LOCK ON ");
}
else{
escreve_usb("NO LOCK ");
}

/*//calibração
    ajuste_raw=read_ch(4);
    ajuste_v=((ajuste_raw*3300.0)/4095);
    ajuste=ajuste_v/1000;
 sprintf(buffer_dados, "% 5.3Lf", ajuste);

         ltrim(buffer_dados);

         escreve_usb(buffer_dados);
         escreve_usb("Volts ");
         delay_ms(500);*/
  ///////////////////////////////////////////////////////////////////////
 
//amplitude
    amplitude_raw=read_ch(0);
    amplitude_v=((amplitude_raw*3300.0)/4095);

    if (amplitude_v==900){

         escreve_usb("0 dB ");

    }

   else if (amplitude_v>900.0){
      amplitude=(amplitude_v/930.0);

      sprintf(buffer_dados, "% 5.3Lf", amplitude);

         ltrim(buffer_dados);
         escreve_usb("+");
         escreve_usb(buffer_dados);
         escreve_usb("dB ");
    }

   else  if (amplitude_v<900.0){
    
      amplitude=(amplitude_v/930.0);
                     sprintf(buffer_dados, "%12f", amplitude);

         ltrim(buffer_dados);
         escreve_usb("-");
         escreve_usb(buffer_dados);
         escreve_usb("dB ");
    }

    else{
    ;
    }

    
//fase
    phase_raw=read_ch(1);
      phase_v=(phase_raw*3300.0)/4095;
      
    if (phase_v>=1800.0){

         escreve_usb("0");   //extremo menor

    }

    else if (phase_v==0.0){

         escreve_usb("-180");  //extremo maior
    }

   else if ((phase_v>0.0) && (phase_v<1800.0)){

       phase=(phase_v-1800)/(-10.0);
      sprintf(buffer_dados, "% 5.3Lf", phase);

         ltrim(buffer_dados);

         escreve_usb(buffer_dados);


    }


    else{
    ;
    }
     soft_uart_write(13);
     
     

}

void blink(){
char i;

  for(i=0;i<6;i++){
  LED_A=!LED_A;
    delay_ms(250);
  }


}


void main() {
unsigned int error;

//uc boot up and config
delay_ms(50);

config_io();
delay_ms(25);

config_comp();
delay_ms(25);

reset_dac=0x00;
//adf4351_ce=0x00;

adf4351_ce=0x01;

config_adc();
delay_ms(25);

reset_dac=0x01;

//adf4351_ce=0x01;
adf4351_ce=0x00;


config_adf4351();
delay_ms(50);

config_dac();
delay_ms(50);

LED_A=0x00;
LED_B=0x00;


error = Soft_UART_Init(&PORTC, 1, 0, 9600, 0);
delay_ms(25);

blink();
LED_A=0x01;

//main loop


         
        while(1){

        ler_canais();
        LED_B=!LED_B;
        
        
             //delay_ms(250);
            //  output_dac(0);
           delay_ms(500);
         /*output_dac(255);
           delay_ms(2000);
             output_dac(512);
           delay_ms(2000);
             output_dac(1024);
           delay_ms(2000);
             output_dac(2048);
           delay_ms(2000);
           output_dac(4095);
           delay_ms(2000);*/

        }
 
}