//***********INCLUDES*********
#include <Arduino.h>
#include "TinyGPS.h"
#include <MKRGSM.h>

TinyGPS gps;
 

#define SerialGPS Serial1  // RX&TX PINS
//#define SerialGPS Serial3 // SCL&SDA PINS

//*********pin definitions *****************
#define radar_pin 5
#define AVERAGE 4
#define ultrasonic_pin 4

//**************functions****************
String * gps_data();
float get_cars_speed();
void gsm_connect();
float get_distance();


//***********VARIABLES*************
unsigned int doppler_div = 44;
unsigned int samples[AVERAGE];
unsigned int x;
unsigned int ultrasonic_pulse;
float bike_speed;

char server[] = "rootsaid.com"; //Enter the Web Server
char path[] = "/rootsaid.txt"; //Enter the Path of the File
int port = 80; //Enter the Port

#define CHECK_INTERVAL 5  //seconds.

unsigned long previousTest;
bool mGPS_got_line = false, mGPS_paused = false;
uint8_t mGPS_idx=0;
char mGPS_TempLine[120];