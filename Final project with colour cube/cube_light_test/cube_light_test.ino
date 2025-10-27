// This project aims to show the combination of the three traditional primary colours
// in a colourful light way.
// There will be four sides of a cube been used,including yellow, blue, red and white sides.
// V3.1 with lights control changes from long-press to short-press. 
// Each sensor now has two status: 0 for off and 1 for on.

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
const uint8_t BRIGHT_MIN = 30;
const uint8_t BRIGHT_MAX = 255;
const uint8_t BRIGHT_STEP = 6;
int8_t brightDir = +1;
unsigned long lastBrightStep = 0;
const unsigned long BRIGHT_INTERVAL_MS = 150;

uint8_t latched = 0;
bool whiteOn = false;

inline uint8_t scale(uint8_t v){ return (uint16_t)v * brightness / 255; }

inline void publish_payload(){
  if (mqttClient.connected()){
    mqttClient.publish(mqtt_topic.c_str(), RGBpayload, payload_size);
  }
}

inline void fill_all_scaled(uint8_t r, uint8_t g, uint8_t b){
  const uint8_t R = scale(r), G = scale(g), B = scale(b);
  for (int i=0;i<num_leds;i++){
    RGBpayload[i*3+0]=R; RGBpayload[i*3+1]=G; RGBpayload[i*3+2]=B;
  }
  publish_payload();
}

inline void send_blocks_frame_scaled(int offset){
  if (!mqttClient.connected()) return;
  for (int b=0;b<BLOCKS;b++){
    const uint8_t* c = COLS[(b + offset) % BLOCKS];
    const uint8_t R=scale(c[0]), G=scale(c[1]), B=scale(c[2]);
    const int start=b*BLOCK_SIZE, end=start+BLOCK_SIZE;
    for (int i=start;i<end;i++){
      RGBpayload[i*3+0]=R; RGBpayload[i*3+1]=G; RGBpayload[i*3+2]=B;
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

void applyOutput(bool forceSend=false){
  // White light only appears when no colours are latched
  if (whiteOn && (latched==0)){
    LedWhite(); send_all_white(); return;
  }
  if (latched==0){
    LedOff(); send_all_off(); return;
  }

  if ((latched & 0x7)==0b111 && !whiteOn){
    if (forceSend){ animOffset=0; lastAnim=0; }
    if (millis()-lastAnim >= ANIM_INTERVAL_MS){
      lastAnim = millis();
      send_blocks_frame_scaled(animOffset);
      animOffset = (animOffset + 1) % BLOCKS;
    }
    return;
  }

  switch (latched & 0x7){
    case 0b001: LedRed();    send_all_red();    break;
    case 0b010: LedYellow(); send_all_yellow(); break;
    case 0b100: LedBlue();   send_all_blue();   break;
    case 0b011: LedOrange(); send_all_orange(); break;
    case 0b101: LedPurple(); send_all_purple(); break;
    case 0b110: LedGreen();  send_all_green();  break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(60);
  Serial.println("Touch Sensor Luminaire Controller (latched taps + white toggle + slow brightness)");

  // MAC to clientId
  byte mac[6]; WiFi.macAddress(mac);
  Serial.print("MAC address: "); printMacAddress(mac);
  clientId = "MKR1010-" + String(mac[5],HEX) + String(mac[4],HEX);

  Serial.print("This device is Luminaire ");
  Serial.println(lightId);

  // Inputs
  pinMode(PIN_R, INPUT);
  pinMode(PIN_Y, INPUT);
  pinMode(PIN_B, INPUT);
  pinMode(PIN_W, INPUT);

  // Network/MQTT
  startWifi();
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setBufferSize(2200);
  mqttClient.setCallback(callback);
  reconnectMQTT();

  send_all_off(); LedOff();
  Serial.println("Ready: A2=R, A3=Y, A4=B (tap to toggle); D6=White (tap on/off, hold brightness).");
}

void loop() {
  if (!mqttClient.connected()) reconnectMQTT();
  if (WiFi.status()!=WL_CONNECTED) startWifi();
  mqttClient.loop();

  static unsigned long tDeb=0;
  if (millis()-tDeb < 30) {
    if ((latched & 0x7)==0b111 && !whiteOn) applyOutput(false);
    return;
  }
  tDeb = millis();

  bool rNow = (digitalRead(PIN_R)==HIGH);
  bool yNow = (digitalRead(PIN_Y)==HIGH);
  bool bNow = (digitalRead(PIN_B)==HIGH);
  bool wNow = (digitalRead(PIN_W)==HIGH);

  static bool rPrev=false, yPrev=false, bPrev=false, wPrev=false;
  static unsigned long wDown=0;
  static bool inLongBrightness=false;

  if (rNow && !rPrev) { latched ^= 0b001; Serial.print("R toggled -> "); Serial.println((latched&1)?"ON":"OFF"); applyOutput(true); }
  if (yNow && !yPrev) { latched ^= 0b010; Serial.print("Y toggled -> "); Serial.println((latched&2)?"ON":"OFF"); applyOutput(true); }
  if (bNow && !bPrev) { latched ^= 0b100; Serial.print("B toggled -> "); Serial.println((latched&4)?"ON":"OFF"); applyOutput(true); }

  // Define the functions on the white side:
  // Short press = white light, long press = brightness 
  if (wNow && !wPrev) { wDown = millis(); inLongBrightness=false; }
  if (wNow && !inLongBrightness && (millis()-wDown >= 600)) {
    inLongBrightness = true;
    Serial.println("White long-press: brightness control active");
  }
  if (!wNow && wPrev) {
    if (!inLongBrightness) {
      whiteOn = !whiteOn;
      Serial.print("White toggle -> "); Serial.println(whiteOn?"ON (white)":"OFF");
      applyOutput(true);
    } else {
      inLongBrightness = false;
      Serial.println("White long-press ended");
    }
  }

  rPrev=rNow; yPrev=yNow; bPrev=bNow; wPrev=wNow;

  // Keep the rainbow effect if R+Y+B unless white on.
  if ((latched & 0x7)==0b111 && !whiteOn) applyOutput(false);

  // Brightness slowly changes when long-press the white side.
  if (inLongBrightness){
    const bool anyColour = (latched & 0x7) != 0;
    if (millis()-lastBrightStep >= BRIGHT_INTERVAL_MS){
      lastBrightStep = millis();
      int16_t next = (int16_t)brightness + brightDir * BRIGHT_STEP;
      if (next >= BRIGHT_MAX){ next=BRIGHT_MAX; brightDir=-1; Serial.println("Brightness: MAX"); }
      else if (next <= BRIGHT_MIN){ next=BRIGHT_MIN; brightDir=+1; Serial.println("Brightness: MIN"); }
      brightness = (uint8_t)next;

      if (whiteOn && !anyColour) {
        send_all_white();
      } else {
        if      ((latched & 0x7)==0b111) send_blocks_frame_scaled(animOffset);
        else if ((latched & 0x7)==0b011) send_all_orange();
        else if ((latched & 0x7)==0b101) send_all_purple();
        else if ((latched & 0x7)==0b110) send_all_green();
        else if ((latched & 0x7)==0b001) send_all_red();
        else if ((latched & 0x7)==0b010) send_all_yellow();
        else if ((latched & 0x7)==0b100) send_all_blue();
        else                             send_all_off();
      }
    }
  } else {
    applyOutput(false);
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