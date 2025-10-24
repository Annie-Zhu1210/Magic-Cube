// Functions to control the onboard RGB LED on MKR WiFi 1010
// These provide visual feedback for connection status and touch states

void LedRed(){
  WiFiDrv::analogWrite(25, 155);
  WiFiDrv::analogWrite(26, 0);
  WiFiDrv::analogWrite(27, 0);  
}

void LedYellow(){
  WiFiDrv::analogWrite(25, 155);
  WiFiDrv::analogWrite(26, 155);
  WiFiDrv::analogWrite(27, 0);  
}

void LedOrange(){
  WiFiDrv::analogWrite(25, 155);
  WiFiDrv::analogWrite(26, 80);
  WiFiDrv::analogWrite(27, 0);  
}

void LedBlue(){
  WiFiDrv::analogWrite(25, 0);
  WiFiDrv::analogWrite(26, 0);
  WiFiDrv::analogWrite(27, 155); 
}

void LedGreen(){
  WiFiDrv::analogWrite(25, 0);
  WiFiDrv::analogWrite(26, 155);
  WiFiDrv::analogWrite(27, 0);  
}

void LedPurple(){
  WiFiDrv::analogWrite(25, 155);
  WiFiDrv::analogWrite(26, 0);
  WiFiDrv::analogWrite(27, 155); 
}

void LedWhite(){
  WiFiDrv::analogWrite(25, 155);
  WiFiDrv::analogWrite(26, 155);
  WiFiDrv::analogWrite(27, 155); 
}

void LedOff(){
  WiFiDrv::analogWrite(25, 0);
  WiFiDrv::analogWrite(26, 0);
  WiFiDrv::analogWrite(27, 0);  
}