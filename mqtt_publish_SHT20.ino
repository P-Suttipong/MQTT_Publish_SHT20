#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EasyScheduler.h>
#include <Wire.h>
#include "DFRobot_SHT20.h"


#define WIFI_STA_NAME "servesook-broker"
#define WIFI_STA_PASS  "#servesook"
#define MQTT_SERVER   "192.168.1.177"
#define MQTT_PORT     1883
#define MQTT_USERNAME "servesook"
#define MQTT_PASSWORD "servesook"
#define MQTT_NAME     "esp8266"
#define POSITION  "IN"
char msg[50];

WiFiClient client;
PubSubClient mqtt(client);
DFRobot_SHT20    sht20;

int num = 0;
float temp = 0.0;
float humd = 0.0;


Schedular ReadDataTask;
Schedular PublishTask;


// __________________CALLBACK_FUNCTION_FOR_MQTT________________
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
}

// ________________PUBLISH_TO_MQTT_BROKER______________________
void publish_mqttt() {
  if (mqtt.connect(MQTT_NAME, MQTT_USERNAME, MQTT_PASSWORD)) {
    snprintf (msg, 75, "ENVI,%s,%.2f,%.2f", POSITION,temp,humd);
    Serial.print("Publish message: ");
    Serial.println(msg);
    if (mqtt.publish("node/envi", msg) == true) {
      Serial.println("Success sending");
    } else {
      Serial.println("Fail sending");
    }
  }
}

// ________________PUBLISH_FOR_INIT_DATA____________________
void publish_init() {
  if (mqtt.connect(MQTT_NAME, MQTT_USERNAME, MQTT_PASSWORD)) {
    snprintf (msg, 75, "INITENVI");
    Serial.print("Publish message: ");
    Serial.println(msg);
    if (mqtt.publish("node/envi", msg) == true) {
      Serial.println("Success sending");
    } else {
      Serial.println("Fail sending");
    }
  }
}


// _______________READ_DATA_SHT20_______________________
void read_data() {
  humd = sht20.readHumidity();                  // Read Humidity
  temp = sht20.readTemperature();               // Read Temperature
  Serial.print("Time:");
  Serial.print(millis());
  Serial.print(" Temperature:");
  Serial.print(temp, 1);
  Serial.print("C");
  Serial.print(" Humidity:");
  Serial.print(humd, 1);
  Serial.print("%");
  Serial.println();
}

// _____________________SETUP______________________
void setup() {
  Serial.begin(115200);

  //  Initial SHT20
  sht20.initSHT20();                                  // Init SHT20 Sensor
  delay(100);
  sht20.checkSHT20();                                 // Check SHT20 Sensor

  // Initial Wifi
  WiFi.mode(WIFI_STA);
  while (!Serial) ;
  delay(250);
  Serial.println("");
  Serial.println(WIFI_STA_NAME);
  Serial.println("WIFI Connecting");
  WiFi.begin(WIFI_STA_NAME, WIFI_STA_PASS); //เชื่อมต่อ wifi
  while (num < 40) {
    delay(500);
    Serial.print(".");
    num++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("\n WiFi Connected. \n");
  } else {
    Serial.print("\n WIFI Connect fail. ");
  }

  //  Initial MQTT SERVER
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(callback);

  publish_init();

  ReadDataTask.start();
  PublishTask.start();
}


// ______________________LOOP______________________
void loop() {
  ReadDataTask.check(read_data, 50000);
  PublishTask.check(publish_mqttt, 60000); 
}
