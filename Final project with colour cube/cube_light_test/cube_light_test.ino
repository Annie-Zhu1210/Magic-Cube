// This project aims to show the combination of the three traditional primary colours
// in a colourful light way.
// There will be four sides of a cube been used,including yellow, blue, red and white sides.
// V1.1 to test for the red side only.

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

// Define the red pin
const int PIN_R = A2;  

// NeoPixel Configuration - we need to know this to know how to send messages 
// to vespera 
const int num_leds = 72;
const int payload_size = num_leds * 3;

// Create the byte array to send in MQTT payload this stores all the colours 
// in memory so that they can be accessed in for example the rainbow function
byte RGBpayload[payload_size];

void setup() {
  Serial.begin(115200);
  delay(60);
  Serial.println("Touch Sensor Luminaire Controller");

  // print the MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  printMacAddress(mac);
  
  // Create unique client ID from MAC address
  clientId = "MKR1010-" + String(mac[5], HEX) + String(mac[4], HEX);
  Serial.print("This device is Luminaire ");
  Serial.println(lightId);

  // Set up touch sensor pin
  pinMode(PIN_R, INPUT); 

  // Connect to WiFi
  startWifi();

  // Connect to MQTT broker
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setBufferSize(2200);
  mqttClient.setCallback(callback);
  reconnectMQTT();
  
  // Turn all LEDs off at startup
  send_all_off();
  LedOff();
  
  Serial.println("Set-up complete - Touch A2 for RED!");
}
 
void loop() {
  // Reconnect if necessary
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  
  if (WiFi.status() != WL_CONNECTED){
    startWifi();
  }
  
  // keep mqtt alive
  mqttClient.loop();

  // Simple debounce
  static unsigned long tDeb = 0;
  if(millis() - tDeb < 50) return;
  tDeb = millis();

  // Read touch sensor on A2
  bool touchNow = (digitalRead(PIN_R) == HIGH);
  static bool touchPrev = false;

  // Detect state change
  if(touchNow != touchPrev){
    if(touchNow){
      Serial.println("Touch detected - RED ON");
      send_all_red();
      LedRed();
    } else {
      Serial.println("Touch released - OFF");
      send_all_off();
      LedOff();
    }
    touchPrev = touchNow;
  }
}

// Function to send red to all pixels
void send_all_red() {
  // Check if the mqttClient is connected before publishing
  if (mqttClient.connected()) {
    // Update the byte array with red colour
    for(int pixel = 0; pixel < num_leds; pixel++){
      RGBpayload[pixel * 3 + 0] = (byte)255; // Red
      RGBpayload[pixel * 3 + 1] = (byte)0;   // Green
      RGBpayload[pixel * 3 + 2] = (byte)0;   // Blue
    }
    // Publish the byte array
    mqttClient.publish(mqtt_topic.c_str(), RGBpayload, payload_size);
    
    Serial.println("Published RED to all pixels.");
  } else {
    Serial.println("MQTT mqttClient not connected, cannot publish from *send_all_red*.");
  }
}

void send_all_off() {
  // Check if the mqttClient is connected before publishing
  if (mqttClient.connected()) {
    // Fill the byte array with zeros (off)
    for(int pixel = 0; pixel < num_leds; pixel++){
      RGBpayload[pixel * 3 + 0] = (byte)0; // Red
      RGBpayload[pixel * 3 + 1] = (byte)0; // Green
      RGBpayload[pixel * 3 + 2] = (byte)0; // Blue
    }
    // Publish the byte array
    mqttClient.publish(mqtt_topic.c_str(), RGBpayload, payload_size);
    
    Serial.println("Published all OFF.");
  } else {
    Serial.println("MQTT mqttClient not connected, cannot publish from *send_all_off*.");
  }
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}