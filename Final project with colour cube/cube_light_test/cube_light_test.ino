// This project aims to show the combination of the three traditional primary colours
// in a colourful light way.
// There will be four sides of a cube been used,including yellow, blue, red and white sides.
// V2.1 to test for the white side and brightness control.

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

// Define the four pins
const int PIN_R = A2;  
const int PIN_Y = A3;
const int PIN_B = A4;
const int PIN_W = 6;

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
void LedWhite();
void LedOff();

void send_all_off();
void send_all_red();
void send_all_yellow();
void send_all_orange();
void send_all_blue();
void send_all_green();
void send_all_purple();
void send_all_white();

// Rainbow Effect -- Red, orange, yellow, green, blue, purple
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

// Brightness control
uint8_t brightness = 180;
const uint8_t BRIGHT_MIN = 40;
const uint8_t BRIGHT_MAX = 255;
const uint8_t BRIGHT_STEP = 12;
int8_t brightDir = +1;
unsigned long lastBrightStep = 0;
const unsigned long BRIGHT_INTERVAL_MS = 140;

// Touch sensors were out of control, may cuased by noise or false triggering. 
// So the number of reads has increased, and the number of TRUE returns received has been improved to prevent the mess.
bool readTouchStable(int pin) {
  const uint8_t SAMPLES = 7;
  const uint8_t NEED    = 5;
  uint8_t hi = 0;
  for (uint8_t i=0;i<SAMPLES;i++){
    if (digitalRead(pin) == HIGH) hi++;
    delayMicroseconds(2000);
  }
  return (hi >= NEED);
}


inline uint8_t scale(uint8_t v){ return (uint16_t)v * brightness / 255; }

inline void publish_payload(){
  if (mqttClient.connected()){
    mqttClient.publish(mqtt_topic.c_str(), RGBpayload, payload_size);
  }
}

inline void fill_all_scaled(uint8_t r, uint8_t g, uint8_t b){
  uint8_t R = scale(r), G = scale(g), B = scale(b);
  for (int i=0; i<num_leds; i++){
    RGBpayload[i*3+0] = R;
    RGBpayload[i*3+1] = G;
    RGBpayload[i*3+2] = B;
  }
  publish_payload();
}

inline void send_blocks_frame_scaled(int offset){
  if (!mqttClient.connected()) return;
  for(int b=0; b<BLOCKS; b++){
    const uint8_t* c = COLS[(b + offset) % BLOCKS];
    uint8_t R = scale(c[0]);
    uint8_t G = scale(c[1]);
    uint8_t B = scale(c[2]);
    int start = b * BLOCK_SIZE;
    int end   = start + BLOCK_SIZE;
    for(int i=start; i<end; i++){
      RGBpayload[i*3 + 0] = R;
      RGBpayload[i*3 + 1] = G;
      RGBpayload[i*3 + 2] = B;
    }
  }
  publish_payload();
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
  Serial.println("Touch Sensor Luminaire Controller (R/Y/B/W + robust filtering)");


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
  pinMode(PIN_W, INPUT);

  // WiFi + MQTT
  startWifi();
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setBufferSize(2200);
  mqttClient.setCallback(callback);
  reconnectMQTT();

  send_all_off(); LedOff();
  Serial.println("Set-up complete - A2=RED, A3=YELLOW, A4=BLUE, D6=WHITE");
}

void loop() {
  if (!mqttClient.connected()) reconnectMQTT();
  if (WiFi.status() != WL_CONNECTED) startWifi();
  mqttClient.loop();

  bool rNow = readTouchStable(PIN_R);
  bool yNow = readTouchStable(PIN_Y);
  bool bNow = readTouchStable(PIN_B);
  bool wNow = readTouchStable(PIN_W);

  static bool rPrev=false, yPrev=false, bPrev=false, wPrev=false;

  if (rNow != rPrev || yNow != yPrev || bNow != bPrev || wNow != wPrev) {
    rPrev=rNow; yPrev=yNow; bPrev=bNow; wPrev=wNow;

    if (wNow && !(rNow || yNow || bNow)) {
      Serial.println("WHITE only");
      LedWhite(); send_all_white();
    }
    else if (rNow && yNow && bNow) {
      Serial.println("R+Y+B - COLOURFUL EFFECT (blocks)");
      animOffset = 0; lastAnim = 0;
      send_blocks_frame_scaled(animOffset);
      animOffset = (animOffset + 1) % BLOCKS;
    } 
    else if (rNow && yNow && !bNow) {
      Serial.println("R+Y - ORANGE");
      LedOrange(); send_all_orange();
    } 
    else if (rNow && !yNow && bNow) {
      Serial.println("R+B - PURPLE");
      LedPurple(); send_all_purple();
    } 
    else if (!rNow && yNow && bNow) {
      Serial.println("Y+B - GREEN");
      LedGreen(); send_all_green();
    } 
    else if (rNow && !yNow && !bNow) {
      Serial.println("RED only");
      LedRed(); send_all_red();
    } 
    else if (!rNow && yNow && !bNow) {
      Serial.println("YELLOW only");
      LedYellow(); send_all_yellow();
    } 
    else if (!rNow && !yNow && bNow) {
      Serial.println("BLUE only");
      LedBlue(); send_all_blue();
    } 
    else {
      Serial.println("No touch - OFF");
      LedOff(); send_all_off();
    }
  }

  // The rainbow effect
  if (rPrev && yPrev && bPrev) {
    if (millis() - lastAnim >= ANIM_INTERVAL_MS) {
      lastAnim = millis();
      send_blocks_frame_scaled(animOffset);
      animOffset = (animOffset + 1) % BLOCKS;
    }
  }

  // Touch the white side with other colours will change the brightness
  if (wPrev && (rPrev || yPrev || bPrev)) {
    if (millis() - lastBrightStep >= BRIGHT_INTERVAL_MS) {
      lastBrightStep = millis();
      int16_t next = (int16_t)brightness + brightDir * BRIGHT_STEP;
      if (next >= BRIGHT_MAX) { next = BRIGHT_MAX; brightDir = -1; Serial.println("Brightness: MAX"); }
      else if (next <= BRIGHT_MIN) { next = BRIGHT_MIN; brightDir = +1; Serial.println("Brightness: MIN"); }
      brightness = (uint8_t)next;

      if (rPrev && yPrev && bPrev)      send_blocks_frame_scaled(animOffset);
      else if (rPrev && yPrev)          send_all_orange();
      else if (rPrev && bPrev)          send_all_purple();
      else if (yPrev && bPrev)          send_all_green();
      else if (rPrev)                   send_all_red();
      else if (yPrev)                   send_all_yellow();
      else if (bPrev)                   send_all_blue();
      else                              send_all_white();
    }
  }
}

void send_all_off()      { fill_all_scaled(0,   0,   0  ); Serial.println("Published OFF."); }
void send_all_red()      { fill_all_scaled(255, 0,   0  ); Serial.println("Published RED."); }
void send_all_yellow()   { fill_all_scaled(255, 255, 0  ); Serial.println("Published YELLOW."); }
void send_all_orange()   { fill_all_scaled(255, 128, 0  ); Serial.println("Published ORANGE."); }
void send_all_blue()     { fill_all_scaled(0,   0,   255); Serial.println("Published BLUE."); }
void send_all_green()    { fill_all_scaled(0,   255, 0  ); Serial.println("Published GREEN."); }
void send_all_purple()   { fill_all_scaled(255, 0,   255); Serial.println("Published PURPLE."); }
void send_all_white()    { fill_all_scaled(255, 255, 255); Serial.println("Published WHITE."); }