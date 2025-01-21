#include <Arduino.h>
#include "time.h"

int analogInputPin = A0;
int digitalInputPin = 2;

template <int order> // order is 1 or 2
class LowPass
{
  private:
    float a[order];
    float b[order+1];
    float omega0;
    float dt;
    bool adapt;
    float tn1 = 0;
    float x[order+1]; // Raw values
    float y[order+1]; // Filtered values

  public:  
    LowPass(float f0, float fs, bool adaptive){
      // f0: cutoff frequency (Hz)
      // fs: sample frequency (Hz)
      // adaptive: boolean flag, if set to 1, the code will automatically set
      // the sample frequency based on the time history.
      
      omega0 = 6.28318530718*f0;
      dt = 1.0/fs;
      adapt = adaptive;
      tn1 = -dt;
      for(int k = 0; k < order+1; k++){
        x[k] = 0;
        y[k] = 0;        
      }
      setCoef();
    }

    void setCoef(){
      if(adapt){
        float t = micros()/1.0e6;
        dt = t - tn1;
        tn1 = t;
      }
      
      float alpha = omega0*dt;
      if(order==1){
        a[0] = -(alpha - 2.0)/(alpha+2.0);
        b[0] = alpha/(alpha+2.0);
        b[1] = alpha/(alpha+2.0);        
      }
      if(order==2){
        float alphaSq = alpha*alpha;
        float beta[] = {1, sqrt(2), 1};
        float D = alphaSq*beta[0] + 2*alpha*beta[1] + 4*beta[2];
        b[0] = alphaSq/D;
        b[1] = 2*b[0];
        b[2] = b[0];
        a[0] = -(2*alphaSq*beta[0] - 8*beta[2])/D;
        a[1] = -(beta[0]*alphaSq - 2*beta[1]*alpha + 4*beta[2])/D;      
      }
    }

    float filt(float xn){
      // Provide me with the current raw value: x
      // I will give you the current filtered value: y
      if(adapt){
        setCoef(); // Update coefficients if necessary      
      }
      y[0] = 0;
      x[0] = xn;
      // Compute the filtered values
      for(int k = 0; k < order; k++){
        y[0] += a[k]*y[k+1] + b[k]*x[k];
      }
      y[0] += b[order]*x[order];

      // Save the historical values
      for(int k = order; k > 0; k--){
        y[k] = y[k-1];
        x[k] = x[k-1];
      }
  
      // Return the filtered value    
      return y[0];
    }
};

float* abs_normalized_signal(float data[], const int size) {
  float* result = new float[size];

  float sum = 0.0;
  for (int i = 0; i < size; i++) {sum += data[i];}
  float mean = sum / size; //Calculate mean value of the signal

  for (int i = 0; i < size; i++) {result[i] = abs(data[i] - mean);}

  return result;
}

const int number_of_samples = 100; 
//! caution: if this number is too high, the arduino doesn't provide enough allocation storage

float sample_data[number_of_samples];
float filtered_data[number_of_samples];
float abs_offset_signal[number_of_samples];

// Filter instance
LowPass<2> lp(200,8840,false);

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

  // Serial.print("Sample frequency: ");
  // Serial.println(number_of_samples / loop_duration);

  unsigned long processing_duration_start = micros();

  for (int i = 0; i < number_of_samples; i++)
  {
    sample_data[i] *= (5.0/1023.0);
    filtered_data[i] = lp.filt(sample_data[i]);    
  }

  float* abs_offset_signal = abs_normalized_signal(filtered_data, number_of_samples);

  for (int i = 0; i < number_of_samples; i++)
  {
    // Serial.print(">raw:");
    // Serial.println(sample_data[i], 4);
    
    // Serial.print(">filtered:");
    // Serial.println(filtered_data[i]);

    Serial.print(">abs_offset:");
    Serial.println(abs_offset_signal[i]);
  }

  delete[] abs_offset_signal;
  
  
  Serial.print("Processing duration [s]: ");
  Serial.println((micros() - processing_duration_start)/1e6,6);
}

