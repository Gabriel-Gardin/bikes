#include <TinyGPS++.h>
#include <SPI.h>
#include <SD.h>
#include "DHT.h"
#include <ArduinoJson.h>

//Instância a classe TinyGPS.
TinyGPSPlus gps;

//Pino do modulo de temperatura DHT22
#define temp_pin 7

//Chip select
#define chipSelect 53

//Pino analógico do módulo de U.V
#define uv_pin A1

//Pino analógico do módulo de gás
#define gas_pin A0

//Rx do gps neo 6m
#define gps_rx 18

//Tx do gps neo 6m
#define gps_tx 19

//Le o sensor de gás e retorna o vlaor em da leitura analógica. 0 - 1023;
int get_gas();

//Le o sensor de ultra violeta e retorna o valor analógico. 0 - 1023
int get_uv(); 

//Recebe os dados já serializados pela biblioteca json e salva no sd card.
void save_data(char *data);

//Retorna os dados do gps na forma de um ponteiro.
String * gps_data();

//Checa todos os sensores
void check_all_sensors();







