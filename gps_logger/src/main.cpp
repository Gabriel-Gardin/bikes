/***********************************************************************************************
* GPS logger desenvolvido para o Prof. Carlos departamento de engenharia civil da UEL.         *
*                                                                                              *
* Utiliza um arduino mkrzero com SD card integrado, e módulo gps neo-6m.                       *
* Notas:                                                                                       *
* O sd card não deve ser maior do que 2GB;                                                     *
* Todas as bibliotecas usadas estão disponíveis dentro da pasta lib do projeto.                *
* Compilar este código utilizando na plataforma ARDUINO.                                       *
************************************************************************************************/

#include <Arduino.h>
#include <SD.h>
#include <TinyGPS++.h>

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
void save_data(String data);

//Retorna os dados do gps na forma de um ponteiro.
String gps_data();

const int chipSelect = SS1;


void setup() {
  // put your setup code here, to run once:
  //Serial.begin(9600);  //Comentar essa linha se não estiver debugando...
  Serial1.begin(9600); //Serial do GPS.
  LOG("iniciando o cartão");
  if (!SD.begin(chipSelect)) {
    LOG("Card failed, or not present");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  String gps = gps_data();
  if(gps.length() > 2)
  {
    save_data(gps);
  }
  delay(DELAY_TIME);
}

void save_data(String data)
{
  File dataFile = SD.open("DATA.TXT", FILE_WRITE);
  if (dataFile)
  {
      dataFile.println(data);
      dataFile.close();
  }
}


String gps_data()
{
  String gps_dados;
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
      String flat, flon, satelite_number, bike_speed, time, date;
      if(gps.location.isValid())
      {
        flat = ((String)(gps.location.lat() * 1000000));
        flon = ((String)(gps.location.lng() * 1000000));
        bike_speed = (String)gps.speed.kmph();
        satelite_number = (String)gps.satellites.value();
        time = (String)gps.time.value();
        date = (String)gps.date.value();
        gps_dados = flat + "," + flon + "," + bike_speed + "," + satelite_number + "," + time + "," + date;
        return gps_dados;
      }
    }
  }
}