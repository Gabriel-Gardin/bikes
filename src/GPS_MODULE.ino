#include <modules.h>

void setup() {
  pinMode(radar_pin, INPUT);
  pinMode(ultrasonic_pin, INPUT);
  Serial.begin(9600); //USB
  SerialGPS.begin(9600); //TEST GPS.
  //gsm_connect();  //Connect to GPRS

  while(!Serial) { ; } //WAIT FOR SERIAL USB.
  Serial.print("\nTEST SOMETHING\n\n");
 
}

void loop()
{
  String * received_gps;

  float dist = get_distance();
  Serial.print("Distancia ");
  Serial.println(dist);
  Serial.println();
  delay(500);

  Serial.print("Velocidade: ");
  Serial.print(get_cars_speed());
  Serial.println("km/h");
  Serial.println();
  delay(500);

  received_gps = gps_data();
  for(int i = 0; i < 4; i++){
    Serial.print(" ");
    Serial.print(received_gps[i]);
    delay(100);
  }
  Serial.println();

}

float get_cars_speed()
{
  noInterrupts();
  pulseIn(radar_pin, HIGH);
  unsigned int pulse_length = 0;
  for (x = 0; x < AVERAGE; x++)
  {
    pulse_length = pulseIn(radar_pin, HIGH); 
    pulse_length += pulseIn(radar_pin, LOW);    
    samples[x] =  pulse_length;
  }
  interrupts();

  // Check for consistency
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
      //Serial.prit(Ttime);
      //Serial.print("\r\n");
      //Serial.print(Freq);
      //Serial.print("Hz : ");
      //Serial.print(Freq/doppler_div);
      //Serial.print("km/h\r\n");
      float speed_read = Freq/doppler_div;
      return(speed_read);
    }
    bool newData = false;
    unsigned long chars;
  }
}

String * gps_data()
{
  static String gps_data[4];

  bool newData = false;
  unsigned long chars;
  // For one second we parse GPS data and report some key values
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
      flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6;
      flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6;
      int satelite_number = gps.satellites();
      satelite_number == TinyGPS::GPS_INVALID_SATELLITES ? 0 : satelite_number;
      //Serial.print(gps.hdop() == TinyGPS::GPS_INVALID_HDOP ? 0 : gps.hdop());

      gps_data[0] = flat;
      gps_data[1] = flon;
      gps_data[2] = satelite_number;
      gps_data[3] = bike_speed;

//      Serial.print("BIKE SPEEDDDD = ");
//      Serial.println(bike_speed);
//      delay(1000);
      return(gps_data);
    }
  }
}

float get_distance()
{
  noInterrupts();
  ultrasonic_pulse = pulseIn(ultrasonic_pin, HIGH);  
  //ultrasonic_pulse += pulseIn(ultrasonic_pin, LOW);    
  interrupts();
  //Serial.print("pulse ");
  //Serial.println(ultrasonic_pulse);
  return(ultrasonic_pulse/(58.0)); //Retorna a distância em centimetros. 

}

void gsm_connect(){
  //Enter Your SIM Card details below
  char pin[] = "";
  char apn[] = "timbrasil.br";
  char login[] = "";
  char pwd[] = "tim";
  
  GSMClient client;
  GPRS gprs;
  GSM gsmAccess;
  
  boolean connected = false;
  
  while (!connected) 
  {
    if ((gsmAccess.begin(pin) == GSM_READY) && (gprs.attachGPRS(apn, login, pwd) == GPRS_READY)) 
    {
      connected = true;
    } 
    else 
    {
      Serial.println("Status: Could Not Connect to the Network!!!");
      delay(1000);
    }
  }
}