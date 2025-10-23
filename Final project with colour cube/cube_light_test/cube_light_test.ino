// This project aims to show the combination of the three traditional primary colours
// in a colourful light way.
// There will be four sides of a cube been used,including yellow, blue, red and white sides.
// V1.2 to test for the red and the yellow sides.

#include <SPI.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include "arduino_secrets.h"
#include <utility/wifi_drv.h>  

// Passwords
const char* ssid          = SECRET_SSID;
const char* password      = SECRET_PASS;
const char* mqtt_username = SECRET_MQTTUSER;
const char* mqtt_password = SECRET_MQTTPASS;

// MQTT broker
const char* mqtt_server   = "mqtt.cetools.org";
const int   mqtt_port     = 1884;

// Create wifi object and mqtt object
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Define the MQTT topic
String lightId = "31"; 
String mqtt_topic = "student/CASA0014/luminaire/" + lightId;
String clientId = ""; 

// Define the red and the yellow pins
const int PIN_R = A2;  
const int PIN_Y = A3;

// NeoPixel Configuration - we need to know this to know how to send messages 
// to vespera 
const int num_leds = 72;
const int payload_size = num_leds * 3;

// Create the byte array to send in MQTT payload this stores all the colours 
// in memory so that they can be accessed in for example the rainbow function
byte RGBpayload[payload_size];

void callback(char* topic, byte* payload, unsigned int length);
void startWifi();
void reconnectMQTT();

void LedRed();
void LedYellow();
void LedOrange();
void LedOff();

void send_all_off();
void send_all_red();
void send_all_yellow();
void send_all_orange();

void setup() {
  Serial.begin(115200);
  delay(60);
  Serial.println("Touch Sensor Luminaire Controller (Red + Yellow)");

  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  printMacAddress(mac);
  clientId = "MKR1010-" + String(mac[5], HEX) + String(mac[4], HEX);

  Serial.print("This device is Luminaire ");
  Serial.println(lightId);

  pinMode(PIN_R, INPUT);
  pinMode(PIN_Y, INPUT);

  startWifi();
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setBufferSize(2200);
  mqttClient.setCallback(callback);
  reconnectMQTT();

  send_all_off();
  LedOff();

  Serial.println("Set-up complete - Touch A2=RED, A3=YELLOW");
}

void loop() {
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  if (WiFi.status() != WL_CONNECTED){
    startWifi();
  }
  mqttClient.loop();

  static unsigned long tDeb = 0;
  if (millis() - tDeb < 50) return;
  tDeb = millis();

  bool rNow = (digitalRead(PIN_R) == HIGH);
  bool yNow = (digitalRead(PIN_Y) == HIGH);

  static bool rPrev = false;
  static bool yPrev = false;

  if (rNow != rPrev || yNow != yPrev) {
    rPrev = rNow;
    yPrev = yNow;

    if (rNow && yNow) {
      Serial.println("Both touched - ORANGE ON");
      send_all_orange();
      LedOrange();
    } else if (rNow && !yNow) {
      Serial.println("Red touched - RED ON");
      send_all_red();
      LedRed();
    } else if (!rNow && yNow) {
      Serial.println("Yellow touched - YELLOW ON");
      send_all_yellow();
      LedYellow();
    } else {
      Serial.println("No touch - OFF");
      send_all_off();
      LedOff();
    }
  }
}


void send_all_off() {
  if (mqttClient.connected()) {
    for (int pixel = 0; pixel < num_leds; pixel++) {
      RGBpayload[pixel * 3 + 0] = (byte)0;   
      RGBpayload[pixel * 3 + 1] = (byte)0;   
      RGBpayload[pixel * 3 + 2] = (byte)0;   
    }
    mqttClient.publish(mqtt_topic.c_str(), RGBpayload, payload_size);
    Serial.println("Published all OFF.");
  } else {
    Serial.println("MQTT mqttClient not connected, cannot publish from *send_all_off*.");
  }
}

void send_all_red() {
  if (mqttClient.connected()) {
    for (int pixel = 0; pixel < num_leds; pixel++) {
      RGBpayload[pixel * 3 + 0] = (byte)255; 
      RGBpayload[pixel * 3 + 1] = (byte)0;   
      RGBpayload[pixel * 3 + 2] = (byte)0;   
    }
    mqttClient.publish(mqtt_topic.c_str(), RGBpayload, payload_size);
    Serial.println("Published RED to all pixels.");
  } else {
    Serial.println("MQTT mqttClient not connected, cannot publish from *send_all_red*.");
  }
}

void send_all_yellow() {
  if (mqttClient.connected()) {
    for (int pixel = 0; pixel < num_leds; pixel++) {
      RGBpayload[pixel * 3 + 0] = (byte)255; 
      RGBpayload[pixel * 3 + 1] = (byte)255; 
      RGBpayload[pixel * 3 + 2] = (byte)0;   
    }
    mqttClient.publish(mqtt_topic.c_str(), RGBpayload, payload_size);
    Serial.println("Published YELLOW to all pixels.");
  } else {
    Serial.println("MQTT mqttClient not connected, cannot publish from *send_all_yellow*.");
  }
}

void send_all_orange() {
  if (mqttClient.connected()) {
    for (int pixel = 0; pixel < num_leds; pixel++) {
      RGBpayload[pixel * 3 + 0] = (byte)255; 
      RGBpayload[pixel * 3 + 1] = (byte)128; 
      RGBpayload[pixel * 3 + 2] = (byte)0;   
    }
    mqttClient.publish(mqtt_topic.c_str(), RGBpayload, payload_size);
    Serial.println("Published ORANGE to all pixels.");
  } else {
    Serial.println("MQTT mqttClient not connected, cannot publish from *send_all_orange*.");
  }
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) Serial.print("0");
    Serial.print(mac[i], HEX);
    if (i > 0) Serial.print(":");
  }
  Serial.println();
}