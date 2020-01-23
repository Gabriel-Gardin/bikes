#include <modules.h>  


#define DEBUG true

#if DEBUG == true
#define LOG(X) Serial.print(X);
#else
#define LOG(X)
#endif


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
  
  File dataFile = SD.open("DATA.TXT", FILE_WRITE);
  
  if (dataFile)
  {
    String data = "tempo_on,velocidade,distancia,latitude,longitude,bike_speed,satelite";
    dataFile.println(data);
    dataFile.close();
  }

  Wire.onReceive(receiveEvent);

  #if DEBUG == true  
    Serial.begin(9600); //Comentar
  #endif
  
  SerialGPS.begin(9600); //TEST GPS.

}

void loop()
{
  String * received_gps;

  float car_distance;
  uint8_t car_speed;
  int saved;

  if(millis() - saved_time > 3000) //Caso não tenha enviados dados em 10s atualiza a posição do gps.
  {
    received_gps = get_gps_data();
    car_speed = 0;
    car_distance = 0;
    on_time = millis();
    saved = save_data(on_time, car_speed, car_distance, received_gps);
    delay(100); //Trocar para millis?
    if(saved == 1)
    {
      saved_time = millis();
    }
    else
    {
      LOG("Erro ao salvar os dados\n");
    }
  }

  else if(get_data)
  {
    car_speed = SlaveReceived;
    car_distance = get_distance();
    on_time = millis();
    LOG("Distancia:");
    LOG(car_distance);
    LOG("\n")
    received_gps = get_gps_data();
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

    get_data = false;
  }
}


int save_data(unsigned long on_time, double speed, float distance, String *gpss)
{
  String on_t = (String) on_time;
  String str_speed = (String) speed;
  String str_distance = (String) distance;
  char comma = ',';

  String data = on_t + comma + str_speed + comma +str_distance + comma + gpss[0] + comma + gpss[1] + comma + gpss[2] + comma + gpss[3];

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
      float flat, flon, satelite_number, bike_speed;

      if(gps.location.isValid())
      {
        flat = gps.location.lat();
        flon = gps.location.lng();
        bike_speed = gps.speed.kmph();
        satelite_number = gps.satellites.value();

        gps_data[0] = flat * 1000000;
        gps_data[1] = flon * 1000000;
        gps_data[2] = bike_speed;
        gps_data[3] = satelite_number;

        return(gps_data);
      }
    }
  }
}

float get_distance()
{
  noInterrupts(); //DEsabilita interrupções para não afetar a leitura da duração do pulso.
  ultrasonic_pulse = pulseIn(ultrasonic_pin, HIGH);  
  //ultrasonic_pulse += pulseIn(ultrasonic_pin, LOW);    
  interrupts();
  return(ultrasonic_pulse/(58.0)); //Retorna a distância em centimetros. 
}

void receiveEvent(int howMany){
  SlaveReceived = Wire.read();
  //LOG("data: ");
  //LOG(SlaveReceived);
  //LOG("\n");
  get_data = true;
}