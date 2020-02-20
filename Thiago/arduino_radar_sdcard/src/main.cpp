#include <modules.h>  


#define DEBUG true

#if DEBUG == true
#define LOG(X) Serial.print(X);
#else
#define LOG(X)
#endif

String * received_gps;

void setup() {
  
  pinMode(ultrasonic_pin, INPUT);
  Wire.begin(8);                          //Begins I2C communication with Slave Address as 8 at pin (A4,A5)
  const int chipSelect = SS1;

  if (!SD.begin(chipSelect))
  {
    LOG("Erro ao iniciar o cartao");
  }
  else
  {
    LOG("Cartao iniciado com sucesso\n")
  }
  
  delay(100);

  File dataFile = SD.open("DATA.TXT", FILE_WRITE);
  
  if (dataFile)
  {
    String data = "tempo_on,velocidade,distancia,latitude,longitude,bike_speed,hora";
    dataFile.println(data);
    dataFile.close();
  }

  Wire.onReceive(receiveEvent);

  Serial.begin(9600); //Comentar
  SerialGPS.begin(9600); //TEST GPS.

  received_gps = get_gps_data();
}

void loop()
{
  float car_distance;
  uint8_t car_speed;
  int saved;

  if(millis() - check_gps_time > 3000) //Atualiza o GPS a cada 3 segundos
  {
    received_gps = get_gps_data();
    check_gps_time = millis();
  }

  if(get_data)
  {
    car_speed = SlaveReceived;
    get_data = false;
  }
  else
  {
    car_speed = 0;
  }

  car_distance = get_distance();
  on_time = millis()/1000;
  LOG("Distancia:");
  LOG(car_distance);
  LOG("\n")
  saved = save_data(on_time, car_speed, car_distance, received_gps);
  delay(100);

  if(saved == 1)
  {
    LOG("Dados salvo com sucesso\n");
  }
  else
  {
    LOG("Erro ao salvar os dados");
  }
}



int save_data(unsigned long on_time, double speed, float distance, String *gpss)
{
  String on_t = (String) on_time;
  String str_speed = (String) speed;
  String str_distance = (String) distance;
  String data;
  char comma = ',';

  if(gpss[0] != NULL)
  {
    data = on_t + comma + str_speed + comma +str_distance + comma + gpss[0] + comma + gpss[1] + comma + gpss[2] + comma + gpss[3];
  }
  else
  {
    data = on_t + comma + str_speed + comma +str_distance;
  }
  

  File dataFile = SD.open("DATA.TXT", FILE_WRITE);
  
  if (dataFile)
  {
      dataFile.println(data);
      dataFile.close();
      return 1;
  }
  else
  {
    return 0;
  }
}

String *get_gps_data()
{
  static String gps_data[4];

  bool newData = false;
  // Espera 0.5s para garantir que recebeu todos os dados pela serial do modulo de GPS.
  for (unsigned long start = millis(); millis() - start < 500;)
  {
    if (SerialGPS.available())
    {
      char c = SerialGPS.read();
      // Serial.write(c); //apague o comentario para mostrar os dados crus
      if (gps.encode(c)){ // Atribui true para newData caso novos dados sejam recebidos
        newData = true;
    }
  }
    if (newData)
    {
      float flat, flon, bike_speed;
      String hours, minutes, seconds, time2;
  
      if(gps.location.isValid())
      {
        flat = gps.location.lat();
        flon = gps.location.lng();
        bike_speed = gps.speed.kmph();
        
        hours = (String)gps.time.hour();
        minutes = (String)gps.time.minute();
        seconds = (String)gps.time.second();
        time2 = (hours + ":" + minutes + ":" + seconds);

        gps_data[0] = flat * 1000000;
        gps_data[1] = flon * 1000000;
        gps_data[2] = bike_speed;
        gps_data[3] = time2;

        return(gps_data);
      }
    }
  }
}

float get_distance()
{
  analogReadResolution(12);
  int analog_voltage = 0;
  for(int i = 0; i < 5; i++)
  {
    analog_voltage += analogRead(A1);

  }
  analog_voltage = analog_voltage / 5;
  float distance = ((analog_voltage * 3300) / 4096) / 3.2;
  return(distance); //Retorna a distância em centimetros. 
}

void receiveEvent(int howMany){
  SlaveReceived = Wire.read();
  //LOG("data: ");
  //LOG(SlaveReceived);
  //LOG("\n");
  get_data = true;
}