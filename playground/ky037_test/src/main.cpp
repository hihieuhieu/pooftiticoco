#include <Arduino.h>
#include "time.h"
// #include <Async_Operations.h> //! think of necessity of async stuff

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
int sample_frequency = 2e3;
int sample_time = (1 / sample_frequency) * 1e6;

const int number_of_samples = 50; 
//! caution: if this number is too high, the arduino doesn't provide enough allocation storage

int sample_data[number_of_samples];

void setup() {
  pinMode(analogInputPin, INPUT);
  pinMode(digitalInputPin, INPUT);
  Serial.begin(115200);
  Serial.println("Initialized");
}

void loop() {
  unsigned long loop_start = micros();
  for (int i = 0; i < number_of_samples; i++)
  {
    sample_data[i] = analogRead(analogInputPin);
  }

  float loop_duration = (micros() - loop_start) / 1e6;

  Serial.print("Sample frequency: ");
  Serial.println(number_of_samples / loop_duration);

  Serial.println("start");

  // // Print data over Serial for DSP on laptop
  for (int i = 0; i < number_of_samples; i++)
  {
    Serial.println(sample_data[i] * (5.0/1023.0), 4);

  }
  
  Serial.println("stop");
  
  // Serial.print("Loop duration [s]: ");
  // Serial.println((micros() - loop_start)/1e6,6);
}
