//***********INCLUDES*********
#include <Arduino.h>
#include <TinyGPS++.h>
#include <MKRGSM.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include<Wire.h> 

TinyGPSPlus gps;
 
GSMClient client;
GPRS gprs;
GSM gsmAccess;

//tópico do MQTT 
char* mqtttopic = "/ctu/bike1";

//Servidor MQTT
char* mqttserver = "co2incubator.xyz";

//usuário configurado no broker com permissão para enviar dados.
const char* mqttUser = "estufa1";

//Senha do usuário client1 do mqtt
const char* mqttPassword = "co2incubator132";

//millis() desde o ultimo envio de dados
unsigned long mqtt_time_send = 0;

//Millis desde a ultima vez que checaram os sensores.
unsigned long check_time = 0;

//Sinaliza se houve um evento de velocidade para pegar os outros parametros.
volatile bool get_data = false;

PubSubClient mqttclient(client); //Instância da classe do Pubcubcliente passando o construtor "client" do GSM


#define SerialGPS Serial1  // RX&TX PINS

//Número de leituras para calcular a média da frequência do módulo radar cdm324.
const uint8_t AVERAGE = 25; 

//Pino conectado ao pino 2 (pwm) do módulo ultrasonico maxbotix. 
const uint8_t ultrasonic_pin = 4;

//Dados recebido pelo i2c. Velocidade radar.
volatile uint8_t SlaveReceived;

//Verifica os dados do GPS através da serial UART do módulo 6neo m e retorna um ponteiro para um array contendo 4 parametros; Latitude, Longitude, numero satelite, e a velocidade da BICICLETA. 
String *get_gps_data();

//Função que conecta o gsm para ter acesso a rede de dados móveis e tbm conecta ao broker MQTT. 
void gsm_connect_func();

//Função chamada para ler o sensor ultrasônico da MaxBotix. Pode ser chamada 10 vezes por segundo. Retorna a distância em centimetros. 
float get_distance();

//Reconecta ao mqtt broker.
void mqtt_reconect();

//Callback I2C
void receiveEvent(int howMany);

//Função que envia os dados para o servidor por mqtt.
int send_data(double speed, float distance, String *gpss);



//Constante usada para converter a frequência do radar CDM324 em velocidade.
unsigned int doppler_div = 44; 



//variáveis.

unsigned int samples[AVERAGE];
unsigned int x;
unsigned int ultrasonic_pulse;

unsigned long previousTest;
bool mGPS_got_line = false, mGPS_paused = false;
uint8_t mGPS_idx=0;
char mGPS_TempLine[120];
