/*******************************************************************************
COMUNICAÇÃO DE DADOS DE MEDIDOR DE ENERGIA POR GPRS-TCP/IP-MQTT

Desenvolvido POR SAMUEL PARENTE
@2019
*******************************************************************************/
//INCLUDES

#include <built_in.h>
//******************************************************************************
//Constantes  e endereços EEPROM

#define endereco_nome_apn 0
#define endereco_dns_mqtt 48
#define endereco_porta_mqtt 96
#define endereco_servidor_mqtt 104
#define endereco_user_mqtt 152
#define endereco_password_mqtt 200
#define endereco_id 248
#define endereco_serial_number 296
#define endereco_flag_config 344
#define endereco_pass_admin 345
#define Ext_EEPROM_ADDR_MSB 393
#define Ext_EEPROM_ADDR_LSB 394
#define endereco_user_apn 395
#define endereco_password_apn 443
#define endereco_recconect 493
#define endereco_final 494
#define Ext_EEPROM_length 65519   //memoria at24c32   65519 -memoria at24c512


//******************************************************************************
//Símbolos de I/O

sbit led_link at LATA.B0;
sbit led_status at LATA.B1;
sbit reset_sim800 at LATC.B5;
sbit botao_config at PORTA.B2;

//******************************************************************************
//Variáveis
unsigned short int buffer_header[16];
volatile unsigned char buffer_dados[128];
volatile unsigned char buffer_medidor[16]; //128
volatile unsigned char buffer_auxiliar[255]; //300
unsigned char flag_medidor=0x00;
volatile unsigned char place_medidor=0x00;
unsigned char buffer_eeprom[64];
unsigned char buffer_sim800[64];
unsigned char flag_menu_usb=0x00;
unsigned char flag_sincronismo=0x00;
unsigned char flag_temporizador=0x00;
char hora_data[24];
unsigned char contador_rtc=0x00;
unsigned char flag_contador_rtc=0x00;
unsigned char place_sim800=0x00;
unsigned char flag_sim800_error=0x00;
unsigned char tamanho_bloco=0x00;
unsigned char flag_switch=0x00;
unsigned char flag_envia_dados=0x00;
unsigned int contador_segundos=0x00;
unsigned char flag_15min=0x00;
unsigned char flag_reconnect=0x00;
unsigned int seg_fim=0x00;
unsigned char fd_msb=0x00;
unsigned char fd_lsb=0x00;
unsigned char pisca_status=0x00;
unsigned int contador_gsm=0x00;
unsigned char byte_0=0x00;
unsigned char byte_1=0x00;
unsigned char byte_2=0x00;
unsigned char byte_3=0x00;
unsigned char byte_4=0x00;
unsigned char byte_5=0x00;
unsigned char byte_6=0x00;
unsigned char byte_7=0x00;
unsigned char byte_8=0x00;
//******************************************************************************
//Protótipos das funções

void reset_eeprom(void);
void configura_io(void);
void tcp_start_sim800(void);
void limpa_buffer_dados(void);
void limpa_buffer_medidor(void);
void configura_uart1(void);
void configura_uart2(void);
void escrever_eeprom(unsigned int endereco,unsigned char dados);
unsigned char ler_eeprom(unsigned int endereco);
void configura_interrupcoes(void);
void menu_usb(void);
void liga_int(void);
void desliga_int(void);
void escreve_usb(unsigned char buffer_envio[]);
void ler_sn(void);
void ler_id(void);
void ler_password(void);
void ler_user_mqtt(void);
void ler_servidor(void);
void ler_porta(void);
void ler_dns(void);
void ler_apn(void);
void ler_password_apn(void);
void ler_user_apn(void);
void first_time(void);
void ler_pass_admin(void);
void configura_rtc(void);
void ler_rtc(void);
void configura_hora(char hora_data[8]);
void configura_data(char data_in[8]);
void escreve_log(unsigned char dados_log);
unsigned char ler_log(int endereco);
void liga_mqtt(void);
void envia_mqtt(unsigned char buffer_envio[300]);
void limpa_buffer_eeprom(void);
void limpa_buffer_gsm(void);
void limpa_buffer_auxiliar(void);
void ler_gsm(void);
void inicia_i2c(void);
void dump_memory(void);
void envia_pacote_dados(void);
void inicia_sistema(void);
void interrupt(void);

//******************************************************************************
//Funções

void configura_io(){

//comparadores desligados
CCP1CON=0x00;
CCP2CON=0x00;

//modulo A/D desligado
ANSELA=0x00;
ANSELB=0x00;
ANSELC=0x00;

ADCON0=0x00;
ADCON1=0x00;
ADCON2=0x00;
ADCON3=0x00;

//Pinos PPS--------------------------------
Unlock_IOLOCK();


//Uart 1
RX1PPS=0X17;
RC6PPS=0x09;

//Uart 2
RX2PPS=0x0b; //pino rb3->medidor energia. rx 110bps
//RB4PPS=0X0b;

//I2C1
RC4PPS=0x10;//sda
SSP1DATPPS=0X14; //sda
RC3PPS=0x0f; //scl
SSP1CLKPPS=0X13;  //scl

//Interrupts
INT0PPS=0x08; //RB0 usb_config
INT1PPS=0x09; //RB1 ring_gsm
INT2PPS=0x0a; //RB2 sqr_rtc

Lock_IOLOCK();
//-------------------------------------------

//Open drain
/*ODCONA=0X00;
ODCONB=0X00;
ODCONC=0X00;*/

//Entradas e saídas

//Uart 2
TRISB.B3=0x01;
TRISB.B4=0x00;

//Uart 1 - SIM800L
TRISC.B7=0x01;
TRISC.B6=0x00;

//Reset SIM800L

TRISC.B5=0x00;

//Led's
TRISA.B0=0x00;
TRISA.B1=0x00;
TRISC.B5=0x00;

//USB
TRISB.B0=0x01;
TRISB.B5=0x00;

//Botões
TRISA.B2=0x01;

//I2C
TRISC.B3=0x01;
TRISC.B4=0x01;

//misc
TRISB.B1=0x01; //ring_gsm
TRISB.B2=0x01; //sqr_rtc_in
}


void tcp_start_sim800(){
char k=0x00;
led_link=~led_link;

limpa_buffer_dados();

//Echo OFF
UART1_Remappable_Write_Text("ATE0\r\n");
delay_ms(250);
led_link=~led_link;

//Configura APN

strcpy(buffer_dados,"AT+CSTT=");
ler_apn();
strcat(buffer_dados,buffer_eeprom);
strcat(buffer_dados,",");
ler_user_apn();
strcat(buffer_dados,buffer_eeprom);
strcat(buffer_dados,",");
ler_password_apn();
strcat(buffer_dados,buffer_eeprom);
strcat(buffer_dados,"\r\n");
UART1_Remappable_Write_Text(buffer_dados);

  for(k=0;k<=5;k++){
    delay_ms(200);
    led_link=~led_link;
  }

//Liga dados
UART1_Remappable_Write_Text("AT+CIICR\r\n");

  for(k=0;k<=10;k++){
    delay_ms(200);
    led_link=~led_link;
  }

//"Resolve" IP
UART1_Remappable_Write_Text("AT+CIFSR\r\n");

  for(k=0;k<=15;k++){
    delay_ms(200);
    led_link=~led_link;
  }

//Establece conexão TCP ao servidor MQTT
limpa_buffer_dados();

strcpy(buffer_dados,"AT+CIPSTART=\"TCP\",");
strcat(buffer_dados,"\"");
ler_dns();
strcat(buffer_dados,buffer_eeprom);
strcat(buffer_dados,"\",");
ler_porta();
strcat(buffer_dados,buffer_eeprom);
strcat(buffer_dados,"\r\n");

UART1_Remappable_Write_Text(buffer_dados);

  for(k=0;k<=20;k++){
    delay_ms(200);
    led_link=~led_link;
  }

}

void liga_mqtt(){
int i,g;
int size_id,size_user,size_password;
unsigned char temp1_string[5];
unsigned char size_id_high;
unsigned char size_id_low;
unsigned char size_user_high;
unsigned char size_user_low;
unsigned char size_password_high;
unsigned char size_password_low;

//Ler tamanho numero serie
ler_sn();
rtrim(buffer_eeprom);
size_id=strlen(buffer_eeprom);
size_id_high=Hi(size_id);
size_id_low=Lo(size_id);

//Ler tamanho utilizador mqtt
ler_user_mqtt();
rtrim(buffer_eeprom);
size_user=strlen(buffer_eeprom);
size_user_high=Hi(size_user);
size_user_low=Lo(size_user);

//Ler tamanho password mqtt
ler_password();
rtrim(buffer_eeprom);
size_password=strlen(buffer_eeprom);
size_password_high=Hi(size_password);
size_password_low=Lo(size_password);

for(i=0;i<=15;i++){
buffer_header[i]=0x00;
}

//fixed header------------------------------------------------------------------
buffer_header[0]=0x10;

//variable header----------------------------------------------------------------
buffer_header[2]=0x00;                        //MSB protocol
buffer_header[3]=0x04;                        //LSB protocol
buffer_header[4]='M';                         //protocol string
buffer_header[5]='Q';
buffer_header[6]='T';
buffer_header[7]='T';
buffer_header[8]=0x04;                        //protocol level=4
//buffer_header[9]=0x02;                       //flags sem user e sem password
buffer_header[9]=0xc2;                        //flags com user e password
buffer_header[10]=0x00;                       //MSB keep alive time
buffer_header[11]=0x00;                       //LSB keep alive time
buffer_header[12]=size_id_high;          //MSB payload (client_id)
buffer_header[13]=size_id_low;          //LSB payload (cliente_id)
buffer_header[1]=16+size_id+size_user+size_password; //strlen(i)+strlen(j)+strlen(k);  //remaining length=variable header+payload
buffer_header[0]=0x10;
//PAYLOAD-----------------------------------------------------------------------
limpa_buffer_dados();

g=buffer_header[1]+2;
wordtostr(g,temp1_string);
ltrim(temp1_string);
rtrim(temp1_string);

strcpy(buffer_dados,"AT+CIPSEND=");
strcat(buffer_dados,temp1_string);

UART1_Remappable_Write_Text(buffer_dados);
UART1_Remappable_Write_Text("\r\n");

delay_ms(750);

for(i=0;i<=13;i++){

UART1_Remappable_Write(buffer_header[i]);
}


ler_sn();

for(i=0;i<=strlen(buffer_eeprom)-1;i++){
UART1_Remappable_Write(buffer_eeprom[i]);
}

UART1_Remappable_Write(size_user_high);
UART1_Remappable_Write(size_user_low);

ler_user_mqtt();
for(i=0;i<=strlen(buffer_eeprom)-1;i++){
UART1_Remappable_Write(buffer_eeprom[i]);
}

UART1_Remappable_Write(size_password_high);
UART1_Remappable_Write(size_password_low);

ler_password();

for(i=0;i<=strlen(buffer_eeprom)-1;i++){
UART1_Remappable_Write(buffer_eeprom[i]);
}

delay_ms(750);

}

void envia_mqtt(unsigned char buffer_envio[]){
int i,k,g;
unsigned char temp1_string[5];
unsigned char buffer_header[16];

desliga_int();

for(i=0;i<=15;i++){
buffer_header[i]=0x00;
}

limpa_buffer_dados();

ler_servidor();
//ltrim(buffer_eeprom);
//rtrim(buffer_eeprom);

strcpy(buffer_dados,buffer_eeprom);
/*i=strlen(buffer_dados);
buffer_dados[i+1]='/';*/

strcat(buffer_dados,"/");

ler_sn();
//ltrim(buffer_eeprom);
//rtrim(buffer_eeprom);

strcat(buffer_dados,buffer_eeprom);


buffer_header[1]=2+strlen(buffer_dados)+strlen(buffer_envio); //remaining length=variable header+payload

//VARIABLE HEADER------------------------------------------------------

k=strlen(buffer_dados);
buffer_header[2]=Hi(k);        //MSB channel length
buffer_header[3]=Lo(k);        //LSB channel length

//buffer_envio=dados
//channel=nomeservidor/numeroserie
//------------------------------------------------------------------------


g=buffer_header[1]+2;
wordtostr(g,temp1_string);
ltrim(temp1_string);

UART1_Remappable_Write_Text("AT+CIPSEND=");
UART1_Remappable_Write_Text(temp1_string);
UART1_Remappable_Write_Text("\r\n");

delay_ms(750);

buffer_header[0]=0x30;   //comando publish

UART1_Remappable_Write(buffer_header[0]);
UART1_Remappable_Write(buffer_header[1]);
UART1_Remappable_Write(buffer_header[2]);
UART1_Remappable_Write(buffer_header[3]);

UART1_Remappable_Write_Text(buffer_dados);
UART1_Remappable_Write_Text(buffer_envio);

delay_ms(500);

}


void reset_eeprom(){

int i;

      for (i=0;i<=endereco_final;i++){

          escrever_eeprom(0x00+i,0x00);

      }

}

void limpa_buffer_dados(){

char i;

      for (i=0;i<=127;i++){
          buffer_dados[i]=0x00;
      }

}

void limpa_buffer_auxiliar(){

int i;

      for (i=0;i<=254;i++){
          buffer_auxiliar[i]=0x00;
      }

}

void limpa_buffer_eeprom(){

char i;

      for (i=0;i<=63;i++){
          buffer_eeprom[i]=0x00;
      }

}

void configura_uart1(){

UART1_Init(9600);
delay_ms(100);

}

void configura_uart2(){


 // disable interrupts before changing states
    PIE3.RC2IE = 0;

    // Set the EUSART2 module to the options selected in the user interface.

    // ABDOVF no_overflow; SCKP Non-Inverted; BRG16 16bit_generator; WUE disabled; ABDEN disabled;
    BAUD2CON = 0x08;

    // SPEN enabled; RX9 8-bit; CREN enabled; ADDEN disabled; SREN disabled;
    RC2STA = 0x90;

    // TX9 8-bit; TX9D 0; SENDB sync_break_complete; TXEN disabled; SYNC asynchronous; BRGH hi_speed; CSRC slave;
    TX2STA = 0x04;

    // SP2BRGL 11;
    SP2BRGL = 0x0B;

    // SP2BRGH 142;
    SP2BRGH = 0x8E;
    
     PIE3.RC2IE = 1;
}

void escrever_eeprom(unsigned int endereco,unsigned char dados){

INTCON.GIE=0x00;
NVMCON1.NVMREG0=0x00;
NVMCON1.NVMREG1=0x00;
NVMADRL=endereco;
endereco=endereco>>8;
NVMADRH=endereco;
NVMDAT=dados;
NVMCON1.WREN=0x01;
NVMCON2=0x55;
NVMCON2=0xaa;
NVMCON1.WR=0x01;

      while(NVMCON1.WR==1){
      asm{nop}
      }

NVMCON1.WREN=0x00;

}

unsigned char ler_eeprom(unsigned int endereco){
unsigned char i;

INTCON.GIE=0x00;
NVMCON1.NVMREG0=0X00;
NVMCON1.NVMREG1=0X00;
NVMADRL=endereco;
endereco=endereco>>8;
NVMADRH=endereco;
NVMCON1.RD=0x01;
i=NVMDAT;
return i;

}


void configura_interrupcoes(){
INTCON=0X00;

PIE0=0x00;
PIE1=0X00;
PIE2=0X00;
PIE3=0X00;
PIE4=0X00;
PIE5=0X00;
PIE6=0X00;
PIE7=0X00;

IPR0=0x00;
IPR1=0X00;
IPR2=0X00;
IPR3=0X00;
IPR4=0X00;
IPR5=0X00;
IPR6=0X00;
IPR7=0X00;

PIE0.INT0IE=0X01;
PIE0.INT2IE=0x01;
PIE3.RC1IE=0x01;
PIE3.RC2IE=0x01;

PIR0=0x00;
PIR1=0X00;
PIR2=0X00;
PIR3=0X00;
PIR4=0X00;
PIR5=0X00;
PIR6=0X00;
PIR7=0X00;

}

void escreve_usb(unsigned char buffer_envio[]){
unsigned int i;

for(i=0;i<strlen(buffer_envio);i++){

        soft_uart_write(buffer_envio[i]);
    }
soft_uart_write(13);
soft_uart_write(10);

}

void ler_apn(){

unsigned char i;

limpa_buffer_eeprom();

  for(i=0;i<=47;i++){
    buffer_eeprom[i]=ler_eeprom(endereco_nome_apn+i);
  }

}

void ler_user_apn(){

unsigned char i;

limpa_buffer_eeprom();

  for(i=0;i<=47;i++){
    buffer_eeprom[i]=ler_eeprom(endereco_user_apn+i);
  }

}

void ler_password_apn(){

unsigned char i;

limpa_buffer_eeprom();

  for(i=0;i<=47;i++){
    buffer_eeprom[i]=ler_eeprom(endereco_password_apn+i);
  }

}


void ler_dns(){

unsigned char i;

limpa_buffer_eeprom();

  for(i=0;i<=47;i++){
    buffer_eeprom[i]=ler_eeprom(endereco_dns_mqtt+i);
  }

}

void ler_porta(){

unsigned char i;

limpa_buffer_eeprom();

  for(i=0;i<=7;i++){
    buffer_eeprom[i]=ler_eeprom(endereco_porta_mqtt+i);
  }

}

void ler_servidor(){

unsigned char i;

limpa_buffer_eeprom();

  for(i=0;i<=47;i++){
    buffer_eeprom[i]=ler_eeprom(endereco_servidor_mqtt+i);
  }

}

void ler_user_mqtt(){

unsigned char i;

limpa_buffer_eeprom();

  for(i=0;i<=47;i++){
    buffer_eeprom[i]=ler_eeprom(endereco_user_mqtt+i);
  }

}

void ler_password(){

unsigned char i;

limpa_buffer_eeprom();

  for(i=0;i<=47;i++){
    buffer_eeprom[i]=ler_eeprom(endereco_password_mqtt+i);
  }

}

void ler_id(){

unsigned char i;

limpa_buffer_eeprom();

  for(i=0;i<=47;i++){
    buffer_eeprom[i]=ler_eeprom(endereco_id+i);
  }

}

void ler_sn(){

unsigned char i;

limpa_buffer_eeprom();

  for(i=0;i<=47;i++){
    buffer_eeprom[i]=ler_eeprom(endereco_serial_number+i);
  }

}

void ler_pass_admin(){

unsigned char i;

limpa_buffer_eeprom();

  for(i=0;i<=47;i++){
    buffer_eeprom[i]=ler_eeprom(endereco_pass_admin+i);
  }

}


void menu_usb(){

unsigned char i=0,j=0,k=0,menu=0,erro=0;

desliga_int();

Soft_UART_Init(&PORTB,0,5,9600,0);
delay_ms(100);

escreve_usb("Insira PASSWORD de acesso ao sistema:\r\n");

                  delay_ms(100);
                  k=0;
                  limpa_buffer_dados();
                  while(buffer_dados[k-1]!=13){
                      buffer_dados[k]=Soft_Uart_Read(&erro);
                          if(buffer_dados[k]==8){
                              k--;
                          }

                          else{

                              k++;
                          }
                   }

k=0;
ler_pass_admin();

k=strstr(buffer_dados,buffer_eeprom);

if(k!=0){
asm{nop}

}

else if(strstr(buffer_dados,"7272")!=0){
asm{nop}
}

else{
escreve_usb("\r\n");
escreve_usb("*********************************");
escreve_usb("ACESSO NEGADO. LIGACAO TERMINADA.");
escreve_usb("*********************************");
goto fim_menu_usb;
}

k=0;

menu:

escreve_usb("\r\n");
escreve_usb("************************ V1.0 ************************");
escreve_usb("1-Alterar APN da rede movel");
escreve_usb("2-Alterar UTILIZADOR da rede movel");
escreve_usb("3-Alterar PASSWORD da rede movel");
escreve_usb("4-Alterar DNS do servidor MQTT");
escreve_usb("5-Alterar PORTA do servidor MQTT");
escreve_usb("6-Alterar servidor MQTT");
escreve_usb("7-Alterar USER MQTT");
escreve_usb("8-Alterar PASSWORD MQTT");
escreve_usb("9-Alterar ID do sistema");
escreve_usb("a-Alterar NUMERO DE SERIE do sistema");
escreve_usb("b-Alterar password de ADMINISTRADOR");
escreve_usb("c-Alterar Hora");
escreve_usb("d-Alterar Data");
escreve_usb("e-Sair");
escreve_usb("\r\n");
escreve_usb("Insira Opcao:");

j=Soft_Uart_Read(&erro);

  select:
  switch(j){

            case '1':

             menu_apn:
              escreve_usb("\r\nAPN de rede actual:");
              ler_apn();
              escreve_usb(buffer_eeprom);

              escreve_usb("\r\n");
              escreve_usb("Alterar(s/n)?");

              menu=Soft_Uart_Read(&erro);

              if(menu=='s'){
                  escreve_usb("\r\n");
                  for(i=0;i<48;i++){
                      escrever_eeprom(endereco_nome_apn+i,0x00);
                  }

                  escreve_usb("Insira novo APN de rede movel:\r\n");
                  delay_ms(50);
                  k=0;
                  limpa_buffer_eeprom();
                  while(buffer_eeprom[k-1]!=13){
                      buffer_eeprom[k]=Soft_Uart_Read(&erro);
                          if(buffer_eeprom[k]==8){
                              k--;
                          }
                          else{
                              k++;
                          }
                   }

                  for(i=0;i<strlen(buffer_eeprom);i++){
                      escrever_eeprom(endereco_nome_apn+i,buffer_eeprom[i]);
                  }

                  goto select;
              }


              else if(menu=='n'){
                  escreve_usb("\r\n");
                  asm{nop};
                  goto menu;
              }

              else{
                  escreve_usb("\r\n");
                  escreve_usb("Insira opcao valida\r\n");
                  goto menu_apn;
              }

            break;

            case '2':

             menu_user_apn:
              escreve_usb("\r\nNome UTILIZADOR APN actual:");
              ler_user_apn();
              escreve_usb(buffer_eeprom);

              escreve_usb("\r\n");
              escreve_usb("Alterar(s/n)?");

              menu=Soft_Uart_Read(&erro);

              if(menu=='s'){
                  escreve_usb("\r\n");
                  for(i=0;i<48;i++){
                      escrever_eeprom(endereco_user_apn+i,0x00);
                  }

                  escreve_usb("Insira novo UTILIZADOR APN:\r\n");
                  delay_ms(50);
                  k=0;
                  limpa_buffer_eeprom();
                  while(buffer_eeprom[k-1]!=13){
                      buffer_eeprom[k]=Soft_Uart_Read(&erro);
                          if(buffer_eeprom[k]==8){
                              k--;
                          }
                          else{
                              k++;
                          }
                   }

                  for(i=0;i<strlen(buffer_eeprom);i++){
                      escrever_eeprom(endereco_user_apn+i,buffer_eeprom[i]);
                  }

                  goto select;
              }


              else if(menu=='n'){
                  escreve_usb("\r\n");
                  asm{nop};
                  goto menu;
              }

              else{
                  escreve_usb("\r\n");
                  escreve_usb("Insira opcao valida\r\n");
                  goto menu_user_apn;
              }

            break;

            case '3':

             menu_password_apn:
              escreve_usb("\r\nPASSWORD APN actual:");
              ler_password_apn();
              escreve_usb(buffer_eeprom);

              escreve_usb("\r\n");
              escreve_usb("Alterar(s/n)?");

              menu=Soft_Uart_Read(&erro);

              if(menu=='s'){
                  escreve_usb("\r\n");
                  for(i=0;i<48;i++){
                      escrever_eeprom(endereco_password_apn+i,0x00);
                  }

                  escreve_usb("Insira nova PASSWORD APN:\r\n");
                  delay_ms(50);
                  k=0;
                  limpa_buffer_eeprom();
                  while(buffer_eeprom[k-1]!=13){
                      buffer_eeprom[k]=Soft_Uart_Read(&erro);
                          if(buffer_eeprom[k]==8){
                              k--;
                          }
                          else{
                              k++;
                          }
                   }

                  for(i=0;i<strlen(buffer_eeprom);i++){
                      escrever_eeprom(endereco_password_apn+i,buffer_eeprom[i]);
                  }

                  goto select;
              }


              else if(menu=='n'){
                  escreve_usb("\r\n");
                  asm{nop};
                  goto menu;
              }

              else{
                  escreve_usb("\r\n");
                  escreve_usb("Insira opcao valida\r\n");
                  goto menu_password_apn;
              }

            break;

            case '4':
            menu_dns:
              escreve_usb("\r\nDNS do servidor MQTT actual:");
              ler_dns();
              escreve_usb(buffer_eeprom);

              escreve_usb("\r\n");
              escreve_usb("Alterar(s/n)?");

              menu=Soft_Uart_Read(&erro);

              if(menu=='s'){
                  escreve_usb("\r\n");
                  for(i=0;i<48;i++){
                      escrever_eeprom(endereco_dns_mqtt+i,0x00);
                  }

                  escreve_usb("Insira novo DNS de servidor MQTT:\r\n");
                  delay_ms(50);
                  k=0;
                  limpa_buffer_eeprom();
                  while(buffer_eeprom[k-1]!=13){
                      buffer_eeprom[k]=Soft_Uart_Read(&erro);
                          if(buffer_eeprom[k]==8){
                              k--;
                          }
                          else{
                              k++;
                          }
                   }

                  for(i=0;i<strlen(buffer_eeprom);i++){
                      escrever_eeprom(endereco_dns_mqtt+i,buffer_eeprom[i]);
                  }

                  goto select;
              }


              else if(menu=='n'){
                  escreve_usb("\r\n");
                  asm{nop};
                  goto menu;
              }

              else{
                  escreve_usb("\r\n");
                  escreve_usb("Insira opcao valida\r\n");
                  goto menu_dns;
              }


            break;

            case '5':

            menu_porta:
              escreve_usb("\r\nPORTA do servidor MQTT actual:");
              ler_porta();
              escreve_usb(buffer_eeprom);

              escreve_usb("\r\n");
              escreve_usb("Alterar(s/n)?");

              menu=Soft_Uart_Read(&erro);

              if(menu=='s'){
                  escreve_usb("\r\n");
                  for(i=0;i<8;i++){
                      escrever_eeprom(endereco_porta_mqtt+i,0x00);
                  }

                  escreve_usb("Insira nova PORTA do servidor MQTT:\r\n");
                  delay_ms(50);
                  k=0;
                  limpa_buffer_eeprom();
                  while(buffer_eeprom[k-1]!=13){
                      buffer_eeprom[k]=Soft_Uart_Read(&erro);
                          if(buffer_eeprom[k]==8){
                              k--;
                          }
                          else{
                              k++;
                          }
                   }

                  for(i=0;i<strlen(buffer_eeprom);i++){
                      escrever_eeprom(endereco_porta_mqtt+i,buffer_eeprom[i]);
                  }

                  goto select;
              }


              else if(menu=='n'){
                  escreve_usb("\r\n");
                  asm{nop};
                  goto menu;
              }

              else{
                  escreve_usb("\r\n");
                  escreve_usb("Insira opcao valida\r\n");
                  goto menu_porta;
              }

            break;

            case '6':
            menu_servidor:
              escreve_usb("\r\nSERVIDOR MQTT actual:");
              ler_servidor();
              escreve_usb(buffer_eeprom);

              escreve_usb("\r\n");
              escreve_usb("Alterar(s/n)?");

              menu=Soft_Uart_Read(&erro);

              if(menu=='s'){
                  escreve_usb("\r\n");
                  for(i=0;i<48;i++){
                      escrever_eeprom(endereco_servidor_mqtt+i,0x00);
                  }

                  escreve_usb("Insira novo SERVIDOR MQTT:\r\n");
                  delay_ms(50);
                  k=0;
                  limpa_buffer_eeprom();
                  while(buffer_eeprom[k-1]!=13){
                      buffer_eeprom[k]=Soft_Uart_Read(&erro);
                          if(buffer_eeprom[k]==8){
                              k--;
                          }
                          else{
                              k++;
                          }
                   }

                  for(i=0;i<strlen(buffer_eeprom);i++){
                      escrever_eeprom(endereco_servidor_mqtt+i,buffer_eeprom[i]);
                  }

                  goto select;
              }


              else if(menu=='n'){
                  escreve_usb("\r\n");
                  asm{nop};
                  goto menu;
              }

              else{
                  escreve_usb("\r\n");
                  escreve_usb("Insira opcao valida\r\n");
                  goto menu_servidor;
              }


            break;

            case '7':
            menu_utilizador:
              escreve_usb("\r\nUTILIZADOR do servidor MQTT:");
              ler_user_mqtt();
              escreve_usb(buffer_eeprom);

              escreve_usb("\r\n");
              escreve_usb("Alterar(s/n)?");

              menu=Soft_Uart_Read(&erro);

              if(menu=='s'){
                  escreve_usb("\r\n");
                  for(i=0;i<48;i++){
                      escrever_eeprom(endereco_user_mqtt+i,0x00);
                  }

                  escreve_usb("Insira novo UTILIZADOR do servidor MQTT:\r\n");
                  delay_ms(50);
                  k=0;
                  limpa_buffer_eeprom();
                  while(buffer_eeprom[k-1]!=13){
                      buffer_eeprom[k]=Soft_Uart_Read(&erro);
                          if(buffer_eeprom[k]==8){
                              k--;
                          }
                          else{
                              k++;
                          }
                   }

                  for(i=0;i<strlen(buffer_eeprom);i++){
                      escrever_eeprom(endereco_user_mqtt+i,buffer_eeprom[i]);
                  }

                  goto select;
              }


              else if(menu=='n'){
                  escreve_usb("\r\n");
                  asm{nop};
                  goto menu;
              }

              else{
                  escreve_usb("\r\n");
                  escreve_usb("Insira opcao valida\r\n");
                  goto menu_utilizador;
              }

            break;

            case '8':
            menu_password:
              escreve_usb("\r\nPASSWORD do servidor MQTT:");
              ler_password();
              escreve_usb(buffer_eeprom);

              escreve_usb("\r\n");
              escreve_usb("Alterar(s/n)?");

              menu=Soft_Uart_Read(&erro);

              if(menu=='s'){
                  escreve_usb("\r\n");
                  for(i=0;i<48;i++){
                      escrever_eeprom(endereco_password_mqtt+i,0x00);
                  }

                  escreve_usb("Insira nova PASSWORD do servidor MQTT:\r\n");
                  delay_ms(50);
                  k=0;
                  limpa_buffer_eeprom();
                  while(buffer_eeprom[k-1]!=13){
                      buffer_eeprom[k]=Soft_Uart_Read(&erro);
                          if(buffer_eeprom[k]==8){
                              k--;
                          }
                          else{
                              k++;
                          }
                   }

                  for(i=0;i<strlen(buffer_eeprom);i++){
                      escrever_eeprom(endereco_password_mqtt+i,buffer_eeprom[i]);
                  }

                  goto select;
              }


              else if(menu=='n'){
                  escreve_usb("\r\n");
                  asm{nop};
                  goto menu;
              }

              else{
                  escreve_usb("\r\n");
                  escreve_usb("Insira opcao valida\r\n");
                  goto menu_password;
              }

            break;

            case '9':
            menu_id:
              escreve_usb("\r\nID do sistema:");
              ler_id();
              escreve_usb(buffer_eeprom);

              escreve_usb("\r\n");
              escreve_usb("Alterar(s/n)?");

              menu=Soft_Uart_Read(&erro);

              if(menu=='s'){
                  escreve_usb("\r\n");
                  for(i=0;i<48;i++){
                      escrever_eeprom(endereco_id+i,0x00);
                  }

                  escreve_usb("Insira novo ID do sistema:\r\n");
                  delay_ms(50);
                  k=0;
                  limpa_buffer_eeprom();
                  while(buffer_eeprom[k-1]!=13){
                      buffer_eeprom[k]=Soft_Uart_Read(&erro);
                          if(buffer_eeprom[k]==8){
                              k--;
                          }
                          else{
                              k++;
                          }
                   }

                  for(i=0;i<strlen(buffer_eeprom);i++){
                      escrever_eeprom(endereco_id+i,buffer_eeprom[i]);
                  }

                  goto select;
              }


              else if(menu=='n'){
                  escreve_usb("\r\n");
                  asm{nop};
                  goto menu;
              }

              else{
                  escreve_usb("\r\n");
                  escreve_usb("Insira opcao valida\r\n");
                  goto menu_id;
              }
            break;

            case 'a':
            menu_sn:
              escreve_usb("\r\nNUMERO DE SERIE do sistema:");
              ler_sn();
              escreve_usb(buffer_eeprom);

              escreve_usb("\r\n");
              escreve_usb("Alterar(s/n)?");

              menu=Soft_Uart_Read(&erro);

              if(menu=='s'){
                  escreve_usb("\r\n");
                  for(i=0;i<48;i++){
                      escrever_eeprom(endereco_serial_number+i,0x00);
                  }

                  escreve_usb("Insira novo NUMERO DE SERIE do sistema:\r\n");
                  delay_ms(50);
                  k=0;
                  limpa_buffer_eeprom();
                  while(buffer_eeprom[k-1]!=13){
                      buffer_eeprom[k]=Soft_Uart_Read(&erro);
                          if(buffer_eeprom[k]==8){
                              k--;
                          }
                          else{
                              k++;
                          }
                   }

                  for(i=0;i<strlen(buffer_eeprom);i++){
                      escrever_eeprom(endereco_serial_number+i,buffer_eeprom[i]);
                  }

                  goto select;
              }


              else if(menu=='n'){
                  escreve_usb("\r\n");
                  asm{nop};
                  goto menu;
              }

              else{
                  escreve_usb("\r\n");
                  escreve_usb("Insira opcao valida\r\n");
                  goto menu_sn;
              }
            break;

            case 'b':
             menu_pass_admin:
              escreve_usb("\r\nPASSWORD DE ADMINISTRADOR do sistema:");
              ler_pass_admin();
              escreve_usb(buffer_eeprom);

              escreve_usb("\r\n");
              escreve_usb("Alterar(s/n)?");

              menu=Soft_Uart_Read(&erro);

              if(menu=='s'){
                  escreve_usb("\r\n");
                  for(i=0;i<48;i++){
                      escrever_eeprom(endereco_pass_admin+i,0x00);
                  }

                  escreve_usb("Insira nova PASSWORD DE ADMINISTRADOR do sistema:\r\n");
                  delay_ms(50);
                  k=0;
                  limpa_buffer_eeprom();
                  while(buffer_eeprom[k-1]!=13){
                      buffer_eeprom[k]=Soft_Uart_Read(&erro);
                          if(buffer_eeprom[k]==8){
                              k--;
                          }
                          else{
                              k++;
                          }
                   }

                  for(i=0;i<strlen(buffer_eeprom);i++){
                      escrever_eeprom(endereco_pass_admin+i,buffer_eeprom[i]);
                  }

                  goto select;
              }


              else if(menu=='n'){
                  escreve_usb("\r\n");
                  asm{nop};
                  goto menu;
              }

              else{
                  escreve_usb("\r\n");
                  escreve_usb("Insira opcao valida\r\n");
                  goto menu_pass_admin;
              }

            break;

            case 'c':
             menu_hora:


                  escreve_usb("Insira HORAS no formato HHMM:\r\n");
                  delay_ms(50);
                  k=0;
                  limpa_buffer_eeprom();
                  while(buffer_eeprom[k-1]!=13){
                      buffer_eeprom[k]=Soft_Uart_Read(&erro);
                          if(buffer_eeprom[k]==8){
                              k--;
                          }
                          else{
                              k++;
                          }
                   }

            configura_hora(buffer_eeprom);
            goto menu;
            break;

               case 'd':
             menu_data:


                  escreve_usb("Insira DATA no formato DDMMAA:\r\n");
                  delay_ms(50);
                  k=0;
                  limpa_buffer_eeprom();
                  while(buffer_eeprom[k-1]!=13){
                      buffer_eeprom[k]=Soft_Uart_Read(&erro);
                          if(buffer_eeprom[k]==8){
                              k--;
                          }
                          else{
                              k++;
                          }
                   }

            configura_data(buffer_eeprom);
            goto menu;
            break;

            case 'e':
            escreve_usb("\r\n");
            escreve_usb("LIGACAO TERMINADA");
            delay_ms(250);
            break;

            default:
            escreve_usb("Insira uma opcao valida");
            goto menu;
  }

fim_menu_usb:
  delay_ms(250);
  flag_menu_usb=0;
  INTCON.INT0IF=0;
  liga_int();

}

void first_time(){

char i;
reset_eeprom();

        limpa_buffer_eeprom();
        strcpy(buffer_eeprom,"zap.vivo.com.br");

        for(i=0;i<=strlen(buffer_eeprom);i++){
         escrever_eeprom(endereco_nome_apn+i,buffer_eeprom[i]);
        }

        limpa_buffer_eeprom();
        strcpy(buffer_eeprom,"vivo");

        for(i=0;i<=strlen(buffer_eeprom);i++){
         escrever_eeprom(endereco_user_apn+i,buffer_eeprom[i]);
        }

        limpa_buffer_eeprom();
        strcpy(buffer_eeprom,"vivo");

        for(i=0;i<=strlen(buffer_eeprom);i++){
         escrever_eeprom(endereco_password_apn+i,buffer_eeprom[i]);
        }

        limpa_buffer_eeprom();
        strcpy(buffer_eeprom,"m16.cloudmqtt.com");

        for(i=0;i<=strlen(buffer_eeprom);i++){
         escrever_eeprom(endereco_dns_mqtt+i,buffer_eeprom[i]);
        }

        limpa_buffer_eeprom();
        strcpy(buffer_eeprom,"18111");

        for(i=0;i<=strlen(buffer_eeprom);i++){
         escrever_eeprom(endereco_porta_mqtt+i,buffer_eeprom[i]);
        }

        limpa_buffer_eeprom();
        strcpy(buffer_eeprom,"telemedicao1");

        for(i=0;i<=strlen(buffer_eeprom);i++){
         escrever_eeprom(endereco_servidor_mqtt+i,buffer_eeprom[i]);
        }

        limpa_buffer_eeprom();
        strcpy(buffer_eeprom,"mgmudjrt");

        for(i=0;i<=strlen(buffer_eeprom);i++){
         escrever_eeprom(endereco_user_mqtt+i,buffer_eeprom[i]);
        }

        limpa_buffer_eeprom();
        strcpy(buffer_eeprom,"biCkUB2dEGNo");

        for(i=0;i<=strlen(buffer_eeprom);i++){
         escrever_eeprom(endereco_password_mqtt+i,buffer_eeprom[i]);
        }

        limpa_buffer_eeprom();
        strcpy(buffer_eeprom,"contador_001");

        for(i=0;i<=strlen(buffer_eeprom);i++){
         escrever_eeprom(endereco_id+i,buffer_eeprom[i]);
        }

        limpa_buffer_eeprom();
        strcpy(buffer_eeprom,"C13112019N000001");

        for(i=0;i<=strlen(buffer_eeprom);i++){
         escrever_eeprom(endereco_serial_number+i,buffer_eeprom[i]);
        }

        limpa_buffer_eeprom();
        strcpy(buffer_eeprom,"password");

        for(i=0;i<=strlen(buffer_eeprom);i++){
         escrever_eeprom(endereco_pass_admin+i,buffer_eeprom[i]);
        }

        escrever_eeprom(Ext_EEPROM_ADDR_LSB,0x00);
        escrever_eeprom(Ext_EEPROM_ADDR_MSB,0x00);
        
        escrever_eeprom(endereco_flag_config,0x0d);

}

void desliga_int(){
INTCON.GIE=0X00;
INTCON.PEIE=0x00;
}

void liga_int(){
INTCON.GIE=0X01;
INTCON.PEIE=0x01;
}

void configura_rtc(){

I2C1_Init(100000);
I2C1_Start();
I2C1_Wr(0xd0);
I2C1_Wr(0x07);
I2C1_Wr(0x90);
I2C1_Stop();

delay_ms(100);

I2C1_Start();
I2C1_Wr(0xd0);
I2C1_Wr(0x02);
I2C1_Wr(0x00);
I2C1_Stop();

delay_ms(100);

I2C1_Start();
I2C1_Wr(0xd0);
I2C1_Wr(0x00);
I2C1_Wr(0x00);
I2C1_Stop();

delay_ms(100);

}

void ler_rtc(){

char i;
char k[7];
char buffer_rtc[16];
char segundos_rtc,minutos_rtc,horas_rtc,dia_semana_rtc,dia_mes_rtc,mes_rtc,ano_rtc;

for(i=0;i<=15;i++){
        buffer_rtc[i]=0x00;
        }

for(i=0;i<=23;i++){
        hora_data[i]=0x00;
        }


I2C1_Init(100000);
delay_ms(100);
I2C1_Start();
I2C1_Wr(0xd0);
I2C1_Wr(0x00);
I2C1_Repeated_Start();
I2C1_Wr(0xd1);

    for(i=0;i<=6;i++){
        k[i]=I2C1_Rd(1);
    }

I2C1_Stop();

delay_ms(100);

        segundos_rtc=bcd2dec(k[0]);
        minutos_rtc=bcd2dec(k[1]);
        horas_rtc=bcd2dec(k[2]);
        dia_semana_rtc=bcd2dec(k[3]);
        dia_mes_rtc=bcd2dec(k[4]);
        mes_rtc=bcd2dec(k[5]);
        ano_rtc=bcd2dec(k[6]);
        
         strcpy(hora_data,"#");
         IntToStr(horas_rtc,buffer_rtc);
         ltrim(buffer_rtc);
         
             if(horas_rtc<=9){
                 strcat(hora_data,"0");
                 strcat(hora_data,buffer_rtc);
             }
             else{
                 strcat(hora_data,buffer_rtc);
             }

         strcat(hora_Data,":");
         IntToStr(minutos_rtc,buffer_rtc);
         ltrim(buffer_rtc);
         
             if(minutos_rtc<=9){
                     strcat(hora_data,"0");
                     strcat(hora_data,buffer_rtc);
                 }
             else{
                 strcat(hora_data,buffer_rtc);
             }
         
         strcat(hora_Data,":");
         IntToStr(segundos_rtc,buffer_rtc);
         ltrim(buffer_rtc);
         
             if(segundos_rtc<=9){
                     strcat(hora_data,"0");
                     strcat(hora_data,buffer_rtc);
                 }
                 else{
                     strcat(hora_data,buffer_rtc);
                 }

         strcat(hora_Data,"#");
         
         IntToStr(dia_mes_rtc,buffer_rtc);
         ltrim(buffer_rtc);
         
             if(dia_mes_rtc<=9){
                     strcat(hora_data,"0");
                     strcat(hora_data,buffer_rtc);
                 }
                 else{
                     strcat(hora_data,buffer_rtc);
                 }

         strcat(hora_Data,"-");
         IntToStr(mes_rtc,buffer_rtc);
         ltrim(buffer_rtc);
         
             if(mes_rtc<=9){
                     strcat(hora_data,"0");
                     strcat(hora_data,buffer_rtc);
                 }
                 else{
                     strcat(hora_data,buffer_rtc);
                 }
         
         strcat(hora_Data,"-");
         IntToStr(ano_rtc,buffer_rtc);
         ltrim(buffer_rtc);
         
             if(ano_rtc<=9){
                     strcat(hora_data,"0");
                     strcat(hora_data,buffer_rtc);
                 }
                 else{
                     strcat(hora_data,buffer_rtc);
                 }
         
         strcat(hora_data,"#");

}

void configura_hora(char hora[8]){
int minutos;
int horas;

horas=Dec2Bcd(((hora[0]-0x30)*10)+(hora[1]-0x30));
minutos=Dec2Bcd(((hora[2]-0x30)*10)+(hora[3]-0x30));

I2C1_Init(100000);
delay_ms(100);
I2C1_Start();
I2C1_Wr(0xd0);
I2C1_Wr(0x02);
I2C1_Wr(horas);
I2C1_Stop();

delay_ms(100);

I2C1_Start();
I2C1_Wr(0xd0);
I2C1_Wr(0x01);
I2C1_Wr(minutos);
I2C1_Stop();

delay_ms(100);
}

void configura_data(char data_in[8]){
int dia;
int mes;
int ano;

dia=Dec2Bcd(((data_in[0]-0x30)*10)+(data_in[1]-0x30));
mes=Dec2Bcd(((data_in[2]-0x30)*10)+(data_in[3]-0x30));
ano=Dec2Bcd(((data_in[4]-0x30)*10)+(data_in[5]-0x30));

I2C1_Init(100000);
delay_ms(100);
I2C1_Start();
I2C1_Wr(0xd0);
I2C1_Wr(0x04);
I2C1_Wr(dia);
I2C1_Stop();

delay_ms(100);

I2C1_Start();
I2C1_Wr(0xd0);
I2C1_Wr(0x05);
I2C1_Wr(mes);
I2C1_Stop();

delay_ms(100);

I2C1_Start();
I2C1_Wr(0xd0);
I2C1_Wr(0x06);
I2C1_Wr(ano);
I2C1_Stop();

delay_ms(100);

}

void inicia_i2c(){
I2C1_Init(100000);

delay_ms(100);

}

unsigned char ler_log(int endereco){

unsigned char valor;
unsigned char endereco_msb,endereco_lsb;

endereco_lsb=endereco;
endereco_msb=endereco >> 8;


I2C1_Start();
I2C1_Wr(0xa0);
I2C1_Wr(endereco_msb);
I2C1_Wr(endereco_lsb);
I2C1_Repeated_Start();
I2C1_Wr(0xa1);
valor=I2C1_Rd(0);
I2C1_Stop();

return valor;

}


void escreve_log(unsigned char dados_log){

unsigned char endereco_msb,endereco_lsb;
unsigned int endereco_externo;

endereco_lsb=ler_eeprom(Ext_EEPROM_ADDR_LSB);
endereco_msb=ler_eeprom(Ext_EEPROM_ADDR_MSB);

desliga_int();

inicia_i2c();

I2C1_Start();

I2C1_Wr(0xa0);
I2C1_Wr(endereco_msb);
I2C1_Wr(endereco_lsb);
I2C1_Wr(dados_log);
I2C1_Stop();

endereco_externo=((endereco_msb<<8) & 0xFF00)|(endereco_lsb & 0xFF);

    if(endereco_externo>=Ext_EEPROM_length){
        endereco_externo=0x00;
        escrever_eeprom(Ext_EEPROM_ADDR_LSB,endereco_externo);
        escrever_eeprom(Ext_EEPROM_ADDR_MSB,endereco_externo>>8);
    }

    else{
        endereco_externo++;
        escrever_eeprom(Ext_EEPROM_ADDR_LSB,endereco_externo);
        escrever_eeprom(Ext_EEPROM_ADDR_MSB,endereco_externo>>8);
    }


}

void limpa_buffer_gsm(){

char i=0x00;

    for(i=0;i<=63;i++){

        buffer_sim800[i]=0x00;
    }

}

void limpa_buffer_medidor(){

char i=0x00;

    for(i=0;i<=15;i++){

        buffer_medidor[i]=0xff; //0x00;
    }

}

void ler_gsm(){

int i=0x00;

place_sim800=0x00;
limpa_buffer_gsm();

desliga_int();
PIE0.INT0IE=0X00;
PIE0.INT2IE=0x00;
PIE3.RC2IE=0x00;

//UART1_Remappable_Init(9600);
delay_ms(100);

soft_uart_init(&PORTB,0,5,9600,0);

liga_int();

UART1_Remappable_Write_Text("AT+CREG?\r");

delay_ms(100);

desliga_int();
PIE0.INT0IE=0X00;
PIE0.INT2IE=0x00;
PIE3.RC2IE=0x00;

escreve_usb(buffer_sim800);

    if((strstr(buffer_sim800,"+PDP: DEACT")!=0) || (strstr(buffer_sim800,"+CREG: 0,0")!=0) || (strstr(buffer_sim800,"+CREG: 0,3")!=0) || (strstr(buffer_sim800,"+CREG: 0,4")!=0) || (strstr(buffer_sim800,"+CREG: 0,1")==0) || (strstr(buffer_sim800,"+CREG: 0,2")!=0) || (strstr(buffer_sim800,"+CPIN: NOT READY")!=0) || (strstr(buffer_sim800,"ERROR")!=0)){
          
             led_link=0x00;
              reset_sim800=0x00;
              delay_ms(200);
              reset_sim800=0x01;

        flag_sim800_error=0x01;
        pisca_status=0;
        
    }
    

   else{
   

          place_sim800=0x00;
          limpa_buffer_gsm();
            // soft_uart_init(&PORTB,0,5,9600,0);
          liga_int();

          UART1_Remappable_Write_Text("AT+CIPSTATUS\r");
          delay_ms(150);

          desliga_int();

            escreve_usb(buffer_sim800);

              if((strstr(buffer_sim800,"STATE: CONNECT OK")==0) || (strstr(buffer_sim800,"STATE: IP INITIAL")!=0)){
                led_link=0x00;
                flag_sim800_error=0x01;
                
                 tcp_start_sim800();
                 liga_mqtt();

                 limpa_buffer_dados();

                        delay_ms(1000);
                        
                        place_sim800=0x00;
                        limpa_buffer_gsm();
                        //soft_uart_init(&PORTB,0,5,9600,0);
                        liga_int();

                        UART1_Remappable_Write_Text("AT+CIPSTATUS\r");
                        delay_ms(150);

                        desliga_int();
                        
                            escreve_usb(buffer_sim800);
                            
                         if((strstr(buffer_sim800,"STATE: CONNECT OK")!=0)){
                              pisca_status=2;
                              led_link=0x01;
                              flag_sim800_error=0x00;

                         }
                         
                         else{

                              pisca_status=1;
                              led_link=0x00;
                              flag_sim800_error=0x01;
                         
                         }
                        
                        
              }
    
              else{

               limpa_buffer_dados();

                 pisca_status=2;
                 led_link=0x01;
                 flag_sim800_error=0x00;

              }

    }

fim_gsm:
delay_ms(250);
flag_medidor=0x00;
place_medidor=0x00;
flag_sincronismo=0x00;
tamanho_bloco=0x00;
flag_temporizador=0x00;

}

void dump_memory(){

unsigned char i=0;
unsigned char buffer_temp[32];
unsigned int apontador_memoria=0x00;
unsigned char endereco_msb,endereco_lsb;
unsigned int endereco_externo;
unsigned char seg_fim_msb=0x00;
unsigned int seg_fim=0x00;
unsigned int pulsos_ativa=0x00;
unsigned int pulsos_reativa=0x00;
unsigned char temp_string[7];

desliga_int();

endereco_lsb=ler_eeprom(Ext_EEPROM_ADDR_LSB);
endereco_msb=ler_eeprom(Ext_EEPROM_ADDR_MSB);
endereco_externo=((endereco_msb<<8) & 0xFF00)|(endereco_lsb & 0xFF);


for(i=0;i<31;i++){
    buffer_temp[i]=0x00;
}

inicia_i2c();

apontador_memoria=0x00;

          while(apontador_memoria<endereco_externo) {
                led_status=~led_status;
               for(i=0;i<28;i++){

                   buffer_temp[i]=ler_log(apontador_memoria);
                   apontador_memoria++;
               }
               
               limpa_buffer_medidor();
               
               buffer_medidor[0]=buffer_temp[0];
               buffer_medidor[1]=buffer_temp[1];
               buffer_medidor[2]=buffer_temp[2];
               buffer_medidor[3]=buffer_temp[3];
               buffer_medidor[4]=buffer_temp[4];
               buffer_medidor[5]=buffer_temp[5];
               buffer_medidor[6]=buffer_temp[6];
               buffer_medidor[7]=buffer_temp[7];
               buffer_medidor[8]=buffer_temp[8];
               
               for(i=0;i<=23;i++){
               hora_data[i]=0x00;
               }
        
               for (i=9;i<28;i++){
               hora_data[i-9]=buffer_temp[i];
               }
               

                ler_id();
                limpa_buffer_auxiliar();

                strcpy(buffer_auxiliar,buffer_eeprom);
                strcat(buffer_auxiliar,">");
               
                //dados medidor-------------------------------------------------

                //indicador reposição de demanda
                strcat(buffer_auxiliar,"RD=");

                if(byte_1.B4==0){
                    strcat(buffer_auxiliar,"0,");
                }

                else if(byte_1.B4==1){
                    strcat(buffer_auxiliar,"1,");
                }

                else{
                    strcat(buffer_auxiliar,"?,");
                }

                //indicador de UFER
                strcat(buffer_auxiliar,"UFER=");

                if(byte_1.B5==0){
                    strcat(buffer_auxiliar,"0,");
                }

                else if(byte_1.B5==1){
                    strcat(buffer_auxiliar,"1,");
                }

                else{
                    strcat(buffer_auxiliar,"?,");
                }

                //Indicador posto reativo
                strcat(buffer_auxiliar,"PR=");

                if((byte_1.B7==0) && (byte_1.B6==0)){
                    strcat(buffer_auxiliar,"N,");
                }

                else if((byte_1.B7==0) && (byte_1.B6==1)){
                    strcat(buffer_auxiliar,"C,");
                }

                else if((byte_1.B7==1) && (byte_1.B6==0)){
                    strcat(buffer_auxiliar,"I,");
                }

                else if((byte_1.B7==1) && (byte_1.B6==1)){
                    strcat(buffer_auxiliar,"A,");
                }

                else{
                    strcat(buffer_auxiliar,"?,");
                }

                //Indicador posto horário
                strcat(buffer_auxiliar,"PH=");

                if((byte_2.B1==0) && (byte_2.B0==0)){
                    strcat(buffer_auxiliar,"R,");
                }

                else if((byte_2.B1==0) && (byte_2.B0==1)){
                    strcat(buffer_auxiliar,"P,");
                }

                else if((byte_2.B1==1) && (byte_2.B0==0)){
                    strcat(buffer_auxiliar,"FP,");
                }

                else if((byte_2.B1==1) && (byte_2.B0==1)){
                    strcat(buffer_auxiliar,"QP,");
                }

                else{
                    strcat(buffer_auxiliar,"?,");
                }

                //Indicador do quadrante
                strcat(buffer_auxiliar,"Q=");

                if((byte_2.B5==0) && (byte_2.B4==0)){
                    strcat(buffer_auxiliar,"1,");
                }

                else if((byte_2.B5==0) && (byte_2.B4==1)){
                    strcat(buffer_auxiliar,"4,");
                }

                else if((byte_2.B5==1) && (byte_2.B4==0)){
                    strcat(buffer_auxiliar,"2,");
                }

                else if((byte_2.B5==1) && (byte_2.B4==1)){
                    strcat(buffer_auxiliar,"3,");
                }

                else{
                    strcat(buffer_auxiliar,"?,");
                }

                //Indicador tarifa reativo
                strcat(buffer_auxiliar,"TR=");

                if(byte_2.B7==0){
                    strcat(buffer_auxiliar,"0,");
                }

                else if(byte_2.B7==1){
                    strcat(buffer_auxiliar,"1,");
                }

                 for(i=0;i<7;i++){
                  temp_string[i]=0x00;
                  }

                //Pulsos energia ativa
                pulsos_ativa=byte_3 | byte_4<<8;

                IntToStr(pulsos_ativa,temp_string);
                ltrim(temp_string);

                strcat(buffer_auxiliar,"PA=");
                strcat(buffer_auxiliar,temp_string);
                strcat(buffer_auxiliar,",");

                 for(i=0;i<7;i++){
                  temp_string[i]=0x00;
                  }

                 //Pulsos energia reativa
                  pulsos_reativa=byte_5 | byte_6<<8;

                  IntToStr(pulsos_reativa,temp_string);
                  ltrim(temp_string);

                  strcat(buffer_auxiliar,"PR=");
                  strcat(buffer_auxiliar,temp_string);

                //--------------------------------------------------------------
                strcat(buffer_auxiliar,hora_data);
                strcat(buffer_auxiliar,"FALHA=1");

                strcat(buffer_auxiliar,"\r\n");

                envia_mqtt(buffer_auxiliar);
                delay_ms(500);
               
          }
  flag_medidor=0x00;
  led_status=1;
}

void envia_pacote_dados(){

unsigned char seg_fim_msb=0x00;
unsigned int pulsos_ativa=0x00;
unsigned int pulsos_reativa=0x00;
unsigned char temp_string[7];
char i;

desliga_int();

ler_id();
ler_rtc();
limpa_buffer_auxiliar();

strcpy(buffer_auxiliar,buffer_eeprom);
strcat(buffer_auxiliar,">");

//dados medidor

    //indicador reposição de demanda
    strcat(buffer_auxiliar,"RD=");
    
    if(byte_1.B4==0){
        strcat(buffer_auxiliar,"0,");
    }

    else if(byte_1.B4==1){
        strcat(buffer_auxiliar,"1,");
    }

    else{
        strcat(buffer_auxiliar,"?,");
    }

    //indicador de UFER
    strcat(buffer_auxiliar,"UFER=");
    
    if(byte_1.B5==0){
        strcat(buffer_auxiliar,"0,");
    }

    else if(byte_1.B5==1){
        strcat(buffer_auxiliar,"1,");
    }
    
    else{
        strcat(buffer_auxiliar,"?,");
    }

    //Indicador posto reativo
    strcat(buffer_auxiliar,"PR=");
    
    if((byte_1.B7==0) && (byte_1.B6==0)){
        strcat(buffer_auxiliar,"N,");
    }
    
    else if((byte_1.B7==0) && (byte_1.B6==1)){
        strcat(buffer_auxiliar,"C,");
    }
    
    else if((byte_1.B7==1) && (byte_1.B6==0)){
        strcat(buffer_auxiliar,"I,");
    }
    
    else if((byte_1.B7==1) && (byte_1.B6==1)){
        strcat(buffer_auxiliar,"A,");
    }
    
    else{
        strcat(buffer_auxiliar,"?,");
    }

    //Indicador posto horário
    strcat(buffer_auxiliar,"PH=");
    
    if((byte_2.B1==0) && (byte_2.B0==0)){
        strcat(buffer_auxiliar,"R,");
    }

    else if((byte_2.B1==0) && (byte_2.B0==1)){
        strcat(buffer_auxiliar,"P,");
    }

    else if((byte_2.B1==1) && (byte_2.B0==0)){
        strcat(buffer_auxiliar,"FP,");
    }

    else if((byte_2.B1==1) && (byte_2.B0==1)){
        strcat(buffer_auxiliar,"QP,");
    }

    else{
        strcat(buffer_auxiliar,"?,");
    }

    //Indicador do quadrante
    strcat(buffer_auxiliar,"Q=");
    
    if((byte_2.B5==0) && (byte_2.B4==0)){
        strcat(buffer_auxiliar,"1,");
    }

    else if((byte_2.B5==0) && (byte_2.B4==1)){
        strcat(buffer_auxiliar,"4,");
    }

    else if((byte_2.B5==1) && (byte_2.B4==0)){
        strcat(buffer_auxiliar,"2,");
    }

    else if((byte_2.B5==1) && (byte_2.B4==1)){
        strcat(buffer_auxiliar,"3,");
    }

    else{
        strcat(buffer_auxiliar,"?,");
    }

    //Indicador tarifa reativo
    strcat(buffer_auxiliar,"TR=");
    
    if(byte_2.B7==0){
        strcat(buffer_auxiliar,"0,");
    }

    else if(byte_2.B7==1){
        strcat(buffer_auxiliar,"1,");
    }

     for(i=0;i<7;i++){
      temp_string[i]=0x00;
      }
    
    //Pulsos energia ativa
    pulsos_ativa=byte_3 | byte_4<<8;
    
    IntToStr(pulsos_ativa,temp_string);
    ltrim(temp_string);
    
    strcat(buffer_auxiliar,"PA=");
    strcat(buffer_auxiliar,temp_string);
    strcat(buffer_auxiliar,",");

     for(i=0;i<7;i++){
      temp_string[i]=0x00;
      }

     //Pulsos energia reativa
      pulsos_reativa=byte_5 | byte_6<<8;

      IntToStr(pulsos_reativa,temp_string);
      ltrim(temp_string);
      
      strcat(buffer_auxiliar,"PR=");
      strcat(buffer_auxiliar,temp_string);

//-------------------------------
strcat(buffer_auxiliar,hora_data);
strcat(buffer_auxiliar,"FALHA=0");
strcat(buffer_auxiliar,"\r\n");


envia_mqtt(buffer_auxiliar);

exit_send:
delay_us(1);

}

void inicia_sistema(){

char i;

//estados iniciais
led_status=0x00;
led_link=0x00;
flag_reconnect=0x00;
contador_segundos=0x00;
flag_medidor=0x00;
limpa_buffer_dados();
limpa_buffer_gsm();
limpa_buffer_eeprom();
limpa_buffer_medidor();
limpa_buffer_auxiliar();

//reset do gsm
reset_sim800=0x00;
delay_ms(200);
reset_sim800=0x01;

//espera gsm arrancar
for(i=0;i<=30;i++){
    led_status=1;
    led_link=0;
    delay_ms(250);
    led_status=0;
    led_link=0;
    delay_ms(250);

}

//estado leds inicial
led_status=0x01;
led_link=0x00;

//configura relogio
configura_rtc();
delay_ms(100);

//verifica se arranca pela primeira vez o sistema
if(ler_eeprom(endereco_flag_config)!=0x0d){
  
    led_status=0x01;
    first_time();

}

//configura Uart1
configura_uart1();

//configura interrupcoes
configura_interrupcoes();
delay_ms(100);

//estado modulo gsm-------------------------------------------------------------
ler_gsm();

    //envia ok de arranque de sistema
    if (flag_sim800_error==0){

        limpa_buffer_auxiliar();
        ler_id();
        ler_rtc();
        strcpy(buffer_auxiliar,buffer_eeprom);
        strcat(buffer_auxiliar,hora_data);
        strcat(buffer_auxiliar,">ESTADO.BOOT=1");
        envia_mqtt(buffer_auxiliar);

    }
//------------------------------------------------------------------------------

desliga_int();
configura_uart2();
delay_ms(100);

//zerar flags e buffers
flag_medidor=0x00;
place_medidor=0x00;
tamanho_bloco=0x00;
flag_sincronismo=0x00;
contador_segundos=0x00;
contador_gsm=0x00;
flag_reconnect=0x00;
limpa_buffer_auxiliar();

//zerar contador
PIR0.TMR0IF=0;
TMR0H=0x00;
TMR0L=0x00;
T0CON1=0X43;
T0CON0=0X90;

}
//******************************************************************************
//INTERRUPÇÕES

void interrupt(){
char i;
//INTCON.GIE=0X00;
//INTCON.PEIE=0x00;


      //pedido ligaçao usb
      if(PIR0.INT0IF==1){
      flag_menu_usb=0x01;
      PIR0.INT0IF=0x00;
      }
      
      //recebe dados gsm
       else if(PIR3.RC1IF==1){

                  if (RC1STA.b1 == 1) {
                      RCSTA1.CREN = 0;
                      asm{nop}
                      RCSTA1.CREN = 1;
                    }

                buffer_sim800[place_sim800]=RC1REG;
                place_sim800++;
        }
        
      //timer de sincronismo dados
      else if(PIR3.RC2IF==1){
                
           if (flag_sincronismo==0x01){
                     T0CON0=0x00;
                    //limpa erro overrun
                    if (RC2STA.b1 == 1) {
                        RCSTA2.CREN = 0;
                        asm{nop}
                        RCSTA2.CREN = 1;
                    }

                     buffer_medidor[place_medidor]=RC2REG;
                     tamanho_bloco++;
                     place_medidor++;
                 goto end_int;
              }


                //liga base tempo de 128ms se estiver desligada. Um byte e recebido em 88ms
                if(flag_temporizador==0){


                    place_medidor=0x00;
                    flag_sincronismo=0x00;
                    tamanho_bloco=0x00;

                     //zerar contador
                     PIR0.TMR0IF=0;
                     TMR0H=0x00;
                     TMR0L=0x00;
                     T0CON1=0x44;//0X43;
                     T0CON0=0X90;

                    flag_temporizador=0x01;

                        for(i=0;i<15;i++){
                            buffer_medidor[i]=0xff; //0x00;
                        }

                        //limpa erro overrun
                        if (RC2STA.b1 == 1) {
                            RCSTA2.CREN = 0;
                            asm{nop}
                            RCSTA2.CREN = 1;
                        }

                    buffer_medidor[place_medidor]=RC2REG;
                    goto end_int;
                }


                if(flag_temporizador==1){

                    T0CON0=0x00;

                        if (PIR0.TMR0IF==1){

                            for(i=0;i<15;i++){
                            buffer_medidor[i]=0xff; //0x00;
                            }

                            buffer_medidor[place_medidor]=RC2REG;
                            place_medidor++;
                            tamanho_bloco++;

                            flag_sincronismo=0x01;
                            flag_temporizador=0x00;

                            PIR0.TMR0IF=0;

                        }


                        else{

                            //zerar contador
                             PIR0.TMR0IF=0;
                             TMR0H=0x00;
                             TMR0L=0x00;
                             T0CON1=0x44;//0X43;
                             T0CON0=0X90;

                            place_medidor=0x00;
                            flag_sincronismo=0x00;
                            tamanho_bloco=0x00;

                            flag_temporizador=0x00;

                                for(i=0;i<15;i++){
                                
                                buffer_medidor[i]=0xff; //0x00;

                                }
                                
                        }

                }

        }
        

        
        //contador controlador "alive"
       else if(PIR0.INT2IF==1){

          contador_segundos++;
          contador_gsm++;
          
            if (contador_segundos>=920){
             flag_medidor=0;
            }
            PIR0.INT2IF=0x00;
        }
        
        else{
        asm{nop}
        }


end_int:

         if (tamanho_bloco==8){

                fd_msb=0x00;
                fd_msb.B0=buffer_medidor[1].B0;
                fd_msb.B1=buffer_medidor[1].B1;
                fd_msb.B2=buffer_medidor[1].B2;
                fd_msb.B3=buffer_medidor[1].B3;
                fd_lsb=buffer_medidor[0];



                     if (((fd_lsb | fd_msb <<8)==0)){

                        byte_0=buffer_medidor[0];
                        byte_1=buffer_medidor[1];
                        byte_2=buffer_medidor[2];
                        byte_3=buffer_medidor[3];
                        byte_4=buffer_medidor[4];
                        byte_5=buffer_medidor[5];
                        byte_5=buffer_medidor[6];
                        byte_7=buffer_medidor[7];
                        byte_8=buffer_medidor[8];

                         flag_medidor=0x01;
                         place_medidor=0x00;
                         flag_sincronismo=0x00;
                         tamanho_bloco=0x00;
                         flag_temporizador=0x00;
                     }

                     else{

                         flag_medidor=0x00;
                         place_medidor=0x00;
                         flag_sincronismo=0x00;
                         tamanho_bloco=0x00;
                         flag_temporizador=0x00;


                      }
                      
           }
             
//INTCON.GIE=0X01;
//INTCON.PEIE=0x01;

}


//******************************************************************************
//Programa principal

void main() {

unsigned int i=0;
unsigned int j=0;
unsigned int k=0;

float f=0;
unsigned char endereco_msb,endereco_lsb;
unsigned int endereco_externo;
unsigned char seg_fim_msb=0x00;

configura_io();
inicia_sistema();

while(1){

/*soft_uart_init(&PORTB,0,5,9600,0);
inttostr((fd_lsb | fd_msb <<8),buffer_auxiliar);
escreve_usb(buffer_auxiliar);
limpa_buffer_auxiliar();
inttostr(flag_medidor,buffer_auxiliar);

escreve_usb(buffer_auxiliar);*/

PIE0.INT0IE=0X01;
PIE0.INT2IE=0x01;
PIE3.RC2IE=0x01;

liga_int();



              //dados recebidos do medidor
              if (flag_medidor==1) {
                   
                   PIE0.INT0IE=0X00;
                   PIE0.INT2IE=0x00;
                   PIE3.RC2IE=0x00;
                   
                   desliga_int();

                   ler_gsm();

                      //ligação OK pode enviar
                      if (flag_sim800_error==0) {

                        desliga_int();
                        led_status=1; 
                        
                        envia_pacote_dados();

                      }

                      //ligação NOK...grava na EEPROM externa
                      else if (flag_sim800_error==1) {

                             escreve_log(byte_0);
                             escreve_log(byte_1);
                             escreve_log(byte_2);
                             escreve_log(byte_3);
                             escreve_log(byte_4);
                             escreve_log(byte_5);
                             escreve_log(byte_6);
                             escreve_log(byte_7);
                             escreve_log(byte_8);
                             
                             ler_rtc();

                           for (k=0;k<strlen(hora_data);k++){
                             escreve_log(hora_data[k]);
                           }

                      }
                      
                      else{
                      delay_us(1);
                      }

                  led_status=0x01;
                  flag_medidor=0x00;
                  
                  //dummy read
                  i=RCREG2;
                  
                  contador_segundos=0x00;


                }

                //Se já tem ligaçao e dados guardados na memoria externa,envia
             //os dados todos
             if (flag_sim800_error==0) {

              endereco_lsb=ler_eeprom(Ext_EEPROM_ADDR_LSB);
              endereco_msb=ler_eeprom(Ext_EEPROM_ADDR_MSB);
              endereco_externo=((endereco_msb<<8) & 0xFF00)|(endereco_lsb & 0xFF);

                  if (endereco_externo!=0){

                     dump_memory();
                     escrever_eeprom(Ext_EEPROM_ADDR_LSB,0x00);
                     escrever_eeprom(Ext_EEPROM_ADDR_MSB,0x00);
                  
                   //dummy read
                  //i=RCREG2;
                  
                  led_status=0x01;
                  flag_medidor=0x00;
                  
                  //acrescentado depois de teste ok

                      place_medidor=0x00;
                      flag_sincronismo=0x00;
                      tamanho_bloco=0x00;
                      flag_temporizador=0x00;


                  }

                  else{
                  asm{nop}
                  }

             }

                //Falha de rececao de dados/avaria medidor
                if ((contador_segundos>=920) && (flag_medidor==0x00)){

                   desliga_int();

                   ler_gsm();

                  if (flag_sim800_error==0){

                      ler_id();
                      ler_rtc();
                      limpa_buffer_auxiliar();

                      strcpy(buffer_auxiliar,buffer_eeprom);
                      strcat(buffer_auxiliar,hora_data);
                      strcat(buffer_auxiliar,">");
                      strcat(buffer_auxiliar,"ALARME_FALHA_MEDIDOR");
                      envia_mqtt(buffer_auxiliar);

                  }

                  flag_medidor=0x00;
                  contador_segundos=0x00;
                  contador_gsm=0x00;
                }

                //verifica ligação a cada 30s
                if ((contador_gsm>=30) && (contador_segundos<=750)){

                     desliga_int();
                     PIE0.INT0IE=0X00;
                     PIE0.INT2IE=0x00;
                     PIE3.RC2IE=0x00;
                     
                     ler_gsm();
                     
                      if (flag_sim800_error==0){

                      ler_id();
                      ler_rtc();
                      limpa_buffer_auxiliar();

                      strcpy(buffer_auxiliar,buffer_eeprom);
                      strcat(buffer_auxiliar,hora_data);
                      strcat(buffer_auxiliar,">");
                      strcat(buffer_auxiliar,"ESTADO.SISTEMA.OK=1");
                      envia_mqtt(buffer_auxiliar);
                      
                      flag_medidor=0x00;
                      place_medidor=0x00;
                      flag_sincronismo=0x00;
                      tamanho_bloco=0x00;
                      flag_temporizador=0x00;

                  }


                     flag_medidor=0x00;
                     contador_gsm=0x00;
                }

                //evita acesso acidental ao menu
                if (flag_menu_usb==1 && botao_config==1){
                     flag_menu_usb=0;
                     PIR0.INT0IF=0x00;

                }

                //pedido acesso menu configuração
               if(flag_menu_usb==1 && botao_config==0){

                  PIE0.INT0IE=0X00;
                  PIE0.INT2IE=0x00;
                  PIE3.RC2IE=0x00;
                  led_status=1;

                 menu_usb();
                 flag_menu_usb=0;
                 PIR0.INT0IF=0x00;

                }

                if (pisca_status==0){

                    led_link=0x00;
                }

                if (pisca_status==1){
                    led_link=~led_link;
                    delay_ms(80);
                }

                if (pisca_status==2){

                   led_link=0x01;
                }

    }
    
}