#include <AWS_IOT.h>
#include <WiFi.h>
#include <ArduinoJson.h>

// variables Wifi
const char* ssid     = "";
const char* password = "";

//AWS
AWS_IOT hornbill;
char HOST_ADDRESS[]="";
char CLIENT_ID[]= "";
char TOPIC_NAME[]= "";
int status = WL_IDLE_STATUS;
int tick=0,msgCount=0,msgReceived = 0;
char payload[512];
char rcvdPayload[512];

//JSON
StaticJsonDocument<512> doc;

//LED
const int LEDPIN = 2;

// PINRelay
const int RELAYPIN1 = 23; // Sector 1
const int RELAYPIN2 = 22; // Sector 2
const int seconds = 60 * 1000;

void setup() {
    Serial.begin(115200);
    delay(2000);

    //LED
    pinMode(LEDPIN, OUTPUT);

    //RELAY
    pinMode(RELAYPIN1, OUTPUT);
    pinMode(RELAYPIN2, OUTPUT);

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

    // Conectividad AWS
    if(hornbill.connect(HOST_ADDRESS,CLIENT_ID)== 0) {
        Serial.println("AWS - Conectado.");
        delay(1000);
        if(0==hornbill.subscribe(TOPIC_NAME,mySubCallBackHandler)) {
            Serial.println("AWS - Subscripcion correcta.");
        } else {
            Serial.println("AWS - Subscripcion fallida.");
            while(1);
        }
    } else {
      Serial.println("AWS - Conexión Fallida, Validar direccion HOST.");
      while(1);
    }

    delay(2000);
}

void loop() {
    if(msgReceived == 1) {
        msgReceived = 0;
        Serial.print("Mensaje recibido:");
        Serial.println(rcvdPayload);
        deserializeJson(doc, rcvdPayload);
        if(doc["ID"] == 1) {
          Serial.println(1);
          digitalWrite(LEDPIN, HIGH);
          digitalWrite(RELAYPIN1, HIGH);
          delay(seconds);
          digitalWrite(LEDPIN, LOW);
          digitalWrite(RELAYPIN1, LOW);
        } else if (doc["ID"] == 2) {
          Serial.println(2);
          digitalWrite(LEDPIN, HIGH);
          digitalWrite(RELAYPIN2, HIGH);
          delay(seconds);
          digitalWrite(LEDPIN, LOW);
          digitalWrite(RELAYPIN2, LOW);
        }
        else {
          Serial.println("nada");
        }
    }
}

void mySubCallBackHandler (char *topicName, int payloadLen, char *payLoad) {
    strncpy(rcvdPayload,payLoad,payloadLen);
    rcvdPayload[payloadLen] = 0;
    msgReceived = 1;
}
