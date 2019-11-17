  #include <Arduino.h>
#include "definitions.h"

#define DEBUG true
#if DEBUG == true
#define LOG(x) Serial.print(x)
#else
#define LOG(x)
#endif

DHT dht(temp_pin, DHT22);

void setup()
{
  dht.begin();
  Serial.begin(9600);  //Comentar essa linha se não estiver debugando...
  Serial2.begin(9600); //Serial do GPS.
  LOG("iniciando o cartão");
  if (!SD.begin(chipSelect)) {
    LOG("Card failed, or not present");
  }
}

void loop()
{
  check_all_sensors();
  Serial.println();
  unsigned long t1 = millis();
  while(millis() - t1 < 3000){}
}

void check_all_sensors()
{
  DynamicJsonDocument json_data(200);
  json_data.clear();

  String *gps;
  gps = gps_data();
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  int gas = get_gas();
  int uv = get_uv();

  json_data["Humidade"] = humidity;
  json_data["Temperatura"] = temperature;
  json_data["U.V"] = uv;
  json_data["gas"] = gas;
  JsonArray gps_json_data = json_data.createNestedArray("GPS_DATA");

  for(int i = 0; i < 6; i++)
  {
    gps_json_data.add(gps[i]);
    //Serial.println(gpss[i]);
  }

  char data[1024];
  serializeJson(json_data, data);

  save_data(data);
}

int get_gas()
{
  unsigned long t0;
  int gas_voltage = analogRead(gas_pin);
  while(millis() - t0 < 1){} //Pausa por 1 ms
  return gas_voltage;
}

int get_uv()
{
  unsigned long t0;
  int uv_voltage = analogRead(uv_pin);
  while(millis() - t0 < 1){} //Pausa por 1 ms
  return uv_voltage;
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

String *gps_data()
{
  static String gps_data[6];
  bool newData = false;
  // Espera 0.5s para garantir que recebeu todos os dados pela serial do modulo de GPS.
  for (unsigned long start = millis(); millis() - start < 500;)
  {
    if (Serial2.available())
    {
      char c = Serial2.read();
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
        
        return gps_data;
      }
    }
  }
}