# some notes

## Microphone sensor KY-037

![alt text](image.png)

- frequency range: 50Hz...22kHz
    - i.e. is this the bandwidth? i.e. 
- seems like A0 and D0 is basically the same signal, but D0 is amplified. 
- potentiometer to adjust voltage level, i.e. adjust signal amplitude
- i guess the microphone can be thought of as some kind of capacitor with some impedance lets say time-dependent impedance $Z(t)$, depending on the membranes configuration and accordingly the instantaneous capacity. accordingly, the voltage on the opamp input depends on the microphones capacity, which can be adjusted via the potentiometer
- probably response time of microphone and opamp are not quick enough for high frequencies, but should be suitable up to some relevant spectral ranges especially in lower frequency domain

