#include <modules.h>  //Contém definições de variáveis globais, e inclusão de outras bibliotecas.
/* Código fonte do projeto das bikes.
   Todas as bibliotecas usadas estão no diretório /lib do projeto afim de manter a compatibilidade com o código, mesmo se compilado em outros computadores.
   Utiliza arduino MKR 1400 na IDE Vs Code e PlatformIO. 
*/

void setup() {
  pinMode(radar_pin, INPUT);
  pinMode(ultrasonic_pin, INPUT);
  Serial.begin(9600); //USB
  SerialGPS.begin(9600); //TEST GPS.
  //delay(1000);
  gsm_connect_func();  //Connect to GPRS
}

void loop()
{//CHECAR O TEMPO DESDE A ULTIMA VEZ QUE LI OS SENSORES
  float car_distance = 0;
  float car_speed = 0;
  int send;

  String * received_gps;

  if(millis() - mqtt_time_send > 2000) //Se não houve nenhum evento de carros em 15s envia a posição do gps com velocidade e distancia 0
  {
    received_gps = get_gps_data();
    car_speed = 0;
    car_distance = 0;
    send = send_data(car_speed, car_distance, received_gps);
    if(send == 1)
    {
      mqtt_time_send = millis();
      delay(100); //Trocar para millis?
    }
    else
    {
      mqtt_reconect();
    }
  }

  if(millis() - check_time > 100)
  {
    check_time = millis();
    car_speed = get_cars_speed();
    if(car_speed > 5)
    {
      car_distance = get_distance();
      car_speed = get_cars_speed();
      if(car_distance < 100)
      {
        received_gps = get_gps_data();
        Serial.print("Send: ");
        send = send_data(car_speed, car_distance, received_gps);
        delay(100);

        if(send == 1)
        {
          mqtt_time_send = millis();
        }
        else
        {
          mqtt_reconect();
        }

        Serial.println(send);
        Serial.print("IP: ");
        Serial.println(gprs.getIPAddress());
      }
    }
  }
/*  Ao invés de manter a conexão desestabiliza, perdendo a conexão. Vai entender.
  if(!mqttclient.loop())
  {
    Serial.println("Chamando pra reconectar");
    mqtt_reconect();
  }*/
}


int send_data(float speed, float distance, String *gpss)
{
//  Serial.print("speed");
//  Serial.println(speed);
//  Serial.print("distnace");
//  Serial.println(distance);
  mqtt_json_data["Distância"] = distance;
  mqtt_json_data["Car_speed"] = speed;

  for(int i = 0; i < 4; i++)
  {
    gps_json_data.add(gpss[i]);
 //   Serial.println(gpss[i]);
  }

  char data[512];
  serializeJson(mqtt_json_data, data);
  int send_state = mqttclient.publish("/ctu/bike1", data);
  Serial.print("MQTT STATE: ");
  Serial.println(mqttclient.state());
//  if(send_state == 0){gsm_connect_func();}
  return(send_state);
}


float get_cars_speed()
{
  noInterrupts();
  pulseIn(radar_pin, HIGH, 100000);
  unsigned int pulse_length = 0;
  for (x = 0; x < AVERAGE; x++)
  {
    pulse_length = pulseIn(radar_pin, HIGH); 
    pulse_length += pulseIn(radar_pin, LOW);    
    samples[x] =  pulse_length;
  }
  interrupts();

  // Verfica se os dados são válidos.
  bool samples_ok = true;
  unsigned int nbPulsesTime = samples[0];
  for (x = 1; x < AVERAGE; x++)
  {
    nbPulsesTime += samples[x];
    if ((samples[x] > samples[0] * 2) || (samples[x] < samples[0] / 2))
    {
      samples_ok = false;
    }

    if (samples_ok)
    {
      unsigned int Ttime = nbPulsesTime / AVERAGE;
      unsigned int Freq = 1000000 / Ttime;
      float speed_read = Freq/doppler_div;
      return(speed_read);
    }
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
      float flat, flon;
      unsigned long age;
      gps.f_get_position(&flat, &flon, &age);
      bike_speed = gps.f_speed_kmph();
      flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat;
      flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon;
      int satelite_number = gps.satellites();
      satelite_number == TinyGPS::GPS_INVALID_SATELLITES ? 0 : satelite_number;
      //Serial.print(gps.hdop() == TinyGPS::GPS_INVALID_HDOP ? 0 : gps.hdop());

      char convert_flat[16];
      sprintf(convert_flat,"%f", flat);

      gps_data[0] = flat;
      gps_data[1] = flon;
      gps_data[2] = satelite_number;
      gps_data[3] = bike_speed;

      return(gps_data);
    }
  }
}

float get_distance()
{
  noInterrupts(); //DEsabilita interrupções para não afetar a leitura da duração do pulso.
  ultrasonic_pulse = pulseIn(ultrasonic_pin, HIGH);  
  //ultrasonic_pulse += pulseIn(ultrasonic_pin, LOW);    
  interrupts();
  //Serial.print("pulse ");
  //Serial.println(ultrasonic_pulse);
  return(ultrasonic_pulse/(58.0)); //Retorna a distância em centimetros. 

}

void gsm_connect_func(){

  //Configurações para o GPRS da tim.
  const char pin[] = "";
  const char apn[] = "timbrasil.br";
  const char login[] = "";
  const char pwd[] = "tim";

  bool connected = false;
  
  while (!connected) 
  {
    if ((gsmAccess.begin(pin) == GSM_READY) && (gprs.attachGPRS(apn, login, pwd) == GPRS_READY)) 
    {
      mqttclient.setServer(mqttserver, 1883);
      if (mqttclient.connect("arduinoclient", mqttUser, mqttPassword )) {
        connected = true;
      }
      else{
        Serial.println("Falha ao conectar no broker!!");
        Serial.println(mqttclient.state());
      }
    } 
    else 
    {
      Serial.println("Falha na conexão GPRS!!!");
      delay(1000);
    }
  }
}

void mqtt_reconect()
{
  Serial.println("Reconectando");
  if(gprs.status() == GPRS_READY)
  {
    mqttclient.connect("arduinoclient", mqttUser, mqttPassword);
    delay(10);
  }
  else
  {
    gsm_connect_func();
  }
  
}