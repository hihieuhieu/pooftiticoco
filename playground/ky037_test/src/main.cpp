#include <Arduino.h>

int analogInputPin = A0;
int digitalInputPin = 2;

void setup() {
  pinMode(analogInputPin, INPUT);
  pinMode(digitalInputPin, INPUT);
  Serial.begin(9600);
  Serial.println("Initialized");
}

void loop() {
  float measured_value;

  measured_value = analogRead(analogInputPin) * (5.0/1023.0);

  // Serial.println('A');

  Serial.println(measured_value, 4);
  
}
