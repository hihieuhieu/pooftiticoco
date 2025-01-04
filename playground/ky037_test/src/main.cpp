#include <Arduino.h>
#include "time.h"
#include <Async_Operations.h> //! think of necessity of async stuff

/*
async might become necessary if this code explodes by:
- FIR filter
- (FFT)
- Calculating power in frequency regions
- all the actoric stuff

and by this, the sampling rate would be decreased
*/

int analogInputPin = A0;
int digitalInputPin = 2;

void setup() {
  pinMode(analogInputPin, INPUT);
  pinMode(digitalInputPin, INPUT);
  Serial.begin(115200);
  Serial.println("Initialized");
}

void loop() {
  float measured_value;

  unsigned long start_time = micros();

  measured_value = analogRead(analogInputPin) * (5.0/1023.0);

  Serial.print(measured_value, 4);
  
  Serial.print("~");
  Serial.println((1/float(micros() - start_time))*1e6,2);
  
}
