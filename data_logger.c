/*
***********************    DATA_LOGGER 8ch   *****************************
by Samuel Parente 2019
**************************************************************************
*/

//enderecos de eeprom externa
//const int canal1=0x00;  //de 0 a 65535

//Variaveis******************************************************************

char main_buffer[256];
float var1=0.0;
float var2=0.0;
float var3=0.0;
float var4=0.0;
float var5=0.0;
float var6=0.0;
float var7=0.0;
float var8=0.0;
char bit_pointer=0;


//nomes de e/s

sbit led_memoria_cheia at latd.b0;
sbit led_wifi_ok at latd.b1;
sbit led_wifi_erro at late.b5;
sbit led_conexao at late.b3;

// Ligacoes do LCD
sbit LCD_RS at RD2_bit;
sbit LCD_EN at RD3_bit;

sbit LCD_D7 at RD7_bit;
sbit LCD_D6 at RD6_bit;
sbit LCD_D5 at RD5_bit;
sbit LCD_D4 at RD4_bit;

sbit LCD_RS_Direction at TRISD2_bit;
sbit LCD_EN_Direction at TRISD3_bit;

sbit LCD_D7_Direction at TRISD7_bit;
sbit LCD_D6_Direction at TRISD6_bit;
sbit LCD_D5_Direction at TRISD5_bit;
sbit LCD_D4_Direction at TRISD4_bit;

// PROTOTIPOS DE FUNÇÕES ******************************************************

void configura_wifi(void);
void liga_tcp(void);
void desliga_tcp(void);
void envia_dados(float valor1,float valor2,float valor3,float valor4,float valor5,float valor6,float valor7,float valor8);
void configura_portas(void);
void limpa_main_buffer(void);
void system_start(void);

// FUNÇÕES *********************************************************************
void configura_portas(){
ancon0=0x00;
ancon1=0x00;
ancon2=0x00;
cm1con=0;
cm2con=0;
cm3con=0;
cvrcon=0;
trisd.b0=0;
trisd.b1=0;
trise.b5=0;
trise.b3=0;
trisc.b7=1;
trisc.b6=0;
odcon1=0x00;
odcon3=0x00;
}

void limpa_main_buffer(){
int i=0;

        for(i=0;i<255;i++){
        main_buffer[i]=0x00;
        }
}


void configura_wifi(){

UART1_Init(9600);
delay_ms(100);

UART1_Write_Text("AT+RST\r\n");
delay_ms(2000);
UART1_Write_Text("ATE0\r\n");
delay_ms(500);
UART1_Write_Text("AT+CWMODE_DEF=1\r\n");
delay_ms(2000);
UART1_Write_Text("AT+CWJAP_DEF=\"MEO-7735F0\",\"b550a1e6af\"\r\n");
delay_ms(6000);
UART1_Write_Text("AT+CIPMUX=1\r\n");
delay_ms(2000);
}


void envia_dados(float valor1,float valor2,float valor3,float valor4,float valor5,float valor6,float valor7,float valor8){

char buffer[256];
char tamanho_envio_string[5];

char valor1_string[16];
char valor2_string[16];
char valor3_string[16];
char valor4_string[16];
char valor5_string[16];
char valor6_string[16];
char valor7_string[16];
char valor8_string[16];


char api_key_string[32];
char mini_buffer[16];
int i=0;

for(i=0;i<255;i++){
buffer[i]=0x00;
}

for(i=0;i<4;i++){
tamanho_envio_string[i]=0x00;
}

for(i=0;i<31;i++){

api_key_string[i]=0x00;
}

for(i=0;i<15;i++){
mini_buffer[i]=0x00;
valor1_string[i]=0x00;
valor2_string[i]=0x00;
valor3_string[i]=0x00;
valor4_string[i]=0x00;
valor5_string[i]=0x00;
valor6_string[i]=0x00;
valor7_string[i]=0x00;
valor8_string[i]=0x00;
}

floattostr(valor1,valor1_string);
floattostr(valor2,valor2_string);
floattostr(valor3,valor3_string);
floattostr(valor4,valor4_string);
floattostr(valor5,valor5_string);
floattostr(valor6,valor6_string);
floattostr(valor7,valor7_string);
floattostr(valor8,valor8_string);

ltrim(valor1_string);
ltrim(valor2_string);
ltrim(valor3_string);
ltrim(valor4_string);
ltrim(valor5_string);
ltrim(valor6_string);
ltrim(valor7_string);
ltrim(valor8_string);

///////////////////////////////////////////
strcpy(api_key_string,"2J7BARB9EA8EIHZ0");
///////////////////////////////////////////

strcpy(buffer,"GET https://api.thingspeak.com/update?api_key=");
strcat(buffer,api_key_string);

strcat(buffer,"&field1=");
strcat(buffer,valor1_string);
strcat(buffer,"&field2=");
strcat(buffer,valor2_string);
strcat(buffer,"&field3=");
strcat(buffer,valor3_string);
strcat(buffer,"&field4=");
strcat(buffer,valor4_string);
strcat(buffer,"&field5=");
strcat(buffer,valor5_string);
strcat(buffer,"&field6=");
strcat(buffer,valor6_string);
strcat(buffer,"&field7=");
strcat(buffer,valor7_string);
strcat(buffer,"&field8=");
strcat(buffer,valor8_string);

strcat(buffer,"\r\n");

UART1_Init(9600);
delay_ms(100);

strcpy(mini_buffer,"AT+CIPSEND=0,");
wordtostr(strlen(buffer),tamanho_envio_string);
ltrim(tamanho_envio_string);

strcat(mini_buffer,tamanho_envio_string);
strcat(mini_buffer,"\r\n");

UART1_Write_Text(mini_buffer);
delay_ms(1250);
UART1_Write_Text(buffer);
delay_ms(1000);

}


void liga_tcp(){

UART1_Init(9600);
delay_ms(100);
UART1_Write_Text("AT+CIPSTART=0,\"TCP\",\"api.thingspeak.com\",80\r\n");
delay_ms(750);
}

void desliga_tcp(){

UART1_Init(9600);
delay_ms(100);

UART1_Write_Text("AT+CIPCLOSE=5\r\n");

}

void system_start(){

Lcd_Init();

delay_ms(100);
Lcd_Cmd(_LCD_CURSOR_OFF);
Lcd_Cmd(_LCD_CLEAR);
Lcd_Out(2,1,"  DATA LOGGER 8 CH  ");
Lcd_Out(3,1,"    Connecting...   ");

configura_wifi();


}

//INTERRUPÇÕES *****************************************************************

//******************************************************************************
void main() {

configura_portas();

system_start();

limpa_main_buffer();
bit_pointer=0;

Lcd_Cmd(_LCD_CLEAR);

Lcd_Out(2,1,"  DATA LOGGER 8 CH  ");
Lcd_Out(3,1,"Thingspeak connected");

delay_ms(3000);

Lcd_Cmd(_LCD_CLEAR);

Lcd_Out(1,1,"CH1:99999|CH5:99999");
Lcd_Out(2,1,"CH2:999.9|CH6:999.9");
Lcd_Out(3,1,"CH3:99.99|CH7:99.99");
Lcd_Out(4,1,"CH4:9.999|CH8:9.999");

while(1){} ;

/*while(1){
var1=var1+0.8;
var2=var1+0.7;
var3=var1+0.6;
var4=var1+0.5;
var5=var1+0.4;
var6=var1+0.3;
var7=var1+0.2;
var8=var1+0.1;

/*liga_tcp();
envia_dados(var1,var2,var3,var4,var5,var6,var7,var8);
delay_ms(15000);

desliga_tcp();*/


}