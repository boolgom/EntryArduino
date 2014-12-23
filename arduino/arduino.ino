
int analogPin = 3;   // potentiometer connected to analog pin 3

int val = 0;         // variable to store the read value



void setup(){
  Serial.begin(9600);
}

void loop(){
   val = analogRead(analogPin);
   int bytesSent = Serial.write(val, DEC); 
}
