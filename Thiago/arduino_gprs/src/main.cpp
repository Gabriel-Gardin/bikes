#include <modules.h>  //Contém definições de variáveis globais, e inclusão de outras bibliotecas.
/* Código fonte do projeto das bikes.
   Todas as bibliotecas usadas estão no diretório /lib do projeto afim de manter a compatibilidade com o código, mesmo se compilado em outros computadores.
   Utiliza arduino MKR 1400 na IDE Vs Code e PlatformIO. 
*/
#define DEBUG true

#if DEBUG == true
#define LOG(X) Serial.print(X);
#else
#define LOG(X)
#endif


void setup() {
  pinMode(ultrasonic_pin, INPUT);
  Wire.begin(8);                          //Begins I2C communication with Slave Address as 8 at pin (A4,A5)
  Wire.onReceive(receiveEvent); 
  #if DEBUG == true  
    Serial.begin(9600); //Comentar
  #endif
  SerialGPS.begin(9600); //TEST GPS.
  gsm_connect_func();  //Connect to GPRS
}

void loop()
{
  //CHECAR O TEMPO DESDE A ULTIMA VEZ QUE LI OS SENSORES
  String * received_gps;

  float car_distance;
  uint8_t car_speed;
  int send;

  if(millis() - mqtt_time_send > 3000) //Se não houve nenhum evento de carros em 3s envia a posição do gps com velocidade e distancia 0. Na prática leva em torno de 11s devido as outras funções. 
  {
    received_gps = get_gps_data();
    car_speed = 0;
    car_distance = 0;
    send = send_data(car_speed, car_distance, received_gps);
    delay(100); //Trocar para millis?
    if(send == 1)
    {
      mqtt_time_send = millis();
    }
    else
    {
      mqtt_reconect();
    }
  }
  else if(get_data)
  {
    car_speed = SlaveReceived;
    car_distance = get_distance();
    LOG("Distancia:");
    LOG(car_distance);
    LOG("\n")
    received_gps = get_gps_data();
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

    //Serial.println(send);
    //Serial.print("IP: ");
    //Serial.println(gprs.getIPAddress());
    get_data = false;
  }
  
/*  Ao invés de manter a conexão desestabiliza, perdendo a conexão. Vai entender.
  if(!mqttclient.loop())
  {
    Serial.println("Chamando pra reconectar");
    mqtt_reconect();
  }*/
}


int send_data(double speed, float distance, String *gpss)
{
  //Define um objeto utilizando a lib json para enviar os dados via mqtt. 165 é a RAM separada para alocar os dados.
  DynamicJsonDocument mqtt_json_data(165);

  mqtt_json_data.clear(); //Limpa o payload do json toda vez. 
  mqtt_json_data["Distância"] = distance;
  mqtt_json_data["Car_speed"] = speed;
  
  //Add um array ao mqtt_data. Dados do gps
  JsonArray gps_json_data = mqtt_json_data.createNestedArray("GPS_DATA"); 

  for(int i = 0; i < 4; i++)
  {
    gps_json_data.add(gpss[i]);
    //Serial.println(gpss[i]);
  }

  char data[512];
  serializeJson(mqtt_json_data, data);
  int send_state = mqttclient.publish("/ctu/bike1", data);
  LOG("MQTT STATE: ");
  LOG(mqttclient.state());
  LOG("\n")
//  if(send_state == 0){gsm_connect_func();}
  return(send_state);
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
        LOG("COnectado");
        LOG("\n");
        connected = true;
      }
      else{
        LOG("Falha ao conectar no broker!!");
        LOG("\n");
        LOG(mqttclient.state());
        LOG("\n");
      }
    } 
    else 
    {
      LOG("Falha na conexão GPRS!!!");
      LOG("\n");
      delay(1000);
    }
  }
}

void mqtt_reconect()
{
  LOG("Reconectando");
  LOG("\n");
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

void receiveEvent(int howMany){
  SlaveReceived = Wire.read();
  //LOG("data: ");
  //LOG(SlaveReceived);
  //LOG("\n");
  get_data = true;
}