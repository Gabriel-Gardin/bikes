//***********INCLUDES*********
#include <Arduino.h>
#include "TinyGPS.h"
#include <MKRGSM.h>

TinyGPS gps;
 
GSMClient client;
GPRS gprs;
GSM gsmAccess;
  

#define SerialGPS Serial1  // RX&TX PINS
//#define SerialGPS Serial3 // SCL&SDA PINS

//*********pin definitions *****************
#define radar_pin 5
#define AVERAGE 4
#define ultrasonic_pin 4

//Verifica os dados do GPS através da serial UART do módulo 6neo m e retorna um ponteiro para um array contendo 4 parametros; Latitude, Longitude, numero satelite, e a velocidade da BICICLETA. 
String * gps_data(); 

// Função chamada para ler a frequência de saída do módulo radar CDM324. Retorna a velocidade do objeto que estiver se aproximando em km/h 
float get_cars_speed();

//Função que conecta o gsm para ter acesso a rede de dados móveis. 
void gsm_connect_func();

//Função chamada para ler o sensor ultrasônico da MaxBotix. Pode ser chamada 10 vezes por segundo. Retorna a distância em centimetros. 
float get_distance();


//***********VARIABLES*************
unsigned int doppler_div = 44; //Constante usada para converter a frequência do radar CDM324 em velocidade.
unsigned int samples[AVERAGE];
unsigned int x;
unsigned int ultrasonic_pulse;
float bike_speed;

/*
char server[] = "rootsaid.com"; //Enter the Web Server
char path[] = "/rootsaid.txt"; //Enter the Path of the File
int port = 80; //Enter the Port
*/

//#define CHECK_INTERVAL 5  //seconds.

unsigned long previousTest;
bool mGPS_got_line = false, mGPS_paused = false;
uint8_t mGPS_idx=0;
char mGPS_TempLine[120];