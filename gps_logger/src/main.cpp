/***********************************************************************************************
* GPS logger desenvolvido para o Prof. Carlos departamento de engenharia civil da UEL.         *
*                                                                                              *
* Utiliza um arduino mkrzero com SD card integrado, e módulo gps neo-6m.                       *
* Notas:                                                                                       *
* O sd card não deve ser maior do que 2GB;                                                     *
* Todas as bibliotecas usadas estão disponíveis dentro da pasta lib do projeto.                *
* Compilar este código utilizando o plugin Platformio do VS code.                              *
************************************************************************************************/

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SD.h>
#include <TinyGPS++.h>
#include <ArduinoLowPower.h>

#define DEBUG true
#if DEBUG == true   //Mudar para false quando não estiver debugando por um computador... 
#define LOG(x) Serial.print(x)
#else
#define LOG(x)
#endif

#define DELAY_TIME 5000  //Tempo em milisegundos que espera para ler o gps e salvar os dados...


//Instância a classe TinyGPS.
TinyGPSPlus gps;

//Recebe os dados e salva no sd card.
void save_data(char *data);

//Retorna os dados do gps na forma de um ponteiro.
char * gps_data();

const int chipSelect = SS1;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);  //Comentar essa linha se não estiver debugando...
  Serial1.begin(9600); //Serial do GPS.
  LOG("iniciando o cartão");
  if (!SD.begin(chipSelect)) {
    LOG("Card failed, or not present");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  char *gps;
  gps = gps_data();
  save_data(gps);
  LowPower.sleep(DELAY_TIME);  //Utiliza a sleep da biblioteca low power, diminui o uso da bateria....
}

void save_data(char *data)
{
  File dataFile = SD.open("data.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println(data);
    dataFile.close();
    LOG(data);
  }
}

char *gps_data()
{
  static char gps_data[6];
  bool newData = false;
  // Espera 0.5s para garantir que recebeu todos os dados pela serial do modulo de GPS.
  for (unsigned long start = millis(); millis() - start < 500;)
  {
    if (Serial1.available())
    {
      char c = Serial1.read();
//      LOG(c);
      // Serial.write(c); //apague o comentario para mostrar os dados crus
      if (gps.encode(c)){ // Atribui true para newData caso novos dados sejam recebidos
        newData = true;
    }
  }
    if (newData)
    {
      float flat, flon, satelite_number, bike_speed, time, date;
      if(gps.location.isValid())
      {
        flat = gps.location.lat();
        flon = gps.location.lng();
        bike_speed = gps.speed.kmph();
        satelite_number = gps.satellites.value();
        time = gps.time.value();
        date = gps.date.value();
        gps_data[0] = flat * 1000000;
        gps_data[1] = flon * 1000000;
        gps_data[2] = bike_speed;
        gps_data[3] = satelite_number;
        gps_data[4] = time;
        gps_data[5] = date;
        //gps_data[6] = \0; //Terminado da string...
        
        return gps_data;
      }
    }
  }
}

void dummy() {
  // This function will be called once on device wakeup
  // You can do some little operations here (like changing variables which will be used in the loop)
  // Remember to avoid calling delay() and long running functions since this functions executes in interrupt context
}