#include <WiFi.h>
#include <NTPClient.h>
// https://github.com/taranais/NTPClient/blob/master/NTPClient.cpp
#include <WiFiUdp.h>
#include <AWS_IOT.h>
#include "DHT.h"
#include <ArduinoJson.h>

//DHT
#define DHTPIN 15
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float humEnv,temEnv;

//Moisture senser
#define MOISTUREPIN A0
float moistureValue = 0;
const int dryValue = 2975;
const int wetValue = 1372;
int humRange = dryValue - wetValue;
float percentValue = 0;

//JSON
StaticJsonDocument<200> doc;

// NTP Client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

//AWS
AWS_IOT hornbill;
char HOST_ADDRESS[]="";
char CLIENT_ID[]= "";
char TOPIC_NAME[]= "";
int status = WL_IDLE_STATUS;
int tick=0,msgCount=0,msgReceived = 0;
char payload[512];

// variables Wifi
const char* ssid     = "";
const char* password = "";

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  // Conectividad Wifi
  Serial.print("Wifi - Conectando a la red: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
  }
  Serial.println("WiFi - conectado.");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Inicializar timeClient con GMT Santiago de Chile Verano
  timeClient.begin();
  // GMT - 3 Verano = -3*3600 = -1 = -10800
  // GMT - 4 Normal = -4*3600 = -1 = -14400
  timeClient.setTimeOffset(-10800);

  // Conectividad AWS
  if(hornbill.connect(HOST_ADDRESS,CLIENT_ID)== 0) {
        Serial.println("AWS - Conectado.");
        delay(1000);
    } else {
      Serial.println("AWS - Conexión Fallida, Validar direccion HOST.");
      while(1);
    }
    
    // Inicializar DHT
    dht.begin();

    delay(2000);
}


void loop() {
  moistureValue = analogRead(MOISTUREPIN);
  Serial.println(moistureValue);
  percentValue = 100 * (1 - (moistureValue - wetValue) / humRange );
  Serial.println(percentValue);
  
  // Obtener la hora, sin minutos.
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }

  // Obtener variables ambientales
  humEnv = dht.readHumidity();
  temEnv = dht.readTemperature();

  if (isnan(humEnv) || isnan(temEnv)) {
    Serial.println("DHT - Falla en la lectura del sensor.");
  } else {
    // Crear payload con datos a enviar
    //sprintf(payload,"{'ID':1,'hum':%.2f,'tem':%.2f,'hour':%S}",h,t,timeClient.getFormattedTime());
    // crear JSON a enviar
    doc["ID"] = 1;
    doc["hum"] = String(humEnv,2).toFloat();
    doc["tem"] = String(temEnv,2).toFloat();
    doc["mois"] = String(percentValue,2).toFloat();
    doc["time"] = timeClient.getEpochTime();
    doc["hour"] = timeClient.getHours();
    serializeJson(doc, payload);
    // Publicar payload 
    if(hornbill.publish(TOPIC_NAME,payload) == 0) {
      Serial.println("AWS - Mensaje enviado.");   
      Serial.println(payload);
      // Pausa en el envio de datos cada 15 minutos.
      vTaskDelay(900000 / portTICK_RATE_MS);
     } else {
      Serial.println("AWS - publicación fallida");
     }   
  }
}
