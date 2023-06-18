// https://www.makerhero.com/blog/fazer-um-anemometro-monitorado-internet/

#define BLYNK_TEMPLATE_ID "TMPLciii8hw7"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "TjrhjLA6j4QbHk-PKt3gdjPbdk5La02a"

// Comment this out to disable prints and save space
//#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#include <ESP8266WebServer.h>
//#include <SimpleTimer.h>

#include <Adafruit_AHT10.h>
#include <BH1750FVI.h>

#include "HX711.h"
#include "math.h"

#include "Adafruit_Sensor.h"
//#include "Adafruit_AM2320.h"
#include <AM2320_asukiaaa.h>

//SimpleTimer scheduler;
ESP8266WebServer server;

char auth[] = BLYNK_AUTH_TOKEN;
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "TP-LINK_8DA4";
char pass[] = "89820117";
//char ssid[] = "JUTUL";
//char pass[] = "loki5071";

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 12;
const int LOADCELL_SCK_PIN = 13;
const int cal_factor = 24311/50;

String data = "";

BlynkTimer timer;

HX711 scale;

//Adafruit_AM2320 am2320 = Adafruit_AM2320();
AM2320_asukiaaa am2320;

// Create the Lightsensor instance
BH1750FVI LightSensor(BH1750FVI::k_DevModeContLowRes);

// Create the temp/humid instance
Adafruit_AHT10 aht;

void setup() {

  Wire.begin();
  
  Blynk.begin(auth, ssid, pass);  
  //Serial.println("Adafruit AHT10 demo!");

  Serial.begin(115200);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(cal_factor);
  scale.tare();

  LightSensor.begin(); 
  //am2320.begin();
  am2320.setWire(&Wire);

  if (! aht.begin()) {
    Serial.println("Could not find AHT10? Check wiring");
    while (1) delay(10);
  }
  Serial.println("AHT10 found");

  // Setup a function to be called every second
  timer.setInterval(5000L, update_data);  

  //root route
  server.on("/",[](){
    server.send(200,"text/plain", data);
  });
  // start server
  server.begin();

  Serial.println(WiFi.localIP());

  //scale.power_down();
  
}

void loop() {
  //Serial.println("----");

  //uint16_t lux = LightSensor.GetLightIntensity();
  //Serial.print("Light: ");
  //Serial.println(lux);
  
  //Serial.println("----");

  //sensors_event_t humidity, temp;
  //aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
  //Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
  //Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");

  //delay(500);

  Blynk.run();
  timer.run();

  server.handleClient();
}

float FazLeituraTensao(void){
  int ValorADC;
  float TensaoMedida;

  ValorADC = analogRead(0); 

  //   (TensaoMedida-0) / (3.3-0)  =  (ValorADC - 0) / (1024 - 0)
  //      Logo:
  //      TensaoMedida = (3.3/1024)*ValorADC

  TensaoMedida = (3.3/1024.0)*ValorADC;

  //Devido ao divisor de tensao, a tensao real corresponde ao dobro da calculada
  TensaoMedida = TensaoMedida*2;
  
  //Serial.print("[Tensao medida] ");
  //Serial.print(TensaoMedida);
  //Serial.println("V");

  return TensaoMedida;
}


void update_data(){
  //scale.power_up();
  float massa = 0.0;

  float TensaoMedida;
  float VelVentoMedida;
  am2320.update();

  if (scale.is_ready()) {
    long reading = scale.read();
    massa = scale.get_units(10);
    massa = massa * 10;
    massa = round(massa);
    massa = massa / 10;
    //Serial.print("Massa (g): ");
    //Serial.println(reading);
    //Serial.println(m);
  } else {
    massa = 0.0;
    Serial.println("Erro na balança!!");
  }

  //float massa0 = scale.get_units(20);
  //float massa1 = scale.get_units(20);
  //float massa = (massa0 + massa1)/2
  uint16_t lux = LightSensor.GetLightIntensity();

  float temp2 = am2320.temperatureC;
  float humi2 = am2320.humidity;

  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  TensaoMedida = FazLeituraTensao();
  VelVentoMedida = 0.926*TensaoMedida;

  Blynk.virtualWrite(V2, millis() / 1000);
  Blynk.virtualWrite(V4, temp.temperature);
  Blynk.virtualWrite(V5, humidity.relative_humidity);  
  Blynk.virtualWrite(V6, lux);
  Blynk.virtualWrite(V7, massa);
  Blynk.virtualWrite(V8, temp2);
  Blynk.virtualWrite(V9, humi2);
  Blynk.virtualWrite(V10, VelVentoMedida);
  Serial.println(WiFi.localIP());

  data = "";
  data += "T";
  data += temp.temperature;
  data += "_";
  data += "U";
  data += humidity.relative_humidity;
  data += "_";

  data += "T2";
  data += temp2;
  data += "_";
  data += "U2";
  data += humi2;

  data += "_";
  data += "L";
  data += lux;
  data += "_";
  data += "M";
  data += massa;

  data += "_";
  data += "V";
  data += VelVentoMedida;

  //scale.power_down();
}
