#include <Arduino.h>
#include "time.h"

int analogInputPin = A0;
int digitalInputPin = 2;

int digitalOutputTransistorGate = 8;
int digitalOutputTransistorDrain = 9;

//* number of samples to take for one filtering
const int n_acquisition_samples = 100; 
// for n=100, this takes approx 11.21ms (acquisition) + 13.3ms (filtering) = 24.51ms
//! caution: if this number is too high, the arduino doesn't provide enough allocation storage

//* number of samples for reacquisition for consecutive energy calculation
const int n_fifo_samples = 300; 
//for n=500, this takes approx 3*24.51ms = 73.53ms 

const int n_integration_samples = 1000;
// for n=500, this takes approx 10*24.51ms = 245.1ms

int n_currently_acquired_samples = 0;

float sample_frequency = 8840.;
float cut_off_frequency = 200.;

float sample_duration = 1 / sample_frequency;

//* max beats per minute to record
float bpm = 160.; 
//* adjust this according to whatever you think a beat duration is
float beat_duration = 350 * 1e-3; 
float beat_frequency = bpm / 60; 

float led_threshold = 1.5;

int beat_duration_samples = int( beat_duration / sample_duration );

// sum of processed signal of length number_of_samples
float sum_filtered_instantaneous_signal = 0.;
// sum of processed signal of length integration_samples
float sum_integrated_signal = 0.;
// sum of fifo_samples for new acquisition
float sum_fifo_signal_new = 0.;
// sum of fifo_samples of previous samples
float sum_fifo_signal_old = 0.;

template <int order> //* order is 1 or 2
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


float abs_normalized_instantaneous_signal(float data[], const int size) {
  /*
  Given a time sampled signal s[n] with mean value s0, this function
  computes |s[n]-s0| as a metric for the power within an audio signal. 
  */
  float* result = new float[size];

  // for calculating signal mean
  float input_signal_sum = 0.0;

  // for calculating signal frame energy
  float processes_signal_sum = 0.0;

  for (int i = 0; i < size; i++) {input_signal_sum += data[i];}
  
  //* Calculate mean value s0 of the signal
  float mean = input_signal_sum / size; 

  for (int i = 0; i < size; i++) {
    // calculate absolute mean-free signal
    result[i] = abs(data[i] - mean);
    // calculate total signal frame energy
    processes_signal_sum += result[i];
  }

  //! for visualization purposes only:
  for (int i = 0; i < n_acquisition_samples; i++)
  {
    // Serial.print(">raw:");
    // Serial.println(sample_data[i], 4);
    
    // Serial.print(">filtered:");
    // Serial.println(filtered_data[i]);

    // Serial.print(">abs_offset:");
    // Serial.println(result[i]);
  }
  //!

  delete[] result;

  return processes_signal_sum;
}


float sample_data[n_acquisition_samples];
float filtered_data[n_acquisition_samples];
float abs_offset_signal[n_acquisition_samples];

//* Filter instance
//* cutoff: 200Hz, fS: 8840Hz (measured via loop function)
//* --> each sample takes 113.122 µs
LowPass<2> lp(cut_off_frequency,sample_frequency,false);

void setup() {
  pinMode(analogInputPin, INPUT);
  pinMode(digitalInputPin, INPUT);
  pinMode(digitalOutputTransistorGate, OUTPUT);
  pinMode(digitalOutputTransistorDrain, OUTPUT);
  Serial.begin(115200);
  Serial.println("Initialized");
  digitalWrite(digitalOutputTransistorDrain, HIGH);
  digitalWrite(digitalOutputTransistorGate, HIGH);

  int i = 0;
  while ( i <= n_integration_samples ){
    // fill up the buffer initially
    for (int i = 0; i < n_acquisition_samples; i++)
    {
      sample_data[i] = analogRead(analogInputPin);
    }

    for (int i = 0; i < n_acquisition_samples; i++)
    {
      sample_data[i] *= (5.0/1023.0);
      filtered_data[i] = lp.filt(sample_data[i]);    
    }
    //* mean-free abs sum
    sum_integrated_signal += abs_normalized_instantaneous_signal(filtered_data, n_acquisition_samples);
    i += n_acquisition_samples;
  }

}

void loop() {
  // double loop_start = micros();

  //* Record <number_of_samples> samples, which are to be processed
  for (int i = 0; i < n_acquisition_samples; i++)
  {
    sample_data[i] = analogRead(analogInputPin);
  }

  //* loop_duration to calculate sample frequency fS
  // double loop_duration = (micros() - loop_start) / 1e6;
  // Serial.print("loop_duration: ");
  // Serial.println(loop_duration*1e3, 6);
  // Serial.print("Sample frequency: ");
  // Serial.println(number_of_samples / loop_duration);
  // unsigned long processing_duration_start = micros();

  //* Lowpass filter data
  for (int i = 0; i < n_acquisition_samples; i++)
  {
    sample_data[i] *= (5.0/1023.0);
    filtered_data[i] = lp.filt(sample_data[i]);    
  }
  //* mean-free abs sum
  sum_filtered_instantaneous_signal = abs_normalized_instantaneous_signal(filtered_data, n_acquisition_samples);

  sum_fifo_signal_new += sum_filtered_instantaneous_signal;
  n_currently_acquired_samples += n_acquisition_samples;

  if (n_currently_acquired_samples >= n_fifo_samples){
    sum_integrated_signal = sum_integrated_signal + sum_fifo_signal_new - sum_fifo_signal_old;
    n_currently_acquired_samples = 0;

    if (sum_integrated_signal >= led_threshold){
      digitalWrite(digitalOutputTransistorGate, HIGH);
    } else {digitalWrite(digitalOutputTransistorGate, LOW);}

    sum_fifo_signal_old = sum_fifo_signal_new;
    sum_integrated_signal = 0;

    // Serial.print(">sum filtered signal:");
    // Serial.println(sum_integrated_signal); 
  }
  
  // Serial.print("Processing duration [s]: ");
  // Serial.println((micros() - processing_duration_start)/1e6,6);
}

