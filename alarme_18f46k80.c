/*
*********************** CONTROLO ACESSOS v1.0*****************************
by Samuel Parente 2018
**************************************************************************
*/

//enderecos de eeprom gerais
const int pin_admin_address=0x0000; //de 0x00 a 0x03
const int pin_developer_address=0x0004; //de 0x04 a 0x07
const int flag_first_time_address=0x0008;
const int pin_tecnico_address=0x0009; //de 0x09 a 0x0c

//enderecos de eeprom acesso wifi e gsm
const int numero_gsm_address=0x000D; //de 0x0d a 0x1b-15 posicoes de memoria
const int id_address=0x001C; //0x1c a 0x4c-48 posicoes de memoria
const int limite_temperatura_address=0x004D;

const int ip_address=0x5F;  //0x5f a 0x6f-16 posicoes de memoria
const int porta_address=0x70; //0x70 a 0x79-9 posicoes de memoria
const int ssid_address=0x7A; //0x7a a 0xaa-48 posicoes de memoria
const int ssid_password_address=0xAB; //0xab a 0xdb-48 posicoes de memoria
const int dns_servidor_address=0xDE; //0xde a 0x010d-48 posicoes memoria
const int porta_servidor_address=0x010E; // 0x010E a 0x0116 - 9 posicoes de memoria

//eeprom livre de 0x0117 a  0x03ff


//nomes de e/s

sbit coluna1 at latd.b0;
sbit coluna2 at latd.b1;
sbit coluna3 at latd.b2;
sbit coluna4 at latd.b3;
sbit linha1 at porta.b0;
sbit linha2 at porta.b1;
sbit linha3 at porta.b2;
sbit linha4 at porta.b3;

sbit fechadura_principal at lata.b6;
sbit ring at portb.b0;
sbit tamper at portb.b2;
sbit buzzer at lata.b5;
sbit sirene at lata.b7;
sbit reset_wifi at late.b0;
sbit botao_emergencia at portb.b3;

// Ligacoes do LCD
sbit LCD_RS at RD5_bit;
sbit LCD_EN at RD4_bit;

sbit LCD_D7 at RC3_bit;
sbit LCD_D6 at RC2_bit;
sbit LCD_D5 at RC1_bit;
sbit LCD_D4 at RC0_bit;

sbit LCD_RS_Direction at TRISD5_bit;
sbit LCD_EN_Direction at TRISD4_bit;

sbit LCD_D7_Direction at TRISC3_bit;
sbit LCD_D6_Direction at TRISC2_bit;
sbit LCD_D5_Direction at TRISC1_bit;
sbit LCD_D4_Direction at TRISC0_bit;

//RTC I2C
sbit Soft_I2C_Scl at RC4_bit;
sbit Soft_I2C_Sda at RC5_bit;
sbit Soft_I2C_Scl_Direction at TRISC4_bit;
sbit Soft_I2C_Sda_Direction at TRISC5_bit;

//variaveis
int tecla;
int caps_lock;
int contador_tecla0;
int contador_tecla1;
int contador_tecla2;
int contador_tecla3;
int contador_tecla4;
int contador_tecla5;
int contador_tecla6;
int contador_tecla7;
int contador_tecla8;
int contador_tecla9;
int dump_time;
int dump;
int linha_lcd;
int coluna_lcd;
int temp;
int mem_limit;

sbit temp_7 at temp.b7;
sbit temp_6 at temp.b6;
sbit temp_5 at temp.b5;
sbit temp_4 at temp.b4;
sbit temp_3 at temp.b3;
sbit temp_2 at temp.b2;
sbit temp_1 at temp.b1;
sbit temp_0 at temp.b0;

int i;
int y;
int yy;
char menu_id;
char canal_id;
signed int erro;
int flag_teclado;
int flag_rtc;
int endereco;
int dados;
char id[18];
unsigned char buffer[512];
int segundos;
int minutos;
int horas;
int dia_semana;
int dia_mes;
int mes;
int ano;
int segundos_dec;
int minutos_dec;
int horas_dec;
int dia_semana_dec;
int dia_mes_dec;
int mes_dec;
int ano_dec;
int ponto_dec;
char temperatura1[5];
int flag_temperatura;
float buffer_float=0.0;


//prototipos de funcoes
void limpa_teclado(void);
void escreve_eeprom(unsigned int endereco,unsigned char dados);
int ler_teclado_alfanumerico(void);
void configura_io(void);
void configura_interrupcoes(void);
void configura_timer0(void);
void configura_timer1(void);
void configura_rtc(void);
char ler_rtc(void);
void verifica_ocorrencias(void);
void menu_pin(void);
void menu_lcd(void);
void altera_hora_rtc(void);
void altera_data_rtc(int dia_rtc,int mes_rtc,int ano_rtc);
void liga_primeira_vez(void);
void limpa_buffer(void);
void escrever_temperatura(int);
int ler_temperatura(void);
void envia_sms(char texto_sms[]);
void configura_gsm(void);

//funcoes
void configura_rtc(){
soft_i2c_init();
soft_i2c_start();
soft_i2c_write(0xd0);   //endereco do ds1307 e direccao escreve
soft_i2c_write(0x07);   //endereco do byte de controlo
soft_i2c_write(0x90);   //saida 1hz sqrwave
soft_i2c_stop();

soft_i2c_start();
soft_i2c_write(0xd0);   //endereco do ds1307 e direccao escreve
soft_i2c_write(0x00);   //endereco do byte para ligar oscilador
soft_i2c_write(0x00);   //liga oscilador
soft_i2c_stop();

}

char ler_rtc(){

char mini_buffer[5]="";

limpa_buffer();

for (i=0;i<5;i++){
 mini_buffer[i]=0x00;
}

///////////////ler rtc//////////////////////////

soft_i2c_init();
soft_i2c_start();
soft_i2c_write(0xd0); //endereco do ds1307 e direccao escreve
soft_i2c_write(0x00); //ponteiro memoria posicao 0

soft_i2c_start();
soft_i2c_write(0xd1); //endereco do ds1307 e direccao ler
segundos= Soft_I2C_Read(1);
minutos= Soft_I2C_Read(1);
horas= Soft_I2C_Read(1);
dia_semana= Soft_I2C_Read(1);
dia_mes= Soft_I2C_Read(1);
mes= Soft_I2C_Read(1);
ano= Soft_I2C_Read(1);
soft_i2c_stop();
///////////////////////////////////////////////

segundos_dec=bcd2dec(segundos);
minutos_dec=bcd2dec(minutos);
horas_dec=bcd2dec(horas);
dia_semana_dec=bcd2dec(dia_semana);
dia_mes_dec=bcd2dec(dia_mes);
mes_dec=bcd2dec(mes);
ano_dec=bcd2dec(ano);


bytetostr(dia_mes_dec,mini_buffer);
ltrim (mini_buffer);

      if (dia_mes_dec<=9){
      strcpy(buffer,"0");
      strcat(buffer,mini_buffer);
      }
      else {
      strcpy(buffer,mini_buffer);
      }
strcat(buffer,"/");

bytetostr(mes_dec,mini_buffer);
ltrim (mini_buffer);

      if (mes_dec<=9){
      strcat(buffer,"0");
      strcat(buffer,mini_buffer);
      }
      else {
      strcat(buffer,mini_buffer);
      }
strcat(buffer,"/");
       bytetostr(ano_dec,mini_buffer);
ltrim (mini_buffer);

      if (ano_dec<=9){
      strcat(buffer,"0");
      strcat(buffer,mini_buffer);
      }
      else {
      strcat(buffer,mini_buffer);
      }
strcat(buffer,"    ");

bytetostr(horas_dec,mini_buffer);
ltrim (mini_buffer);

      if (horas_dec<=9){
      strcat(buffer,"0");
      strcat(buffer,mini_buffer);
      }
      else {
      strcat(buffer,mini_buffer);
      }

       strcat(buffer,":");
bytetostr(minutos_dec,mini_buffer);
ltrim (mini_buffer);

      if (minutos_dec<=9){
      strcat(buffer,"0");
      strcat(buffer,mini_buffer);
      }
      else {
      strcat(buffer,mini_buffer);
      }

       /*strcat(buffer,":");

bytetostr(segundos_dec,mini_buffer);
ltrim (mini_buffer);

      if (segundos_dec<=9){
      strcat(buffer,"0");
      strcat(buffer,mini_buffer);
      }
      else {
      strcat(buffer,mini_buffer);
      }*/
      //buffer[20]=0x00;
   return buffer;

delay_ms(200);
}

void altera_hora_rtc(){

int horas_rtc;
int minutos_rtc;
int segundos_rtc;

i=((buffer[0]-0x30)*10)+((buffer[1]-0x30)*1);
horas_rtc=dec2bcd(i);
i=((buffer[2]-0x30)*10)+((buffer[3]-0x30)*1);
minutos_rtc=dec2bcd(i);
segundos_rtc=0x00;

soft_i2c_init();
soft_i2c_start();
soft_i2c_write(0xd0);   //endereco do ds1307 e direccao escreve
soft_i2c_write(0x00);   //endereco segundos
soft_i2c_write(segundos_rtc);
soft_i2c_stop();

delay_ms(30);

soft_i2c_start();
soft_i2c_write(0xd0);   //endereco do ds1307 e direccao escreve
soft_i2c_write(0x01);   //endereco minutos
soft_i2c_write(minutos_rtc);
soft_i2c_stop();

delay_ms(30);

soft_i2c_start();
soft_i2c_write(0xd0);   //endereco do ds1307 e direccao escreve
soft_i2c_write(0x02);   //endereco horas
soft_i2c_write(horas_rtc);
soft_i2c_stop();

}

void altera_data_rtc(){

int dia_rtc;
int mes_rtc;
int ano_rtc;

i=((buffer[0]-0x30)*10)+((buffer[1]-0x30)*1);
dia_rtc=dec2bcd(i);
i=((buffer[2]-0x30)*10)+((buffer[3]-0x30)*1);
mes_rtc=dec2bcd(i);
i=((buffer[4]-0x30)*10)+((buffer[5]-0x30)*1);
ano_rtc=dec2bcd(i);


soft_i2c_start();
soft_i2c_write(0xd0);   //endereco do ds1307 e direccao escreve
soft_i2c_write(0x06);   //endereco ano
soft_i2c_write(ano_rtc);
soft_i2c_stop();

delay_ms(30);

soft_i2c_start();
soft_i2c_write(0xd0);   //endereco do ds1307 e direccao escreve
soft_i2c_write(0x05);   //endereco mes
soft_i2c_write(mes_rtc);
soft_i2c_stop();

delay_ms(30);

soft_i2c_start();
soft_i2c_write(0xd0);   //endereco do ds1307 e direccao escreve
soft_i2c_write(0x04);   //endereco dia
soft_i2c_write(dia_rtc);
soft_i2c_stop();

}


void configura_timer0(){
//Prescaler 1:8;  524 ms para atualizar relogio no lcd
T0CON=0b00000000;
TMR0H=0x80;
TMR0L=0x00;
}

void configura_timer1(){
//prescaler 1:8;carregar contador com 0. 360ms para timeout do teclado

  T1CON=0b00110010;
  TMR1IF_bit=0;
  TMR1H = 0x00;
  TMR1L = 0x00;
  TMR1IE_bit=1;
}

void configura_io(){

ANCON0=0x00;
ANCON1=0x00;
CM1CON=0;
CM2CON=0;
INTCON=0;

//teclado 4x4
trisd.b0=0;
trisd.b1=0;
trisd.b2=0;
trisd.b3=0;
trisa.b0=1;
trisa.b1=1;
trisa.b2=1;
trisa.b3=1;

trisa.b6=0; //fechadura_principal
trisb.b0=1; //ring
trisb.b2=1; //tamper
trisa.b5=0; //buzzer
trisa.b7=0; //sirene
trise.b0=0; //reset_wifi
trisb.b3=1; //botao_emergencia

}

void configura_interrupcoes(){
INTCON=0x00;
INTCON2=0x00;
INTCON3=0x00;
PIR1=0x00;
PIR2=0x00;
PIR3=0x00;
PIR4=0x00;
PIR5=0x00;
RCON=0x00;

}

void limpa_teclado(void){
contador_tecla0=0;
contador_tecla1=0;
contador_tecla2=0;
contador_tecla3=0;
contador_tecla4=0;
contador_tecla5=0;
contador_tecla6=0;
contador_tecla7=0;
contador_tecla8=0;
contador_tecla9=0;
}

void limpa_buffer(){

  for(i=0;i<511;i++){
  buffer[i]=0x00;
  }

}


char ler_teclado_alfanumerico(void){
//ALTERAR TECLAS PARA TECLADO FISICO. O DO PROTEUS E DIFERENTE

// tecla 1: 1 e simbolos
// tecla 2: 2 A B C a b c
// tecla 3: 3 D E F d e f
// tecla 4: 4 G H I g h i
// tecla 5: 5 J K L j k l
// tecla 6: 6 M N O m n o
// tecla 7: 7 P Q R S p q r s
// tecla 8: 8 T U V t u v
// tecla 9: 9 W X Y Z w x y z
// tecla 0: 0 espaco
// tecla *: *
// tecla #: #
// tecla A: avancar
// tecla B: retroceder
// tecla C: enter
// tecla D: caps lock


//contador_tecla=0;

coluna1=1;
coluna2=0;
coluna3=0;
coluna4=0;

           if (linha1==1){

                     //circula caracteres
                    // if (linha1==1){
                      while(linha1==1){
                        }

                                    switch (contador_tecla1){
                                    case 0:tecla='1';break;
                                    case 1:tecla=',';break;
                                    case 2:tecla=';';break;
                                    case 3:tecla='.';break;
                                    case 4:tecla=':';break;
                                    case 5:tecla='-';break;
                                    case 6:tecla='+';break;
                                    case 7:tecla='"';break;
                                    case 8:tecla='_';break;
                                    case 9:tecla='!';break;
                                    case 10:tecla='?';break;
                                    case 11:tecla='<';break;
                                    case 12:tecla='>';break;
                                    case 13:tecla='&';break;
                                    case 14:tecla='%';break;
                                    case 15:tecla='/';break;
                                    case 16:tecla='(';break;
                                    case 17:tecla=')';break;
                                    case 18:tecla='@';break;
                                    case 19:tecla='=';break;
                                    default:tecla=0;break;
                                    }
                                  
                                             contador_tecla1++;

                                            if(contador_tecla1>19){
                                            contador_tecla1=0;

                                            }
                             

                             goto fim_teclado_alfanumerico;
                      //   }

          }
          

          if (linha2==1){
              while(linha2==1){
               }
          
                               if(caps_lock==0){
                                    switch(contador_tecla4){
                                    case 0:tecla='4';break;
                                    case 1:tecla='g';break;
                                    case 2:tecla='h';break;
                                    case 3:tecla='i';break;
                                    default:tecla=0;break;
                                    }
                                  }
                                  if(caps_lock==1){
                                    switch(contador_tecla4){
                                    case 0:tecla='4';break;
                                    case 1:tecla='G';break;
                                    case 2:tecla='H';break;
                                    case 3:tecla='I';break;
                                    default:tecla=0;break;
                                    }
                                  }

                                             contador_tecla4++;

                                            if(contador_tecla4>3){
                                            contador_tecla4=0;

                                            }


          goto fim_teclado_alfanumerico;

          }
           // else
         if (linha3==1){
                  while(linha3==1){
               }
                                 if(caps_lock==0){
                                        switch(contador_tecla7){
                                        case 0:tecla='7';break;
                                        case 1:tecla='p';break;
                                        case 2:tecla='q';break;
                                        case 3:tecla='r';break;
                                        case 4:tecla='s';break;
                                        default:tecla=0;break;
                                        }
                                   }
                                   
                                    if(caps_lock==1){
                                        switch(contador_tecla7){
                                        case 0:tecla='7';break;
                                        case 1:tecla='P';break;
                                        case 2:tecla='Q';break;
                                        case 3:tecla='R';break;
                                        case 4:tecla='S';break;
                                        default:tecla=0;break;
                                        }
                                   }
                                             contador_tecla7++;

                                            if(contador_tecla7>4){
                                            contador_tecla7=0;

                                            }


          goto fim_teclado_alfanumerico;

          }
          // else
          if (linha4==1){
                  while(linha4==1){
               }
               
                tecla='*';
                goto fim_teclado_alfanumerico;

          }
         /*else {
          tecla=0;
          }*/

coluna1=0;

coluna2=1;
coluna1=0;
coluna3=0;
coluna4=0;

         if (linha1==1){
                 while(linha1==1){
               }
                                   if(caps_lock==0){
                                    switch(contador_tecla2){
                                    case 0:tecla='2';break;
                                    case 1:tecla='a';break;
                                    case 2:tecla='b';break;
                                    case 3:tecla='c';break;
                                    default:tecla=0;break;
                                    }
                                  }
                                  if(caps_lock==1){
                                    switch(contador_tecla2){
                                    case 0:tecla='2';break;
                                    case 1:tecla='A';break;
                                    case 2:tecla='B';break;
                                    case 3:tecla='C';break;
                                    default:tecla=0;break;
                                    }
                                  }

                                             contador_tecla2++;

                                            if(contador_tecla2>3){
                                            contador_tecla2=0;

                                            }


              goto fim_teclado_alfanumerico;


          }
           //else
           if (linha2==1){
                 while(linha2==1){
               }
                                  if(caps_lock==0){
                                    switch(contador_tecla5){
                                    case 0:tecla='5';break;
                                    case 1:tecla='j';break;
                                    case 2:tecla='k';break;
                                    case 3:tecla='l';break;
                                    default:tecla=0;break;
                                    }
                                  }
                                  if(caps_lock==1){
                                    switch(contador_tecla5){
                                    case 0:tecla='5';break;
                                    case 1:tecla='J';break;
                                    case 2:tecla='K';break;
                                    case 3:tecla='L';break;
                                    default:tecla=0;break;
                                    }
                                  }

                                             contador_tecla5++;

                                            if(contador_tecla5>3){
                                            contador_tecla5=0;

                                            }


          goto fim_teclado_alfanumerico;

          }
          //else
           if (linha3==1){
                 while(linha3==1){
               }
               
                                 if(caps_lock==0){
                                    switch(contador_tecla8){
                                    case 0:tecla='8';break;
                                    case 1:tecla='t';break;
                                    case 2:tecla='u';break;
                                    case 3:tecla='v';break;
                                    default:tecla=0;break;
                                    }
                                  }
                                  if(caps_lock==1){
                                    switch(contador_tecla8){
                                    case 0:tecla='8';break;
                                    case 1:tecla='T';break;
                                    case 2:tecla='U';break;
                                    case 3:tecla='V';break;
                                    default:tecla=0;break;
                                    }
                                  }

                                             contador_tecla8++;

                                            if(contador_tecla8>3){
                                            contador_tecla8=0;

                                            }


          goto fim_teclado_alfanumerico;

          }
          // else
          if (linha4==1){
                  while(linha4==1){
               }

                                    switch(contador_tecla0){
                                    case 0:tecla='0';break;
                                    case 1:tecla=' ';break;
                                    case 2:tecla='{';break;
                                    case 3:tecla='}';break;
                                    case 4:tecla='[';break;
                                    case 5:tecla=']';break;
                                    default:tecla=0;break;
                                    }


                                             contador_tecla0++;

                                            if(contador_tecla0>5){
                                            contador_tecla0=0;

                                            }


          goto fim_teclado_alfanumerico;

          }

          /*else {
          tecla=0;
          }*/

coluna2=0;

coluna3=1;
coluna1=0;
coluna2=0;
coluna4=0;
         if (linha1==1){
                  while(linha1==1){
               }
                                   if(caps_lock==0){
                                    switch(contador_tecla3){
                                    case 0:tecla='3';break;
                                    case 1:tecla='d';break;
                                    case 2:tecla='e';break;
                                    case 3:tecla='f';break;
                                    default:tecla=0;break;
                                    }
                                  }
                                  if(caps_lock==1){
                                    switch(contador_tecla3){
                                    case 0:tecla='3';break;
                                    case 1:tecla='D';break;
                                    case 2:tecla='E';break;
                                    case 3:tecla='F';break;
                                    default:tecla=0;break;
                                    }
                                  }

                                             contador_tecla3++;

                                            if(contador_tecla3>3){
                                            contador_tecla3=0;

                                            }


         goto fim_teclado_alfanumerico;

          }
              // else
         if (linha2==1){
                   while(linha2==1){
               }
                                    if(caps_lock==0){
                                    switch(contador_tecla6){
                                    case 0:tecla='6';break;
                                    case 1:tecla='m';break;
                                    case 2:tecla='n';break;
                                    case 3:tecla='o';break;
                                    default:tecla=0;break;
                                    }
                                  }
                                  if(caps_lock==1){
                                    switch(contador_tecla6){
                                    case 0:tecla='6';break;
                                    case 1:tecla='M';break;
                                    case 2:tecla='N';break;
                                    case 3:tecla='O';break;
                                    default:tecla=0;break;
                                    }
                                  }

                                             contador_tecla6++;

                                            if(contador_tecla6>3){
                                            contador_tecla6=0;

                                            }


          goto fim_teclado_alfanumerico;

          }
           // else
          if (linha3==1){
                   while(linha3==1){
               }
          if(caps_lock==0){
                                    switch(contador_tecla9){
                                    case 0:tecla='9';break;
                                    case 1:tecla='w';break;
                                    case 2:tecla='x';break;
                                    case 3:tecla='y';break;
                                    case 4:tecla='z';break;
                                    default:tecla=0;break;
                                    }
                                  }
                                  if(caps_lock==1){
                                    switch(contador_tecla9){
                                    case 0:tecla='9';break;
                                    case 1:tecla='W';break;
                                    case 2:tecla='X';break;
                                    case 3:tecla='Y';break;
                                    case 4:tecla='Z';break;
                                    default:tecla=0;break;
                                    }
                                  }

                                             contador_tecla9++;

                                            if(contador_tecla9>4){
                                            contador_tecla9=0;

                                            }


          goto fim_teclado_alfanumerico;

          }
             // else
          if (linha4==1){
                    while(linha4==1){
               }
          tecla='#';
          goto fim_teclado_alfanumerico;

          }

       /*else {
          tecla=0;
          }*/

coluna3=0;

coluna4=1;
coluna1=0;
coluna2=0;
coluna3=0;

          if (linha1==1){
                    while(linha1==1){
               }
           tecla=1;
           goto fim_teclado_alfanumerico;

          }
          // else
          if (linha2==1){
               while(linha2==1){
               }
          tecla=2;
          goto fim_teclado_alfanumerico;

          }
            // else
         if (linha3==1){
                 while(linha3==1){
               }
          tecla=3;
          goto fim_teclado_alfanumerico;
          }
            //else

          if (linha4==1){
                 while(linha4==1){
               }
          tecla=4;
          goto fim_teclado_alfanumerico;

          }



coluna1=0;
coluna2=0;
coluna3=0;
coluna4=0;
tecla=0;

fim_teclado_alfanumerico:

                         if (tecla!=0){
                         buzzer=1;
                         delay_ms(15);
                         buzzer=0;
                         }
return tecla;
}


void menu_lcd(){

 fechadura_principal=0;
 
strcpy(buffer," ");
inicio_menu:

          menu_id=1;
          goto menu_show;




            limpa_teclado();
            ler_teclado_alfanumerico();
 ler_tecla:
 

                   if(tecla==0){
                   
                       goto ler_tecla;
                   
                   }
                   
                   else if(tecla==1){

                       if(menu_id>=14){
                       menu_id=1;
                       }
                       else{
                       menu_id++;
                       }
                   
                   }
                   
                    else if(tecla==2){

                       if(menu_id<=1){
                       menu_id=14;
                       }
                       else{
                       menu_id--;
                       }

                   }
                   
                   else if(tecla=='*'){

                        goto fim_menu_lcd;
                   
                   }

//mostra menu conforme seleccao

menu_show:

                   Lcd_Cmd(_LCD_CURSOR_OFF);

                   if(menu_id==1){
                        
                        Lcd_Cmd(_LCD_CLEAR);
                        Lcd_Out(2,1,"    Alterar PIN     ");
                        Lcd_Out(3,1,"       Acesso       ");
                   }


                 else if(menu_id==2){

                        Lcd_Cmd(_LCD_CLEAR);
                        Lcd_Out(2,1,"    Alterar HORA    ");

                   }

                 else if(menu_id==3){

                        Lcd_Cmd(_LCD_CLEAR);
                        Lcd_Out(2,1,"    ALTERAR DATA    ");

                    }
                    
                   else if(menu_id==4){

                        Lcd_Cmd(_LCD_CLEAR);
                        Lcd_Out(2,1," Alterar ID sistema ");

                   }
                   
                   else if(menu_id==5){

                        Lcd_Cmd(_LCD_CLEAR);
                        Lcd_Out(2,1,"   Alterar numero   ");
                        Lcd_Out(3,1,"        GSM         ");

                   }
                   
                   else if(menu_id==6){

                        Lcd_Cmd(_LCD_CLEAR);
                        Lcd_Out(2,1," Limite temperatura ");
                        Lcd_Out(3,1,"     do sistema     ");

                   }
                   
                   
                    else if(menu_id==7){

                        Lcd_Cmd(_LCD_CLEAR);
                        Lcd_Out(2,1,"    Painel Solar    ");
                        Lcd_Out(3,1,"     e Bateria      ");
                   }
                   
                   else if(menu_id==8){

                        Lcd_Cmd(_LCD_CLEAR);
                        Lcd_Out(2,1,"    Alterar rede    ");
                        Lcd_Out(3,1,"        WIFI        ");
                   }
                   
                   else if(menu_id==9){

                        Lcd_Cmd(_LCD_CLEAR);
                        Lcd_Out(2,1,"  Alterar password  ");
                        Lcd_Out(3,1,"        WIFI        ");
                   }
                   
                   else if(menu_id==10){

                        Lcd_Cmd(_LCD_CLEAR);
                        Lcd_Out(2,1,"        I.P.        ");
                        Lcd_Out(3,1,"    do sistema      ");
                   }
                   
                   else if(menu_id==11){

                        Lcd_Cmd(_LCD_CLEAR);
                        Lcd_Out(2,1,"   Alterar PORTA    ");
                        Lcd_Out(3,1,"    do sistema      ");
                   }
                   
                    else if(menu_id==12){

                        Lcd_Cmd(_LCD_CLEAR);
                        Lcd_Out(2,1,"    Alterar DNS     ");
                        Lcd_Out(3,1,"    do servidor     ");
                   }
                   
                   else if(menu_id==13){

                        Lcd_Cmd(_LCD_CLEAR);
                        Lcd_Out(2,1,"   Alterar PORTA    ");
                        Lcd_Out(3,1,"   do servidor      ");
                   }
                   
                   else if(menu_id==14){

                        Lcd_Cmd(_LCD_CLEAR);
                        Lcd_Out(2,1,"   RESET SISTEMA    ");

                   }

again:
                   limpa_teclado();
                   ler_teclado_alfanumerico();
                   
                   if(tecla==0){
                   goto again;
                   }
                   
                   else if(tecla=='*'){
                   goto fim_menu_lcd;
                   }
                   
                   else if(tecla=='#'){
                   ; //salta loop para aceder submenus
                   }
                   
                   else if(tecla==1){
                   goto ler_tecla;
                   }
                   
                   else if(tecla==2){

                       goto ler_tecla;

                   }
                   
                   else {
                   goto again;
                   }
                   
                   
////////////entra nos sub menus consoante menu_id//////////////////////////////

//ALTERA PIN
if(menu_id==1){

              Lcd_Cmd(_LCD_CLEAR);
              Lcd_Out(2,1,"Insira novo PIN:");

              i=0;
              ler_pin_novo:

                   limpa_teclado();
                   ler_teclado_alfanumerico();
                      
                       if(tecla=='*'){
                       goto menu_show;
                       }
                       
                      if (tecla==0){
                      goto ler_pin_novo;
                      }


                   
                      Lcd_Out_CP("*");
                      buffer[i]=tecla;
                      tecla==0x00;
                      i++;

                      if (i<4){
                      goto ler_pin_novo;
                      }

                for (i=0;i<4;i++){
                escreve_eeprom(pin_admin_address+i,buffer[i]);
                }

                Lcd_Cmd(_LCD_CLEAR);
                Lcd_Out(3,1,"    PIN alterado    ");
                delay_ms (2000);

                goto menu_show;
}


//ALTERA HORAS
else if (menu_id==2){

                    Lcd_Cmd(_LCD_CLEAR);
                    Lcd_Out(2,1,"Insira HHMM:");

                    i=0;

                    ler_horas_novo:

                            limpa_teclado();
                            ler_teclado_alfanumerico();


                       
                            if(tecla=='*'){
                             goto menu_show;
                             }

                              if (tecla==0){
                                 goto ler_horas_novo;
                              }

                     Lcd_Chr_CP(tecla);
                     buffer[i]=tecla;
                     tecla==0x00;
                     i++;

                      if (i<4){
                         goto ler_horas_novo;
                      }
      altera_hora_rtc();
                      
goto menu_show;
}

//ALTERA DATA
else if (menu_id==3){

                    Lcd_Cmd(_LCD_CLEAR);
                    Lcd_Out(2,1,"   Insira DDMMAA:   ");
                    Lcd_Cmd(_LCD_THIRD_ROW);
                    i=0;

                    ler_data_novo:

                         limpa_teclado();
                         ler_teclado_alfanumerico();


                           if(tecla=='*'){
                             goto menu_show;
                             }
                              if (tecla==0){
                                 goto ler_data_novo;
                              }

                     Lcd_Chr_CP(tecla);
                     buffer[i]=tecla;
                     tecla==0x00;
                     i++;

                      if (i<6){
                         goto ler_data_novo;
                      }

                      altera_data_rtc();
goto menu_show;
}


//Altera ID da maquina
else if (menu_id==4){


 limpa_buffer();
           i=0;

           ler_id_gravado:

              buffer[i]=eeprom_read(id_address+i);
               if (buffer[i]==0){}
               else {
               i++;
               goto ler_id_gravado;
               }

                   Lcd_Cmd(_LCD_CLEAR);
                   Lcd_Out(1,1,"ID do sistema:");
                   Lcd_Out(2,1,buffer);
                   Lcd_Out(3,1,"Insira novo ID:");

                   i=0;
                   
                   Lcd_Cmd(_LCD_FOURTH_ROW);
                   linha_lcd=4;
                   coluna_lcd=1;

                   Lcd_Cmd(_LCD_UNDERLINE_ON);

                   limpa_teclado();

                   ler_id_novo:

                     ler_teclado_alfanumerico();

                           if(tecla=='*'){
                             goto menu_show;
                             }
                             
                        if (tecla==0){
                           goto ler_id_novo;
                        }

                        else if(tecla==4){      //caps lock
                                Lcd_Cmd(_LCD_CURSOR_OFF);
                                if (caps_lock==1){
                                    caps_lock=0;
                                    lcd_out(1,20," ");

                                    }
                                    else if(caps_lock==0){
                                    caps_lock=1;
                                    lcd_out(1,20,"^");

                                    }
                                    lcd_out(linha_lcd,coluna_lcd,"");
                                     Lcd_Cmd(_LCD_UNDERLINE_ON);
                                 goto ler_id_novo;
                        }

                        //escreve caracter
                        else if ((tecla!=0)&&(tecla!=1)&&(tecla!=2)&&(tecla!=3)&&(tecla!=4)){

                             buffer[i]=tecla;
                             lcd_chr(linha_lcd,coluna_lcd,tecla);
                             Lcd_Cmd(_LCD_MOVE_CURSOR_LEFT);
                         goto ler_id_novo;
                        }

                        else if(tecla==1){      //avança proximo caracter
                        i++;
                        coluna_lcd++;
                        limpa_teclado();
                        Lcd_Cmd(_LCD_MOVE_CURSOR_RIGHT);
                        goto ler_id_novo;
                        }

                        else if(tecla==2){     //apaga
                                   if((contador_tecla0==0)&&
                                     (contador_tecla1==0)&&
                                     (contador_tecla2==0)&&
                                     (contador_tecla3==0)&&
                                     (contador_tecla4==0)&&
                                     (contador_tecla5==0)&&
                                     (contador_tecla6==0)&&
                                     (contador_tecla7==0)&&
                                     (contador_tecla8==0)&&
                                     (contador_tecla9==0)){

                                          i--;
                                        lcd_chr(linha_lcd,coluna_lcd,' ');
                                        coluna_lcd--;
                                        buffer[i]=0x00;
                                        lcd_chr(linha_lcd,coluna_lcd,' ');
                                         Lcd_Cmd(_LCD_MOVE_CURSOR_LEFT);
                                        limpa_teclado();
                                     }

                                     else{

                                        buffer[i]=0x00;
                                        lcd_chr(linha_lcd,coluna_lcd,' ');
                                         Lcd_Cmd(_LCD_MOVE_CURSOR_LEFT);
                                          i--;
                                        limpa_teclado();
                                     }


                        goto ler_id_novo;
                        }

                        else if(tecla==3){     //enter
                        i++;
                        buffer[i]=0x00;
                        limpa_teclado();
                        }

                        else{
                         goto ler_id_novo;
                        }

                        //grava nova ID da maquina
                        for (i=0;i<strlen(buffer);i++){
                         escreve_eeprom(id_address+i,buffer[i]);
                          }


                           escreve_eeprom(id_address+i,0x00);

                       
goto menu_show;
}


//Altera numero GSM
else if (menu_id==5){


 limpa_buffer();
           i=0;

           ler_gsm_gravado:

              buffer[i]=eeprom_read(numero_gsm_address+i);
               if (buffer[i]==0){}
               else {
               i++;
               goto ler_gsm_gravado;
               }

                   Lcd_Cmd(_LCD_CLEAR);
                   Lcd_Out(1,1,"Numero GSM:");
                   Lcd_Out(2,1,buffer);
                   Lcd_Out(3,1,"Insira novo numero:");

                   i=0;

                   Lcd_Cmd(_LCD_FOURTH_ROW);
                   linha_lcd=4;
                   coluna_lcd=1;

                   Lcd_Cmd(_LCD_UNDERLINE_ON);

                   limpa_teclado();

                   ler_numero_novo:

                        ler_teclado_alfanumerico();

                         if(tecla=='*'){
                             goto menu_show;
                             }
                             
                        if (tecla==0){
                           goto ler_numero_novo;
                        }

                        else if(tecla==4){      //caps lock
                                Lcd_Cmd(_LCD_CURSOR_OFF);
                                if (caps_lock==1){
                                    caps_lock=0;
                                    lcd_out(1,20," ");

                                    }
                                    else if(caps_lock==0){
                                    caps_lock=1;
                                    lcd_out(1,20,"^");

                                    }
                                    lcd_out(linha_lcd,coluna_lcd,"");
                                     Lcd_Cmd(_LCD_UNDERLINE_ON);
                                 goto ler_numero_novo;
                        }

                        //escreve caracter
                        else if ((tecla!=0)&&(tecla!=1)&&(tecla!=2)&&(tecla!=3)&&(tecla!=4)){

                             buffer[i]=tecla;
                             lcd_chr(linha_lcd,coluna_lcd,tecla);
                             Lcd_Cmd(_LCD_MOVE_CURSOR_LEFT);
                         goto ler_numero_novo;
                        }

                        else if(tecla==1){      //avança proximo caracter
                        i++;
                        coluna_lcd++;
                        limpa_teclado();
                        Lcd_Cmd(_LCD_MOVE_CURSOR_RIGHT);
                        goto ler_numero_novo;
                        }

                        else if(tecla==2){     //apaga
                                   if((contador_tecla0==0)&&
                                     (contador_tecla1==0)&&
                                     (contador_tecla2==0)&&
                                     (contador_tecla3==0)&&
                                     (contador_tecla4==0)&&
                                     (contador_tecla5==0)&&
                                     (contador_tecla6==0)&&
                                     (contador_tecla7==0)&&
                                     (contador_tecla8==0)&&
                                     (contador_tecla9==0)){

                                          i--;
                                        lcd_chr(linha_lcd,coluna_lcd,' ');
                                        coluna_lcd--;
                                        buffer[i]=0x00;
                                        lcd_chr(linha_lcd,coluna_lcd,' ');
                                         Lcd_Cmd(_LCD_MOVE_CURSOR_LEFT);
                                        limpa_teclado();
                                     }

                                     else{

                                        buffer[i]=0x00;
                                        lcd_chr(linha_lcd,coluna_lcd,' ');
                                         Lcd_Cmd(_LCD_MOVE_CURSOR_LEFT);
                                          i--;
                                        limpa_teclado();
                                     }


                        goto ler_numero_novo;
                        }

                        else if(tecla==3){     //enter
                        i++;
                        buffer[i]=0x00;
                        limpa_teclado();
                        }

                        else{
                         goto ler_numero_novo;
                        }

                        //grava novo numero GSM
                         for (i=0;i<strlen(buffer);i++){
                         escreve_eeprom(numero_gsm_address+i,buffer[i]);
                          }

                           escreve_eeprom(numero_gsm_address+i,0x00);


goto menu_show;
}


//Altera temperatura de disparo da ventoinha
else if (menu_id==6){

 limpa_buffer();
           i=0;

           ler_fan_gravado:


                   Lcd_Cmd(_LCD_CLEAR);
                  
             actualizar_temp:
             
                   Lcd_Out(2,1,"Temp. disparo:");
                    dump=eeprom_read(limite_temperatura_address);
                    wordtostr(dump,buffer);
                    ltrim(buffer);
                    rtrim(buffer);
                   Lcd_Out_cp(buffer);
                   lcd_chr_cp(0xDF); //0xb0 simbolo grau
                   lcd_chr_cp('C');
                   lcd_chr_cp(' ');
                   lcd_chr_cp(' ');
                   Lcd_Out(3,1,"tecla A-Aumentar");
                   Lcd_Out(4,1,"tecla B-Diminuir");
                   i=0;

                   limpa_teclado();

                   ler_fan_novo:

                        ler_teclado_alfanumerico();

                         if(tecla=='*'){
                             goto menu_show;
                             }

                        if (tecla==0){
                           goto ler_fan_novo;
                        }

                        else if(tecla==1){      //incrementa  variavel
                          dump=eeprom_read(limite_temperatura_address);
                          
                           if(dump==99){
                           dump=0;
                           escreve_eeprom(limite_temperatura_address,dump);
                           }
                           
                           else{
                            escreve_eeprom(limite_temperatura_address,dump+1);
                           }

                           
                                 goto actualizar_temp;
                        }


                        else if(tecla==2){      //decrementa variavel
                        dump=eeprom_read(limite_temperatura_address);
                        
                           if(dump==0){
                           dump=99;
                           escreve_eeprom(limite_temperatura_address,dump);
                           }

                           else{
                            escreve_eeprom(limite_temperatura_address,dump-1);
                           }
                                 goto actualizar_temp;
                        }


                        else if(tecla==3){     //enter - grava nova temperatura
                          ;

                        }

                        else{
                         goto ler_fan_novo;
                        }



goto menu_show;
}

//Estado do painel solar e bateria
else if (menu_id==7){


                          /*tensao painel 5 bytes
                          corrente painel 5 bytes

                          tensao bateria 5 bytes
                          corrente bateria 5 bytes

                          20 bytes (ja tudo em caracteres)

                          exemplo:

                          24.5512.4512.050.852\r\n*/



Lcd_Cmd(_LCD_CLEAR);


ler_painel_bat_novo:

limpa_buffer();
i=0;

//comandos de tensao e corrente painel solar e bateria
UART2_Init(9600);
UART_Set_Active(&UART2_Read, &UART2_Write, &UART2_Data_Ready, &UART2_Tx_Idle);

UART2_Write_text("tensao_corrente?\r\n"); //comando para ler tensoes e correntes

while (UART2_Data_Ready() != 1){
   ;
   }

UART2_Read_Text(buffer,"\r\n",22);

Lcd_Out(1,1,"****PAINEL SOLAR****");
Lcd_cmd(_LCD_SECOND_ROW);
Lcd_chr_cp(buffer[0]);
Lcd_chr_cp(buffer[1]);
Lcd_chr_cp(buffer[2]);
Lcd_chr_cp(buffer[3]);
Lcd_chr_cp(buffer[4]);
Lcd_out_cp("V ");

Lcd_chr_cp(buffer[5]);
Lcd_chr_cp(buffer[6]);
Lcd_chr_cp(buffer[7]);
Lcd_chr_cp(buffer[8]);
Lcd_chr_cp(buffer[9]);
Lcd_out_cp("A ");

buffer_float=(((buffer[0]-0x30)*10)+((buffer[1]-0x30)*1)+((buffer[3]-0x30)*0.1)+((buffer[4]-0x30)*0.01))*\
              (((buffer[5]-0x30)*10)+((buffer[6]-0x30)*1)+((buffer[8]-0x30)*0.1)+((buffer[9]-0x30)*0.01));


FloatToStr_FixLen(buffer_float,buffer,5);

Lcd_out_cp(buffer);
Lcd_out_cp("W ");

Lcd_out(3,1,"******BATERIA*******");

Lcd_cmd(_LCD_FOURTH_ROW);
Lcd_chr_cp(buffer[10]);
Lcd_chr_cp(buffer[11]);
Lcd_chr_cp(buffer[12]);
Lcd_chr_cp(buffer[13]);
Lcd_chr_cp(buffer[14]);
Lcd_out_cp("V ");

Lcd_chr_cp(buffer[15]);
Lcd_chr_cp(buffer[16]);
Lcd_chr_cp(buffer[17]);
Lcd_chr_cp(buffer[18]);
Lcd_chr_cp(buffer[19]);
Lcd_out_cp("A ");


buffer_float=(((buffer[10]-0x30)*10)+((buffer[11]-0x30)*1)+((buffer[13]-0x30)*0.1)+((buffer[14]-0x30)*0.01))*\
              (((buffer[15]-0x30)*10)+((buffer[16]-0x30)*1)+((buffer[18]-0x30)*0.1)+((buffer[19]-0x30)*0.01));

FloatToStr_FixLen(buffer_float,buffer,5);

Lcd_out_cp(buffer);
Lcd_out_cp("W ");

   repeat:
   
      ler_teclado_alfanumerico();

       if(tecla=='*'){
           goto menu_show;
           }

      if (tecla==0){
         goto ler_painel_bat_novo;
      }


      else{
       goto repeat;
      }

goto menu_show;

}

//ler e alterar rede wifi
else if (menu_id==8){

limpa_buffer();
           i=0;

           ler_ssid_gravado:

              buffer[i]=eeprom_read(ssid_address+i);
               if (buffer[i]==0){}
               else {
               i++;
               goto ler_ssid_gravado;
               }

                  Lcd_Cmd(_LCD_CLEAR);
                   Lcd_Out(1,1,"     Rede WIFI:     ");
                   Lcd_Out(2,1,buffer);
                    Lcd_Out(3,1,"  Insira Rede WIFI: ");
                    Lcd_Cmd(_LCD_FOURTH_ROW);


                   i=0;
                   linha_lcd=4;
                   coluna_lcd=1;
                   Lcd_Cmd(_LCD_UNDERLINE_ON);
                   limpa_teclado();

                   ler_ssid_novo:

                     ler_teclado_alfanumerico();

                            if(tecla=='*'){
                             goto menu_show;
                             }

                        if (tecla==0){
                           goto ler_ssid_novo;
                        }

                        else if(tecla==4){      //caps lock
                                Lcd_Cmd(_LCD_CURSOR_OFF);
                                if (caps_lock==1){
                                    caps_lock=0;
                                    lcd_out(1,20," ");

                                    }
                                    else if(caps_lock==0){
                                    caps_lock=1;
                                    lcd_out(1,20,"^");

                                    }
                                    lcd_out(linha_lcd,coluna_lcd,"");
                                     Lcd_Cmd(_LCD_UNDERLINE_ON);
                                 goto ler_ssid_novo;
                        }

                        //escreve caracter
                        else if ((tecla!=0)&&(tecla!=1)&&(tecla!=2)&&(tecla!=3)&&(tecla!=4)){

                             buffer[i]=tecla;
                             lcd_chr(linha_lcd,coluna_lcd,tecla);
                             Lcd_Cmd(_LCD_MOVE_CURSOR_LEFT);
                         goto ler_ssid_novo;
                        }

                        else if(tecla==1){      //avança proximo caracter
                        i++;
                        coluna_lcd++;
                        limpa_teclado();
                        Lcd_Cmd(_LCD_MOVE_CURSOR_RIGHT);
                        goto ler_ssid_novo;
                        }

                        else if(tecla==2){     //apaga
                                   if((contador_tecla0==0)&&
                                     (contador_tecla1==0)&&
                                     (contador_tecla2==0)&&
                                     (contador_tecla3==0)&&
                                     (contador_tecla4==0)&&
                                     (contador_tecla5==0)&&
                                     (contador_tecla6==0)&&
                                     (contador_tecla7==0)&&
                                     (contador_tecla8==0)&&
                                     (contador_tecla9==0)){

                                          i--;
                                        lcd_chr(linha_lcd,coluna_lcd,' ');
                                        coluna_lcd--;
                                        buffer[i]=0x00;
                                        lcd_chr(linha_lcd,coluna_lcd,' ');
                                         Lcd_Cmd(_LCD_MOVE_CURSOR_LEFT);
                                        limpa_teclado();
                                     }

                                     else{

                                        buffer[i]=0x00;
                                        lcd_chr(linha_lcd,coluna_lcd,' ');
                                         Lcd_Cmd(_LCD_MOVE_CURSOR_LEFT);
                                          i--;
                                        limpa_teclado();
                                     }


                        goto ler_ssid_novo;
                        }

                        else if(tecla==3){     //enter
                        i++;
                        buffer[i]=0x00;
                        limpa_teclado();
                        }

                        else{
                         goto ler_ssid_novo;
                        }

                        //grava nova rede wifi

                         for (i=0;i<strlen(buffer);i++){
                         escreve_eeprom(ssid_address+i,buffer[i]);
                          }

                           escreve_eeprom(ssid_address+i,0x00);


goto menu_show;
}

//ler e alterar password rede wifi
else if (menu_id==9){

limpa_buffer();
           i=0;

           ler_pass_gravado:

              buffer[i]=eeprom_read(ssid_password_address+i);
               if (buffer[i]==0){}
               else {
               i++;
               goto ler_pass_gravado;
               }

                  Lcd_Cmd(_LCD_CLEAR);
                   Lcd_Out(1,1,"   Password WIFI:   ");
                   Lcd_Out(2,1,buffer);
                    Lcd_Out(3,1,"Insira nova PASSWORD");
                    Lcd_Cmd(_LCD_FOURTH_ROW);

                   i=0;
                   linha_lcd=4;
                   coluna_lcd=1;
                   Lcd_Cmd(_LCD_UNDERLINE_ON);
                   limpa_teclado();

                   ler_pass_novo:

                     ler_teclado_alfanumerico();

                            if(tecla=='*'){
                             goto menu_show;
                             }

                        if (tecla==0){
                           goto ler_pass_novo;
                        }

                        else if(tecla==4){      //caps lock
                                Lcd_Cmd(_LCD_CURSOR_OFF);
                                if (caps_lock==1){
                                    caps_lock=0;
                                    lcd_out(1,20," ");

                                    }
                                    else if(caps_lock==0){
                                    caps_lock=1;
                                    lcd_out(1,20,"^");

                                    }
                                    lcd_out(linha_lcd,coluna_lcd,"");
                                     Lcd_Cmd(_LCD_UNDERLINE_ON);
                                 goto ler_pass_novo;
                        }

                        //escreve caracter
                        else if ((tecla!=0)&&(tecla!=1)&&(tecla!=2)&&(tecla!=3)&&(tecla!=4)){

                             buffer[i]=tecla;
                             lcd_chr(linha_lcd,coluna_lcd,tecla);
                             Lcd_Cmd(_LCD_MOVE_CURSOR_LEFT);
                         goto ler_pass_novo;
                        }

                        else if(tecla==1){      //avança proximo caracter
                        i++;
                        coluna_lcd++;
                        limpa_teclado();
                        Lcd_Cmd(_LCD_MOVE_CURSOR_RIGHT);
                        goto ler_pass_novo;
                        }

                        else if(tecla==2){     //apaga
                                   if((contador_tecla0==0)&&
                                     (contador_tecla1==0)&&
                                     (contador_tecla2==0)&&
                                     (contador_tecla3==0)&&
                                     (contador_tecla4==0)&&
                                     (contador_tecla5==0)&&
                                     (contador_tecla6==0)&&
                                     (contador_tecla7==0)&&
                                     (contador_tecla8==0)&&
                                     (contador_tecla9==0)){

                                          i--;
                                        lcd_chr(linha_lcd,coluna_lcd,' ');
                                        coluna_lcd--;
                                        buffer[i]=0x00;
                                        lcd_chr(linha_lcd,coluna_lcd,' ');
                                         Lcd_Cmd(_LCD_MOVE_CURSOR_LEFT);
                                        limpa_teclado();
                                     }

                                     else{

                                        buffer[i]=0x00;
                                        lcd_chr(linha_lcd,coluna_lcd,' ');
                                         Lcd_Cmd(_LCD_MOVE_CURSOR_LEFT);
                                          i--;
                                        limpa_teclado();
                                     }


                        goto ler_pass_novo;
                        }

                        else if(tecla==3){     //enter
                        i++;
                        buffer[i]=0x00;
                        limpa_teclado();
                        }

                        else{
                         goto ler_pass_novo;
                        }

                        //grava nova password rede wifi
                         for (i=0;i<strlen(buffer);i++){
                         escreve_eeprom(ssid_password_address+i,buffer[i]);
                          }

                           escreve_eeprom(ssid_password_address+i,0x00);


goto menu_show;
}

//ler e alterar IP
else if (menu_id==10){

limpa_buffer();
           i=0;

           ler_ip_gravado:

              buffer[i]=eeprom_read(ip_address+i);
               if (buffer[i]==0){}
               else {
               i++;
               goto ler_ip_gravado;
               }
               
                   Lcd_Cmd(_LCD_CLEAR);
                   Lcd_Out(1,1,"IP:");
                   Lcd_Out_CP(buffer);
                    Lcd_Out(2,1,"Insira novo IP:");
                    Lcd_Cmd(_LCD_THIRD_ROW);


                   i=0;
                   linha_lcd=3;
                   coluna_lcd=1;
                   Lcd_Cmd(_LCD_UNDERLINE_ON);
                   limpa_teclado();

                   ler_ip_novo:

                     ler_teclado_alfanumerico();

                            if(tecla=='*'){
                             goto menu_show;
                             }
                             
                        if (tecla==0){
                           goto ler_ip_novo;
                        }

                        else if(tecla==4){      //caps lock
                                Lcd_Cmd(_LCD_CURSOR_OFF);
                                if (caps_lock==1){
                                    caps_lock=0;
                                    lcd_out(1,20," ");

                                    }
                                    else if(caps_lock==0){
                                    caps_lock=1;
                                    lcd_out(1,20,"^");

                                    }
                                    lcd_out(linha_lcd,coluna_lcd,"");
                                     Lcd_Cmd(_LCD_UNDERLINE_ON);
                                 goto ler_ip_novo;
                        }

                        //escreve caracter
                        else if ((tecla!=0)&&(tecla!=1)&&(tecla!=2)&&(tecla!=3)&&(tecla!=4)){

                             buffer[i]=tecla;
                             lcd_chr(linha_lcd,coluna_lcd,tecla);
                             Lcd_Cmd(_LCD_MOVE_CURSOR_LEFT);
                         goto ler_ip_novo;
                        }

                        else if(tecla==1){      //avança proximo caracter
                        i++;
                        coluna_lcd++;
                        limpa_teclado();
                        Lcd_Cmd(_LCD_MOVE_CURSOR_RIGHT);
                        goto ler_ip_novo;
                        }

                        else if(tecla==2){     //apaga
                                   if((contador_tecla0==0)&&
                                     (contador_tecla1==0)&&
                                     (contador_tecla2==0)&&
                                     (contador_tecla3==0)&&
                                     (contador_tecla4==0)&&
                                     (contador_tecla5==0)&&
                                     (contador_tecla6==0)&&
                                     (contador_tecla7==0)&&
                                     (contador_tecla8==0)&&
                                     (contador_tecla9==0)){

                                          i--;
                                        lcd_chr(linha_lcd,coluna_lcd,' ');
                                        coluna_lcd--;
                                        buffer[i]=0x00;
                                        lcd_chr(linha_lcd,coluna_lcd,' ');
                                         Lcd_Cmd(_LCD_MOVE_CURSOR_LEFT);
                                        limpa_teclado();
                                     }

                                     else{

                                        buffer[i]=0x00;
                                        lcd_chr(linha_lcd,coluna_lcd,' ');
                                         Lcd_Cmd(_LCD_MOVE_CURSOR_LEFT);
                                          i--;
                                        limpa_teclado();
                                     }


                        goto ler_ip_novo;
                        }

                        else if(tecla==3){     //enter
                        i++;
                        buffer[i]=0x00;
                        limpa_teclado();
                        }

                        else{
                         goto ler_ip_novo;
                        }

                        //grava novo endereco IP
                        
                         for (i=0;i<strlen(buffer);i++){
                         escreve_eeprom(ip_address+i,buffer[i]);
                          }

                           escreve_eeprom(ip_address+i,0x00);


goto menu_show;
}

//alterar porta de rede wifi
else if (menu_id==11){

limpa_buffer();
           i=0;

           ler_porta_gravado:

              buffer[i]=eeprom_read(porta_address+i);
               if (buffer[i]==0){}
               else {
               i++;
               goto ler_porta_gravado;
               }

                   Lcd_Cmd(_LCD_CLEAR);
                   Lcd_Out(1,1,"Porta:");
                   Lcd_Out_CP(buffer);
                    Lcd_Out(2,1,"Insira nova PORTA:");
                    Lcd_Cmd(_LCD_THIRD_ROW);


                   i=0;
                   linha_lcd=3;
                   coluna_lcd=1;
                   Lcd_Cmd(_LCD_UNDERLINE_ON);
                   limpa_teclado();

                   ler_porta_novo:

                     ler_teclado_alfanumerico();

                            if(tecla=='*'){
                             goto menu_show;
                             }

                        if (tecla==0){
                           goto ler_porta_novo;
                        }

                        else if(tecla==4){      //caps lock
                                Lcd_Cmd(_LCD_CURSOR_OFF);
                                if (caps_lock==1){
                                    caps_lock=0;
                                    lcd_out(1,20," ");

                                    }
                                    else if(caps_lock==0){
                                    caps_lock=1;
                                    lcd_out(1,20,"^");

                                    }
                                    lcd_out(linha_lcd,coluna_lcd,"");
                                     Lcd_Cmd(_LCD_UNDERLINE_ON);
                                 goto ler_porta_novo;
                        }

                        //escreve caracter
                        else if ((tecla!=0)&&(tecla!=1)&&(tecla!=2)&&(tecla!=3)&&(tecla!=4)){

                             buffer[i]=tecla;
                             lcd_chr(linha_lcd,coluna_lcd,tecla);
                             Lcd_Cmd(_LCD_MOVE_CURSOR_LEFT);
                         goto ler_porta_novo;
                        }

                        else if(tecla==1){      //avança proximo caracter
                        i++;
                        coluna_lcd++;
                        limpa_teclado();
                        Lcd_Cmd(_LCD_MOVE_CURSOR_RIGHT);
                        goto ler_porta_novo;
                        }

                        else if(tecla==2){     //apaga
                                   if((contador_tecla0==0)&&
                                     (contador_tecla1==0)&&
                                     (contador_tecla2==0)&&
                                     (contador_tecla3==0)&&
                                     (contador_tecla4==0)&&
                                     (contador_tecla5==0)&&
                                     (contador_tecla6==0)&&
                                     (contador_tecla7==0)&&
                                     (contador_tecla8==0)&&
                                     (contador_tecla9==0)){

                                          i--;
                                        lcd_chr(linha_lcd,coluna_lcd,' ');
                                        coluna_lcd--;
                                        buffer[i]=0x00;
                                        lcd_chr(linha_lcd,coluna_lcd,' ');
                                         Lcd_Cmd(_LCD_MOVE_CURSOR_LEFT);
                                        limpa_teclado();
                                     }

                                     else{

                                        buffer[i]=0x00;
                                        lcd_chr(linha_lcd,coluna_lcd,' ');
                                         Lcd_Cmd(_LCD_MOVE_CURSOR_LEFT);
                                          i--;
                                        limpa_teclado();
                                     }


                        goto ler_porta_novo;
                        }

                        else if(tecla==3){     //enter
                        i++;
                        buffer[i]=0x00;
                        limpa_teclado();
                        }

                        else{
                         goto ler_porta_novo;
                        }

                        //grava nova porta de acesso

                         for (i=0;i<strlen(buffer);i++){
                         escreve_eeprom(porta_address+i,buffer[i]);
                          }

                           escreve_eeprom(porta_address+i,0x00);


goto menu_show;
}

//alterar DNS servidor
else if (menu_id==12){

limpa_buffer();
           i=0;

           ler_dns_gravado:

              buffer[i]=eeprom_read(dns_servidor_address+i);
               if (buffer[i]==0){}
               else {
               i++;
               goto ler_dns_gravado;
               }

                  Lcd_Cmd(_LCD_CLEAR);
                   Lcd_Out(1,1,"    DNS servidor:    ");
                   Lcd_Out(2,1,buffer);
                    Lcd_Out(3,1,"  Insira novo DNS:  ");
                    Lcd_Cmd(_LCD_FOURTH_ROW);


                   i=0;
                   linha_lcd=4;
                   coluna_lcd=1;
                   Lcd_Cmd(_LCD_UNDERLINE_ON);
                   limpa_teclado();

                   ler_dns_novo:

                     ler_teclado_alfanumerico();

                            if(tecla=='*'){
                             goto menu_show;
                             }

                        if (tecla==0){
                           goto ler_dns_novo;
                        }

                        else if(tecla==4){      //caps lock
                                Lcd_Cmd(_LCD_CURSOR_OFF);
                                if (caps_lock==1){
                                    caps_lock=0;
                                    lcd_out(1,20," ");

                                    }
                                    else if(caps_lock==0){
                                    caps_lock=1;
                                    lcd_out(1,20,"^");

                                    }
                                    lcd_out(linha_lcd,coluna_lcd,"");
                                     Lcd_Cmd(_LCD_UNDERLINE_ON);
                                 goto ler_dns_novo;
                        }

                        //escreve caracter
                        else if ((tecla!=0)&&(tecla!=1)&&(tecla!=2)&&(tecla!=3)&&(tecla!=4)){

                             buffer[i]=tecla;
                             lcd_chr(linha_lcd,coluna_lcd,tecla);
                             Lcd_Cmd(_LCD_MOVE_CURSOR_LEFT);
                         goto ler_dns_novo;
                        }

                        else if(tecla==1){      //avança proximo caracter
                        i++;
                        coluna_lcd++;
                        limpa_teclado();
                        Lcd_Cmd(_LCD_MOVE_CURSOR_RIGHT);
                        goto ler_dns_novo;
                        }

                        else if(tecla==2){     //apaga
                                   if((contador_tecla0==0)&&
                                     (contador_tecla1==0)&&
                                     (contador_tecla2==0)&&
                                     (contador_tecla3==0)&&
                                     (contador_tecla4==0)&&
                                     (contador_tecla5==0)&&
                                     (contador_tecla6==0)&&
                                     (contador_tecla7==0)&&
                                     (contador_tecla8==0)&&
                                     (contador_tecla9==0)){

                                          i--;
                                        lcd_chr(linha_lcd,coluna_lcd,' ');
                                        coluna_lcd--;
                                        buffer[i]=0x00;
                                        lcd_chr(linha_lcd,coluna_lcd,' ');
                                         Lcd_Cmd(_LCD_MOVE_CURSOR_LEFT);
                                        limpa_teclado();
                                     }

                                     else{

                                        buffer[i]=0x00;
                                        lcd_chr(linha_lcd,coluna_lcd,' ');
                                         Lcd_Cmd(_LCD_MOVE_CURSOR_LEFT);
                                          i--;
                                        limpa_teclado();
                                     }


                        goto ler_dns_novo;
                        }

                        else if(tecla==3){     //enter
                        i++;
                        buffer[i]=0x00;
                        limpa_teclado();
                        }

                        else{
                         goto ler_dns_novo;
                        }

                        //grava novo dns

                         for (i=0;i<strlen(buffer);i++){
                         escreve_eeprom(dns_servidor_address+i,buffer[i]);
                          }
                           
                           escreve_eeprom(ssid_address+i,0x00);


goto menu_show;
}


//alterar PORTA do servidor
else if (menu_id==13){

limpa_buffer();
           i=0;

           ler_porta__servidor_gravado:

              buffer[i]=eeprom_read(porta_servidor_address+i);
               if (buffer[i]==0){}
               else {
               i++;
               goto ler_porta__servidor_gravado;
               }

                   Lcd_Cmd(_LCD_CLEAR);
                   Lcd_Out(1,1,"PORTA:");
                   Lcd_Out_CP(buffer);
                    Lcd_Out(2,1,"Insira nova PORTA:");
                    Lcd_Cmd(_LCD_THIRD_ROW);


                   i=0;
                   linha_lcd=3;
                   coluna_lcd=1;
                   Lcd_Cmd(_LCD_UNDERLINE_ON);
                   limpa_teclado();

                   ler_porta_servidor_novo:

                     ler_teclado_alfanumerico();

                            if(tecla=='*'){
                             goto menu_show;
                             }

                        if (tecla==0){
                           goto ler_porta_servidor_novo;
                        }

                        else if(tecla==4){      //caps lock
                                Lcd_Cmd(_LCD_CURSOR_OFF);
                                if (caps_lock==1){
                                    caps_lock=0;
                                    lcd_out(1,20," ");

                                    }
                                    else if(caps_lock==0){
                                    caps_lock=1;
                                    lcd_out(1,20,"^");

                                    }
                                    lcd_out(linha_lcd,coluna_lcd,"");
                                     Lcd_Cmd(_LCD_UNDERLINE_ON);
                                 goto ler_porta_servidor_novo;
                        }

                        //escreve caracter
                        else if ((tecla!=0)&&(tecla!=1)&&(tecla!=2)&&(tecla!=3)&&(tecla!=4)){

                             buffer[i]=tecla;
                             lcd_chr(linha_lcd,coluna_lcd,tecla);
                             Lcd_Cmd(_LCD_MOVE_CURSOR_LEFT);
                         goto ler_porta_servidor_novo;
                        }

                        else if(tecla==1){      //avança proximo caracter
                        i++;
                        coluna_lcd++;
                        limpa_teclado();
                        Lcd_Cmd(_LCD_MOVE_CURSOR_RIGHT);
                        goto ler_porta_servidor_novo;
                        }

                        else if(tecla==2){     //apaga
                                   if((contador_tecla0==0)&&
                                     (contador_tecla1==0)&&
                                     (contador_tecla2==0)&&
                                     (contador_tecla3==0)&&
                                     (contador_tecla4==0)&&
                                     (contador_tecla5==0)&&
                                     (contador_tecla6==0)&&
                                     (contador_tecla7==0)&&
                                     (contador_tecla8==0)&&
                                     (contador_tecla9==0)){

                                          i--;
                                        lcd_chr(linha_lcd,coluna_lcd,' ');
                                        coluna_lcd--;
                                        buffer[i]=0x00;
                                        lcd_chr(linha_lcd,coluna_lcd,' ');
                                         Lcd_Cmd(_LCD_MOVE_CURSOR_LEFT);
                                        limpa_teclado();
                                     }

                                     else{

                                        buffer[i]=0x00;
                                        lcd_chr(linha_lcd,coluna_lcd,' ');
                                         Lcd_Cmd(_LCD_MOVE_CURSOR_LEFT);
                                          i--;
                                        limpa_teclado();
                                     }


                        goto ler_porta_servidor_novo;
                        }

                        else if(tecla==3){     //enter
                        i++;
                        buffer[i]=0x00;
                        limpa_teclado();
                        }

                        else{
                         goto ler_porta_servidor_novo;
                        }

                        //grava nova porta de acesso

                         for (i=0;i<strlen(buffer);i++){
                         escreve_eeprom(porta_servidor_address+i,buffer[i]);
                          }


                           escreve_eeprom(porta_servidor_address+i,0x00);


goto menu_show;
}


//reset do sistema
else if (menu_id==14){

      Lcd_Cmd(_LCD_CLEAR);
      Lcd_Out(2,1,"     PRESSIONE      ");
      Lcd_Out(3,1,"      TECLA D       ");

     ler_reset_sistema:
                   limpa_teclado();
                   ler_teclado_alfanumerico();
                        
                            if(tecla=='*'){
                             goto menu_show;
                             }

                      
                    else if(tecla==4){
                    
                          delay_ms(100);

                    }

                   else if (tecla==0){
                    goto ler_reset_sistema;
                    }
                                    
                   else{
                   goto ler_reset_sistema;
                   }

                    reset_ok:
                    
                    Lcd_Cmd(_LCD_CLEAR);
                    
                    escreve_eeprom(flag_first_time_address,0x00);

                                  
                                   Lcd_Cmd(_LCD_CLEAR);
                                   Lcd_Out(1,1,"      SISTEMA       ");
                                   Lcd_Out(2,1,"         A          ");
                                   Lcd_Out(3,1,"     REINICIAR!     ");
                
                                   buzzer=1;
                                   delay_ms (2000);
                                   buzzer=0;
                    asm{reset}
                    
                    }
fim_menu_lcd:

fechadura_principal=1;
INTCON.F7=1;
INTCON.F6=1;
INTCON.F5=1;
}


void menu_pin(){
/*le o pin e apresenta menu
*/

limpa_buffer();

Lcd_Cmd(_LCD_CLEAR);
Lcd_Out(2,1,"Insira PIN:");

i=0;
        ler_pin_entrada:


            ler_teclado_alfanumerico();
            
              if (tecla==0){
                 goto ler_pin_entrada;
              }

            limpa_teclado();
           Lcd_chr_CP('*');
           buffer[i]=tecla;
           tecla==0x00;
            i++;

            if (i<4){
               goto ler_pin_entrada;
            }

delay_ms(1000);



//ler pin
y=0;
for(i=4;i<8;i++){
buffer[i]=eeprom_read(pin_admin_address+y);
y++;
}


buffer[12]=0x00;


//variavel buffer contem os 3 PINS para comparacao//
// PIN INSERIDO|PIN ADMIN|PIN TECNICO

if (buffer[0]==buffer[4]&&buffer[1]==buffer[5]&&buffer[2]==buffer[6]&&buffer[3]==buffer[7]){
//PIN .abre fechadura e apresenta menu configuracao.


restart_menu:

Lcd_Cmd(_LCD_CLEAR);

  fechadura_principal=0;
  
  Lcd_Cmd(_LCD_CLEAR);
  lcd_out(1,1," ACESSO  AUTORIZADO ");
  lcd_out(3,1," Prima #  para menu" );
  lcd_out(4,1," Prima *  para sair" );
  
  ler_tecla:


            limpa_teclado();
            ler_teclado_alfanumerico();



                   if(tecla==0){

                       goto ler_tecla;

                   }

                   else if(tecla=='#'){

                   menu_lcd();

                   }
                   
                   else if(tecla=='*'){

                   goto fim_menu;

                   }

  
  goto fim_menu;
}

else if(buffer[0]=='7'&&buffer[1]=='2'&&buffer[2]=='7'&&buffer[3]=='2'){
//PIN developer.abre fechadura e apresenta menu configuracao.mesmo do admin.grava log de acesso

restart_menu_developer:

Lcd_Cmd(_LCD_CLEAR);

  fechadura_principal=0;

  Lcd_Cmd(_LCD_CLEAR);
  lcd_out(1,1," ACESSO  AUTORIZADO ");
  lcd_out(3,1," Prima #  para menu" );
  lcd_out(4,1," Prima *  para sair" );

  ler_tecla_developer:


            limpa_teclado();
            ler_teclado_alfanumerico();



                   if(tecla==0){

                       goto ler_tecla_developer;

                   }

                   else if(tecla=='#'){

                   menu_lcd();

                   }

                   else if(tecla=='*'){

                   goto fim_menu;

                   }


  goto fim_menu;
}

else{
//PIN errado.grava log de tentativa de entrada.
buzzer=1;
Lcd_Cmd(_LCD_CLEAR);
Lcd_Out(2,1,"   ACESSO NEGADO!   ");

//ler id para alarme
limpa_buffer();
strcpy(buffer,"Tentativa de acesso nao autorizado. ID:");

            yy=0;
            i=39;
           ler_id_alarme:

              buffer[i]=eeprom_read(id_address+yy);
               if (buffer[i]==0){}
               else {
               i++;
               yy++;
               goto ler_id_alarme;
               }
envia_sms(buffer);


delay_ms(1000);
goto fim_menu;
}


fim_menu:
buzzer=0;
fechadura_principal=1;

Lcd_Cmd(_LCD_CLEAR);
}


void verifica_ocorrencias(){

Lcd_Cmd(_LCD_CURSOR_OFF);
limpa_buffer();
strcpy(buffer,"  LUSOTEC GROUP SA  ");
Lcd_Out(1,1,buffer);

limpa_buffer();
strcpy(buffer,"   Sistema ligado   ");
Lcd_Out(2,1,buffer);


if (flag_rtc==1){
     T0CON.F7=0;
      ler_rtc();
       lcd_out(3,1,buffer);
       flag_rtc=0;
       T0CON.F7=1;
     }


/*if(flag_temperatura==1){
     ler_temperatura();
              buffer[0]=eeprom_read(limite_temperatura_address);
              if (temp>= buffer[0]){
              //fan=1;

              UART2_Init(9600);
              UART_Set_Active(&UART2_Read, &UART2_Write, &UART2_Data_Ready, &UART2_Tx_Idle);

              UART2_Write_text("fan_on\r\n"); //comando para ligar ventoinha
              }
              else{
              //fan=0;
              UART2_Init(9600);
              UART_Set_Active(&UART2_Read, &UART2_Write, &UART2_Data_Ready, &UART2_Tx_Idle);

              UART2_Write_text("fan_off\r\n"); //comando para desligar ventoinha

              }
       escrever_temperatura(temp);
       flag_temperatura=0;
}*/
ler_teclado_alfanumerico();

if (tecla=='#'){
       T0CON.F7=0;
       INTCON.F7=0;
       INTCON.F6=0;
       menu_pin();
}
if (tamper==1){

delay_ms(1000);
Lcd_Cmd(_LCD_CLEAR);
lcd_out(2,1,"      ABERTURA      ");
lcd_out(3,1,"  NAO AUTORIZADA!   ");

buzzer=1;
sirene=1;

//enviar sms e wifi de abertura nao autorizada e gravar log

//ler id para alarme
limpa_buffer();
strcpy(buffer,"Alarme de abertura nao autorizada. ID:");

            yy=0;
            i=39;
           ler_id_alarme:

              buffer[i]=eeprom_read(id_address+yy);
               if (buffer[i]==0){}
               else {
               i++;
               yy++;
               goto ler_id_alarme;
               }
envia_sms(buffer);


      while(tamper==1){

      }

delay_ms(1000);
Lcd_Cmd(_LCD_CLEAR);
}


buzzer=0;
sirene=0;
INTCON.F7=1;
INTCON.F6=1;
INTCON.F5=1;
}

void liga_primeira_vez(){
          
          Lcd_Out(2,1,"Aguarde...");
          Lcd_cmd(_LCD_THIRD_ROW);
          temp=0;
       
        //limpa eeprom
        for (i=0;i<1024;i++){

          escreve_eeprom(0x00+i,0x00);
          temp++;
          
                 if (temp==50){
                  lcd_chr_cp(0xff);
                  temp=0;
                 }
          }


       escreve_eeprom(flag_first_time_address,0xfd);
                          
     //PIN tecnico por defeito
       limpa_buffer();
       strcpy(buffer,"1234");

          for (i=0;i<strlen(buffer);i++){

          escreve_eeprom(pin_tecnico_address+i,buffer[i]);

          }

//configs por defeito
//numero_gsm_address=0x000D; //de 0x0d a 0x1b-15 posicoes de memoria
 limpa_buffer();
       strcpy(buffer,"+351918972411");

          for (i=0;i<strlen(buffer);i++){

          escreve_eeprom(numero_gsm_address+i,buffer[i]);

          }

//nome id por defeito; id_address=0x001C; //0x1c a 0x4c-48 posicoes de memoria
 limpa_buffer();
       strcpy(buffer,"TORRE_001");

          for (i=0;i<strlen(buffer);i++){

          escreve_eeprom(id_address+i,buffer[i]);

          }

//nome de rede wifi por defeito; ssid_address=0x7A; //0x7a a 0xaa-48 posicoes de memoria
  limpa_buffer();
       strcpy(buffer,"Rede_Wifi_1");

          for (i=0;i<strlen(buffer);i++){

          escreve_eeprom(ssid_address+i,buffer[i]);

          }

//password de rede wifi por defeito; ssid_password_address=0xAB; //0xab a 0xdb-48 posicoes de memoria
 limpa_buffer();
       strcpy(buffer,"wifi1234");

          for (i=0;i<strlen(buffer);i++){

          escreve_eeprom(ssid_password_address+i,buffer[i]);

          }

//endereço IP por defeito; ip_address=0x5F;  //0x5f a 0x6f-16 posicoes de memoria
 limpa_buffer();
       strcpy(buffer,"192.168.1.2");

          for (i=0;i<strlen(buffer);i++){

          escreve_eeprom(ip_address+i,buffer[i]);

          }

//porta de rede por defeito; porta_address=0x70; //0x70 a 0x79-9 posicoes de memoria
 limpa_buffer();
       strcpy(buffer,"10001");

          for (i=0;i<strlen(buffer);i++){

          escreve_eeprom(porta_address+i,buffer[i]);

          }

//nome de DNS servidor por defeito;
  limpa_buffer();
       strcpy(buffer,"workserver.ddns.net");

          for (i=0;i<strlen(buffer);i++){

          escreve_eeprom(dns_servidor_address+i,buffer[i]);

          }
          
//porta de servidor por defeito
 limpa_buffer();
       strcpy(buffer,"10001");

          for (i=0;i<strlen(buffer);i++){

          escreve_eeprom(porta_servidor_address+i,buffer[i]);

          }
          
//limite da temperatura por defeito-35.C
escreve_eeprom(limite_temperatura_address,35);

       Lcd_Cmd(_LCD_CLEAR);
       Lcd_Out(1,1,"     Insira novo    ");
       Lcd_Out(2,1,"     PIN acesso     ");
       Lcd_Out(3,1," ");

        i=0;
        ler_pin:

            limpa_teclado();
            ler_teclado_alfanumerico();

              if (tecla==0){
                 goto ler_pin;
              }

           Lcd_Out_CP("*");
           escreve_eeprom(pin_admin_address+i,tecla);
           tecla==0x00;
            i++;

            if (i<4){
               goto ler_pin;
            }
        delay_ms(1000);
       Lcd_Cmd(_LCD_CLEAR);
       delay_ms(1000);
       Lcd_Out(2,1,"     PIN acesso     ");
       Lcd_Out(3,1,"      alterado      ");
       delay_ms(1000);
        
        Lcd_Cmd(_LCD_CLEAR);
       delay_ms(150);
       Lcd_Out(2,1,"     Sistema a      ");
       Lcd_Out(3,1,"     reiniciar      ");
       delay_ms(1500);
       asm{reset}

}

/*void grava_log(void){

//last_data_addr_h=0x00CB
//last_data_addr_l=0x00CA
yy=eeprom_read(last_data_addr_h);
yy=yy<<8;
yy=eeprom_read(last_data_addr_l);
//yy tem agora o ultimo endereco escrito da eeprom externa

soft_i2c_init();

 for(i=0;i<strlen(buffer);i++){
      grava_log_eeprom(yy+i,buffer[i]);
       }

yy=eeprom_read(last_data_addr_h);
yy=yy<<8;
yy=eeprom_read(last_data_addr_l);

yy=yy+strlen(buffer);
escreve_eeprom(last_data_addr_l,yy);
yy=yy<<8;
escreve_eeprom(last_data_addr_h,yy);

}*/


void escreve_eeprom(unsigned int endereco,unsigned char dados){

INTCON.GIE=0x00;
 EEADR=endereco;
 endereco=endereco>>8;
EEADRH=endereco;
 EEDATA=dados;
 EECON1.EEPGD=0X00;
 EECON1.CFGS=0X00;
 EECON1.WREN=0X01;
 EECON2=0X55;
 EECON2=0XAA;
 EECON1.WR=1;

 while(EECON1.WR==1){

 }

 eecon1.WREN=0x00;

}
/*void grava_log_eeprom(int endereco,char dados){
INTCON.F7=0;
INTCON.F6=0;
INTCON.F5=0;
T0CON.F7=0;


soft_i2c_start();
delay_us(150);
soft_i2c_write(0xa0);  //enderece escreve
delay_us(150);
temp=endereco;
endereco=endereco>>8;

soft_i2c_write(endereco);
soft_i2c_write(temp);
soft_i2c_write(dados);
delay_ms(5);
soft_i2c_stop();

}*/
int ler_temperatura() {

INTCON.F7=0;
INTCON.F6=0;
INTCON.F5=0;
T0CON.F7=0;
  
limpa_buffer();

UART2_Init(9600);
UART_Set_Active(&UART2_Read, &UART2_Write, &UART2_Data_Ready, &UART2_Tx_Idle);

UART2_Write_text("Temperatura?\r"); //comando para pedir temperatura

while (UART2_Data_Ready() != 1){

   }
   temp=UART2_Read();

/*Ow_Reset(&PORTD,7);
  Ow_Write(&PORTD,7,0xCC);
  Ow_Write(&PORTD,7,0x4E);
   Ow_Write(&PORTD,7,0xff);  //h_temp
   Ow_Write(&PORTD,7,0x00);  //l_temp
   Ow_Write(&PORTD,7,0x7F);  //12 bits

  Ow_Reset(&PORTD,7);
  Ow_Write(&PORTD,7,0xCC);
  Ow_Write(&PORTD,7,0x44);   //start conversion

  // delay_ms(750);

  Ow_Reset(&PORTD,7);
  Ow_Write(&PORTD,7,0xCC);
  Ow_Write(&PORTD,7,0xBE);   //read conversion

     temp =  Ow_Read(&PORTD, 7);
    temp = (Ow_Read(&PORTD, 7) << 8) + temp;

    Ow_Reset(&PORTD,7);

 if (temp_0==1){
     ponto_dec=1;
     }
  else
      {
      ponto_dec=0;
      }

  temp=temp>>4;
   */
   return  temp;
  
}

void escrever_temperatura(int temp){
     for(i=0;i<5;i++){
        temperatura1[i]=0;
     }
    
    ByteToStr(temp,temperatura1);
    lcd_out(4,1," ");
    lcd_out_cp(temperatura1);

  if (ponto_dec==1){

      lcd_chr_cp('.');
      lcd_chr_cp('5');
      lcd_chr_cp(0xDF); //0xb0 simbolo grau
      lcd_chr_cp('C');
      }
  else
      {
      lcd_chr_cp('.');
      lcd_chr_cp('0');
      lcd_chr_cp(0xDF); //0xb0 simbolo grau
      lcd_chr_cp('C');
      }

}

void configura_gsm(){
INTCON.F7=0;
INTCON.F6=0;
INTCON.F5=0;
T0CON.F7=0;

  UART1_Init(9600);
  Delay_ms(100);

  UART_Set_Active(&UART1_Read, &UART1_Write, &UART1_Data_Ready, &UART1_Tx_Idle);

  UART1_Write_text("at+cscs=\"GSM\"\r");  //modo gsm
  delay_ms (100);
  UART1_Write_text("at+cmgf=1\r");   //modo texto sms
  delay_ms (100);
  UART1_Write_text("at+cnmi=2,1,0,1,0\r");   //notificacao sms
  delay_ms (100);
  UART1_Write_text("ate0\r");   //echo off
  delay_ms (5000);   //espera sim card ready e registo na rede
  UART1_Write_text("at+cmgd=1,4\r");    //apaga mensagens
  delay_ms (1000);
 
 INTCON.F7=1;
INTCON.F6=1;
INTCON.F5=1;
T0CON.F7=1;
}

void envia_sms(char texto_sms[]){
char texto_sms[160];

INTCON.F7=0;
INTCON.F6=0;
INTCON.F5=0;
T0CON.F7=0;


strcpy(texto_sms,buffer);
//ler telefone alarmes para alarmes
limpa_buffer();
i=0;
i=0;

 ler_num_gsm:

    buffer[i]=eeprom_read(numero_gsm_address+i);
       if (buffer[i]==0){}
       else {
       i++;
       goto ler_num_gsm;
       }

rtrim (buffer);

UART1_Init(9600);
Delay_ms(100);
UART_Set_Active(&UART1_Read, &UART1_Write, &UART1_Data_Ready, &UART1_Tx_Idle);

UART1_Write_text("at+cscs=\"GSM\"\r");
delay_ms (100);
UART1_Write_text("at+cmgf=1\r");
delay_ms (100);
UART1_Write_text("at+cmgs=\"");
UART1_Write_text(buffer); //numero gsm
UART1_Write_text("\"\r");
delay_ms (100);
UART1_Write_text(texto_sms);
UART1_Write(26);

delay_ms (2000);

UART1_Write_text("at+cmgd=1,4\r"); //apaga mensagens    0,4
delay_ms (2000);

INTCON.F7=1;
INTCON.F6=1;
INTCON.F5=1;
T0CON.F7=1;
}

//************************* INTERRUPCOES ***************************************
void interrupt() iv 0x0008 ics ICS_AUTO {

INTCON.F7=0;
INTCON.F6=0;
TMR0IE_bit=0;
T0CON.F7=0;
T1CON.F0=0;

if (TMR0IF_bit==1){


         TMR0IF_bit=0;
          TMR0H=0x00;
          TMR0L=0x00;
               
                dump_time++;

                   if(dump_time==25){
                   dump_time=0;
                   flag_rtc=1;
                   //flag_temperatura=1;
                   }
    }


INTCON.F7=1;
INTCON.F6=1;

}

//************************** MAIN LOOP *****************************************
void main() {

//configura oscilador interno 16Mhz
OSCCON=0x77;

//estado inicial das variaveis
tecla=0;
i=0;
y=0;
erro=0;
flag_teclado=0;
flag_rtc=0;
endereco=0;
dados=0;
caps_lock=0;
dump_time=0;

linha_lcd=1;
coluna_lcd=1;
dump=0;
contador_tecla0=0;
contador_tecla1=0;
contador_tecla2=0;
contador_tecla3=0;
contador_tecla4=0;
contador_tecla5=0;
contador_tecla6=0;
contador_tecla7=0;
contador_tecla8=0;
contador_tecla9=0;

limpa_buffer();

configura_io();
configura_timer0();

//Configura RTC
configura_rtc();

//entradas/saidas estado inicial
buzzer=0;
sirene=0;
fechadura_principal=1;

//Inicia LCD
Delay_ms(250);
Lcd_Init();
Lcd_Cmd(_LCD_CURSOR_OFF);
Lcd_Cmd(_LCD_CLEAR);


//Nome empresa
delay_ms(1000);
Lcd_Out(2,1,"  LUSOTEC GROUP SA  ");
Lcd_Out(3,1,"        2018        ");

delay_ms(2000);
Lcd_Cmd(_LCD_CLEAR);

//verifica se e a primeira vez que sistema e ligado. se sim configura PIN Admin.
limpa_buffer();

buffer[0]=eeprom_read(flag_first_time_address);

    if (buffer[0]!=0xfd){

          liga_primeira_vez();
    }
    

Lcd_Out(2,1,"    A configurar    ");
Lcd_Out(3,1,"        GSM         ");
delay_ms(2000);
configura_gsm();

ler_rtc();
lcd_out(3,1,buffer);

// main loop //////////////////////////////////////////////////////////////////
//UART2 PoE; UART1 GSM


//liga interrupcoes
configura_interrupcoes();

   while(1){

      verifica_ocorrencias();

  INTCON.F7=1;
    INTCON.F6=1;
    INTCON.F5=1;
    T0CON.F7=1;

   }

}