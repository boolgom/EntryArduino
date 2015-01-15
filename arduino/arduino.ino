char remainData;

void setup(){
  Serial.begin(38400);
  Serial.flush();
  initPorts();
}

void loop(){
  while (Serial.available()) {
    //delay(5); //delay to allow buffer to fill
    if (Serial.available() >0) {
      char c = Serial.read(); //gets one byte from serial buffer
      updateDigitalPort(c);
    }
  }
   sendPinValues();
   delay(20);
}

void sendPinValues() {
  for (int pinNumber = 0; pinNumber < 6; pinNumber++) {
    sendAnalogValue(pinNumber, analogRead(pinNumber));
  }
}

void sendAnalogValue(int pinNumber, int value) {
  Serial.write(B11000000
               | ((pinNumber & B111)<<3)
               | ((value>>7) & B111));
  Serial.write(value & B1111111);
}

void initPorts () {
  for (int pinNumber = 2; pinNumber < 14; pinNumber++) {
    pinMode(pinNumber, OUTPUT);
    digitalWrite(pinNumber, 0);
  }
}

void updateDigitalPort (char c) {
  // first data
  if (c>>7) {
    // is output
    if ((c>>6) & 1) {
      // is data end at this chunk
      if ((c>>5) & 1) {
        int port = (c >> 1) & B1111;
        if (c & 1)
          digitalWrite(port, HIGH);
        else
          digitalWrite(port, LOW);
      }
      else {
        remainData = c;
      }
    }
  } else {
    int port = (remainData >> 1) & B1111;
    int value = ((remainData & 1) << 7) + (c & B1111111);
    analogWrite(port, value);
    remainData = 0;
  }
}
