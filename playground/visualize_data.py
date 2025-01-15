from __future__ import print_function
from __future__ import division
import numpy as np
import serial
import matplotlib.pyplot as plt
import time
from collections import deque
from scipy.signal import medfilt
from scipy.signal import butter, filtfilt 
import scipy.integrate as integrate

def butter_lowpass(cutoff, fs, order=4):
    nyquist = 0.5 * fs
    normal_cutoff = cutoff / nyquist
    b, a = butter(order, normal_cutoff, btype='low', analog=False)
    return b, a

def lowpass_filter(data, cutoff=2.0, fs=50.0, order=4):
    """Applies a low-pass filter to the data."""
    b, a = butter_lowpass(cutoff, fs, order)
    return filtfilt(b, a, data)

def butter_bandpass(lowcut, highcut, fs, order=4):
    nyquist = 0.5 * fs
    low = lowcut / nyquist
    high = highcut / nyquist
    b, a = butter(order, [low, high], btype='band', analog=False)
    return b, a

def bandpass_filter(data, lowcut=1.0, highcut=10.0, fs=50.0, order=4):
    """Applies a bandpass filter to the data."""
    b, a = butter_bandpass(lowcut, highcut, fs, order)
    return filtfilt(b, a, data)

def signal_energy(signal, buffer_length):
    energy = integrate.quad(signal, 0, buffer_length)
    return energy

port = '/dev/ttyACM0' 
baud_rate = 115200

data_storage = []  
in_data_frame = False
# data_storage = []
data_buffer_size = 2000
data_buffer = deque([0] * data_buffer_size, maxlen=data_buffer_size)  

ser = serial.Serial(port, baud_rate, timeout=1)
time.sleep(2)  

plt.ion() 
fig, ax = plt.subplots()
unfiltered_plot, = ax.plot([], [], 'b', alpha=0.2, label='unfiltered')  # Initial empty plot
lowpass_filtered_plot, = ax.plot([], [], 'r', label='Lowpass: fC = 300Hz')
# bandpass_filtered_plot, = ax.plot([], [], 'g', label='Bandpass: fC = 300, 1500 Hz')
ax.set_xlabel('Sample Number')
ax.set_ylabel('Data Value')
ax.set_title('Real-Time Data Plot')
ax.legend(loc='upper right')

try:
    while True:
        if ser.in_waiting > 0:
            serial_read = ser.readline()

            try:
                line = serial_read.decode('utf-8').strip()
            except UnicodeDecodeError:
                print(f"Received invalid data: {serial_read}")
                continue

            if in_data_frame and line != 'stop':
                try:
                    data_storage.append(float(line))
                except ValueError:
                    print(f"Non-numeric data received: {line}")
                    continue

            if line == 'start':
                in_data_frame = True
            elif line == 'stop':
                in_data_frame = False

                if data_storage:
                    data_buffer.extend(medfilt(data_storage))  

                    lowpass_filtered_buffer = lowpass_filter(list(data_buffer), cutoff=300, fs=8800, order=4)
                    bandpass_filtered_buffer = bandpass_filter(list(data_buffer), lowcut=300, highcut=1500, fs=8800, order=4)

                    unfiltered_plot.set_xdata(range(len(data_buffer)))
                    unfiltered_plot.set_ydata(data_buffer)

                    lowpass_filtered_plot.set_xdata(range(len(lowpass_filtered_buffer)))
                    lowpass_filtered_plot.set_ydata(lowpass_filtered_buffer)

                    # bandpass_filtered_plot.set_xdata(range(len(bandpass_filtered_buffer)))
                    # bandpass_filtered_plot.set_ydata(bandpass_filtered_buffer)

                    # current_signal_energy = signal_energy(data_buffer, len(data_buffer))

                    # print(f'Signal energy: {current_signal_energy}')

                    ax.relim()  
                    ax.autoscale_view(True, True, True)  
                    plt.draw()
                    plt.pause(0.01)
                
                data_storage = []

            elif 'Sample frequency' in line:
                frequency = float(line.split(":")[1].strip())
                print(f'fS: {frequency}')

except KeyboardInterrupt:
    print("\nProgram terminated by user.")

finally:
    if ser.is_open:
        ser.close()
    plt.ioff()
    plt.show() 
    print("Serial port closed.")














# # Continuously read and print data
# while True:
#     if ser.in_waiting > 0:
#         serial_read = ser.readline()

#         try:
#             line = serial_read.decode('utf-8').strip()
#         except UnicodeDecodeError:
#             print(f"Received invalid data: {line}")
#             continue
        
#         if in_data_frame == True and line != 'stop':
#             data_storage.append(float(line))

#         if line == 'start':
#             in_data_frame = True
#         elif line == 'stop':
#             in_data_frame = False
#             data_storage = []



