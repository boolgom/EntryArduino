
int analogPin = 3;   // potentiometer connected to analog pin 3

int val = 0;         // variable to store the read value



void setup(){
  Serial.begin(38400);
  Serial.flush();
  initPorts();
}

void loop(){
   sendPinValues();
   delay(20);
}

void sendPinValues() {
  Serial.write('a');
  for (int pinNumber = 0; pinNumber < 6; pinNumber++) {
    sendAnalogValue(pinNumber, analogRead(pinNumber));
  }
}

void sendAnalogValue(int pinNumber, int value) {
  Serial.write(B11000000
               | ((pinNumber & B111)<<4)
               | ((value>>7) & B111));
  Serial.write(value & B1111111);
}

void initPorts () {
  for (int pinNumber = 2; pinNumber < 14; pinNumber++) {
    pinMode(pinNumber, OUTPUT);
    digitalWrite(pinNumber, 0);
  }
}
