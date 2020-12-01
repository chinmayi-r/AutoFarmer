#include <ThingerESP32.h>
#include "DHTesp.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>


DHTesp dht;

#define DHT11_PIN 4

#define USERNAME "Mockingjay"
#define DEVICE_ID "ESP32"
#define DEVICE_CREDENTIAL "ESP_32"

#define SSID "Redmi"
#define SSID_PASSWORD "31415926"

ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

const int sensor_pin = 34;
float moisture_percentage = 0;
float humidity = 0;
float temperature = 0;
int ssm = 0;
float moisture = 0.0;
int httpCode=0;

int MOTOR = 2;
int THINGSWITCH = 4;
int httpCounter = 0;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  delay(8000);

  thing.add_wifi(SSID, SSID_PASSWORD);
  pinMode(THINGSWITCH, INPUT);
  pinMode(THINGSWITCH, OUTPUT);

  // digital pin control example (i.e. turning on/off a light, a relay, configuring a parameter, etc)
  thing["led"] << digitalPin(THINGSWITCH);
  

  // resource output example (i.e. reading a sensor value)
  thing["millis"] >> outputValue(millis());

   dht.setup(5, DHTesp::DHT11); // Connect DHT11 sensor to GPIO 05 which is D1

  ////////Setup LED in place of motor start/////////
  pinMode(MOTOR, OUTPUT);
  //pinMode(THINGSWITCH, INPUT); 
  digitalWrite(THINGSWITCH, LOW);

}

void loop() {

 /////////////////////////////////REading moisture data from Soil moisture sensor//////////////////////////
  moisture_percentage = (analogRead(sensor_pin)/1023.00);

  Serial.print("Soil Moisture(in Percentage) = ");
  Serial.print((moisture_percentage*100));
  Serial.println("%");
  thing["moisture"] >> outputValue( moisture_percentage*100);
  thing["moistureindex"] >> outputValue(moisture_percentage);

  /////////////////////////////Reading temperature and humidity from DTH11/////////////////////////////

  delay(dht.getMinimumSamplingPeriod());

  humidity = dht.getHumidity();
  temperature = dht.getTemperature();

  Serial.print("\t");
  Serial.print(dht.getStatusString());
  Serial.print("\t");
  Serial.print(humidity, 1);
  Serial.print("\t\t");
  Serial.print(temperature, 1);
  Serial.print("\t\t");
  Serial.print(dht.toFahrenheit(temperature), 1);
  Serial.print("\t\t");
  Serial.print(dht.computeHeatIndex(temperature, humidity, false), 1);
  Serial.print("\t\t");
  Serial.println(dht.computeHeatIndex(dht.toFahrenheit(temperature), humidity, true), 1);
  
  thing["temperature"] >> outputValue(temperature);
  thing["humidity"] >> outputValue(humidity);
  thing["heatindex"] >> outputValue(dht.computeHeatIndex(dht.toFahrenheit(temperature), humidity, true));

//////////////////////////Satetellite soil moisture////////////
//////////////////////////////Reading Agrimonitor API for satellite moisture///////////////////////////

  HTTPClient http; //Object of class HTTPClient
  http.begin("http://api.agromonitoring.com/agro/1.0/soil?polyid=5fb8d346714b524cb6e1c224&appid=58b294b9ec076fc968b1090696c68383");
  //http.begin("http://jsonplaceholder.typicode.com/users/1");
 
  int httpCode = http.GET();

  if (httpCode > 0) 
  {
      const size_t bufferSize = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + 370;
      DynamicJsonDocument jsonBuffer(bufferSize);
      //DynamicJsonBuffer jsonBuffer(bufferSize);
      deserializeJson(jsonBuffer, http.getString());
      JsonObject root = jsonBuffer.as<JsonObject>();
      //JsonObject root = jsonBuffer.parseObject(http.getString());
 
      moisture = root["moisture"]; 

      Serial.print("Moisture:");
      Serial.println(moisture);
      thing["satellite_moisture"] >> outputValue(moisture);
  }

  http.end(); //Close connection

  Serial.println(digitalRead(THINGSWITCH));
  if(digitalRead(THINGSWITCH) == 1)
  {
    Serial.println("Motor switched on by Thinger");
    delay(1000);
  }
  else
  {
    Serial.println("Motor NOT switched on by Thinger");
    delay(1000);
    
  }
  thing["motorstatus"] >> outputValue(digitalRead(THINGSWITCH));
  if( digitalRead(THINGSWITCH) == 1)
  {
    Serial.println("Keeping motor on for 10min");
    digitalWrite(MOTOR, HIGH); // turn the LED on
    thing["motorstatus"] >> outputValue(1);
    delay(10000);
  }
  if(moisture < 0.1) //blink LED later start motor
    {
      Serial.println("Moisture level low. Switching motor on");
      digitalWrite(MOTOR, HIGH); // turn the LED on
      thing["motorstatus"] >> outputValue(1);
    }
    else
    {
      Serial.println("Moisture level high. Switching motor off");
      digitalWrite(MOTOR, LOW); // turn the LED off
      thing["motorstatus"] >> outputValue(0);
    }
 

  delay(2000);
  thing.handle();
}
