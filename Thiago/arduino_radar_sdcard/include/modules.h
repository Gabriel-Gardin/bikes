//***********INCLUDES*********
#include <Arduino.h>
#include <TinyGPS++.h>
#include <SD.h>
#include<Wire.h> 

TinyGPSPlus gps;
 
//Millis desde a ultima vez que checaram os sensores.
unsigned long check_time = 0;

//Sinaliza se houve um evento de velocidade para pegar os outros parametros.
volatile bool get_data = false;

#define SerialGPS Serial1  // RX&TX PINS

//Pino conectado ao pino 2 (pwm) do módulo ultrasonico maxbotix. 
#define ultrasonic_pin 4

//Dados recebido pelo i2c. Velocidade radar.
volatile uint8_t SlaveReceived;

//Verifica os dados do GPS através da serial UART do módulo 6neo m e retorna um ponteiro para um array contendo 4 parametros; Latitude, Longitude, numero satelite, e a velocidade da BICICLETA. 
String *get_gps_data();

//Função chamada para ler o sensor ultrasônico da MaxBotix. Pode ser chamada 10 vezes por segundo. Retorna a distância em centimetros. 
float get_distance();

//Callback I2C
void receiveEvent(int howMany);

//Função que envia os dados para o servidor por mqtt.
int save_data(double speed, float distance, String *gpss);

unsigned long saved_time = 0;


//Constante usada para converter a frequência do radar CDM324 em velocidade.
unsigned int doppler_div = 44; 

//variáveis.
unsigned int ultrasonic_pulse;
