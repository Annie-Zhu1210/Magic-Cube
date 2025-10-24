// This project aims to show the combination of the three traditional primary colours
// in a colourful light way.
// There will be four sides of a cube been used,including yellow, blue, red and white sides.
// V1.3 to test for the new blue side.

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

// Define the red, yellow and blue pins
const int PIN_R = A2;  
const int PIN_Y = A3;
const int PIN_B = A4;

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
void LedBlue();
void LedGreen();
void LedPurple();
void LedOff();

void send_all_off();
void send_all_red();
void send_all_yellow();
void send_all_orange();
void send_all_blue();
void send_all_green();
void send_all_purple();

// Rainbow Effect
const uint8_t COLS[6][3] = {
  {255,   0,   0}, 
  {255, 128,   0}, 
  {255, 255,   0}, 
  {  0, 255,   0}, 
  {  0,   0, 255}, 
  {255,   0, 255}  
};
const int BLOCKS = 6;
const int BLOCK_SIZE = 12;         
unsigned long lastAnim = 0;
const unsigned long ANIM_INTERVAL_MS = 140;
int animOffset = 0;

inline void send_blocks_frame(int offset){
  if (!mqttClient.connected()) return;
  for(int b=0; b<BLOCKS; b++){
    const uint8_t* c = COLS[(b + offset) % BLOCKS];
    int start = b * BLOCK_SIZE;
    int end   = start + BLOCK_SIZE;
    for(int i=start; i<end; i++){
      RGBpayload[i*3 + 0] = c[0];
      RGBpayload[i*3 + 1] = c[1];
      RGBpayload[i*3 + 2] = c[2];
    }
  }
  mqttClient.publish(mqtt_topic.c_str(), RGBpayload, payload_size);
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) Serial.print("0");
    Serial.print(mac[i], HEX);
    if (i > 0) Serial.print(":");
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(60);
  Serial.println("Touch Sensor Luminaire Controller (R/Y/B + mixes)");

  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  printMacAddress(mac);
  clientId = "MKR1010-" + String(mac[5], HEX) + String(mac[4], HEX);

  Serial.print("This device is Luminaire ");
  Serial.println(lightId);

  pinMode(PIN_R, INPUT);
  pinMode(PIN_Y, INPUT);
  pinMode(PIN_B, INPUT);

  // WiFi + MQTT
  startWifi();
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setBufferSize(2200);
  mqttClient.setCallback(callback);
  reconnectMQTT();


  send_all_off();
  LedOff();

  Serial.println("Set-up complete - Touch A2=RED, A3=YELLOW, A4=BLUE");
}

void loop() {
  // Reconnect if necessary
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  if (WiFi.status() != WL_CONNECTED){
    startWifi();
  }
  mqttClient.loop();

  static unsigned long tDeb = 0;
  if (millis() - tDeb < 50) {
    // The rainbow effect
    bool rHeld = (digitalRead(PIN_R) == HIGH);
    bool yHeld = (digitalRead(PIN_Y) == HIGH);
    bool bHeld = (digitalRead(PIN_B) == HIGH);
    if (rHeld && yHeld && bHeld) {
      if (millis() - lastAnim >= ANIM_INTERVAL_MS) {
        lastAnim = millis();
        send_blocks_frame(animOffset);
        animOffset = (animOffset + 1) % BLOCKS;
      }
    }
    return;
  }
  tDeb = millis();

  bool rNow = (digitalRead(PIN_R) == HIGH);
  bool yNow = (digitalRead(PIN_Y) == HIGH);
  bool bNow = (digitalRead(PIN_B) == HIGH);

  static bool rPrev = false;
  static bool yPrev = false;
  static bool bPrev = false;

  if (rNow != rPrev || yNow != yPrev || bNow != bPrev) {
    rPrev = rNow;
    yPrev = yNow;
    bPrev = bNow;

    if (rNow && yNow && bNow) {
      Serial.println("R+Y+B touched - COLOURFUL EFFECT (blocks)");
      animOffset = 0;
      lastAnim = 0;
      send_blocks_frame(animOffset);
      animOffset = (animOffset + 1) % BLOCKS;
      LedPurple();
    } 
    else if (rNow && yNow && !bNow) {
      Serial.println("R+Y - ORANGE");
      send_all_orange();
      LedOrange();
    } 
    else if (rNow && !yNow && bNow) {
      Serial.println("R+B - PURPLE");
      send_all_purple();
      LedPurple();
    } 
    else if (!rNow && yNow && bNow) {
      Serial.println("Y+B - GREEN");
      send_all_green();
      LedGreen();
    } 
    else if (rNow && !yNow && !bNow) {
      Serial.println("RED only");
      send_all_red();
      LedRed();
    } 
    else if (!rNow && yNow && !bNow) {
      Serial.println("YELLOW only");
      send_all_yellow();
      LedYellow();
    } 
    else if (!rNow && !yNow && bNow) {
      Serial.println("BLUE only");
      send_all_blue();
      LedBlue();
    } 
    else {
      Serial.println("No touch - OFF");
      send_all_off();
      LedOff();
    }
  }

  if (rNow && yNow && bNow) {
    if (millis() - lastAnim >= ANIM_INTERVAL_MS) {
      lastAnim = millis();
      send_blocks_frame(animOffset);
      animOffset = (animOffset + 1) % BLOCKS;
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

void send_all_blue() {
  if (mqttClient.connected()) {
    for (int pixel = 0; pixel < num_leds; pixel++) {
      RGBpayload[pixel * 3 + 0] = (byte)0;
      RGBpayload[pixel * 3 + 1] = (byte)0;
      RGBpayload[pixel * 3 + 2] = (byte)255;
    }
    mqttClient.publish(mqtt_topic.c_str(), RGBpayload, payload_size);
    Serial.println("Published BLUE to all pixels.");
  } else {
    Serial.println("MQTT mqttClient not connected, cannot publish from *send_all_blue*.");
  }
}

void send_all_green() {
  if (mqttClient.connected()) {
    for (int pixel = 0; pixel < num_leds; pixel++) {
      RGBpayload[pixel * 3 + 0] = (byte)0;
      RGBpayload[pixel * 3 + 1] = (byte)255;
      RGBpayload[pixel * 3 + 2] = (byte)0; 
    }
    mqttClient.publish(mqtt_topic.c_str(), RGBpayload, payload_size);
    Serial.println("Published GREEN to all pixels.");
  } else {
    Serial.println("MQTT mqttClient not connected, cannot publish from *send_all_green*.");
  }
}

void send_all_purple() {
  if (mqttClient.connected()) {
    for (int pixel = 0; pixel < num_leds; pixel++) {
      RGBpayload[pixel * 3 + 0] = (byte)255;
      RGBpayload[pixel * 3 + 1] = (byte)0;
      RGBpayload[pixel * 3 + 2] = (byte)255;
    }
    mqttClient.publish(mqtt_topic.c_str(), RGBpayload, payload_size);
    Serial.println("Published PURPLE to all pixels.");
  } else {
    Serial.println("MQTT mqttClient not connected, cannot publish from *send_all_purple*.");
  }
}