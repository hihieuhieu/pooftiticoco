# from __future__ import print_function
# from __future__ import division
# import numpy as np
# import serial
# import matplotlib.pyplot as plt
# import time
# from collections import deque
# from scipy.signal import medfilt
# from scipy.signal import butter, filtfilt 

# def butter_lowpass(cutoff, fs, order=4):
#     nyquist = 0.5 * fs
#     normal_cutoff = cutoff / nyquist
#     b, a = butter(order, normal_cutoff, btype='low', analog=False)
#     return b, a

# def lowpass_filter(data, cutoff=2.0, fs=50.0, order=4):
#     """Applies a low-pass filter to the data."""
#     b, a = butter_lowpass(cutoff, fs, order)
#     return filtfilt(b, a, data)

# def fir_lowpass_filter(sampling_frequency: float=8800., 
#                cutoff_frequency: float=300., 
#                filter_length: int=33):
#     # fS = 8800  # Sampling rate.
#     # fL = 300  # Cutoff frequency.
#     # N = 33  # Filter length, must be odd.

#     h = np.sinc(2 * cutoff_frequency / sampling_frequency * (np.arange(filter_length) - (filter_length - 1) / 2))

#     # Apply window.
#     h *= np.blackman(filter_length)

#     # Normalize to get unity gain.
#     h /= np.sum(h)

#     print(h)

#     # Applying the filter to a signal s can be as simple as writing
#     # s = np.convolve(s, h)

# def signal_energy(signal):
#     """
#     ...
#     """
    
#     mean = np.mean(signal)
#     nulled_abs_signal = np.abs(signal - mean)
#     sum_signal = np.sum(nulled_abs_signal)

#     return sum_signal

# port = '/dev/ttyACM0' 
# baud_rate = 115200

# low_pass_cutoff = 300

# data_storage = []  
# in_data_frame = False
# data_buffer_size = 200
# signal_energy_buffer_size = 20
# data_buffer = deque([0] * data_buffer_size, maxlen=data_buffer_size)
# lowpass_signal_energy_buffer = deque([0] * signal_energy_buffer_size, maxlen=signal_energy_buffer_size)

# # --- CREATE SERIAL CONNECTION --- #
# ser = serial.Serial(port, baud_rate, timeout=1)
# time.sleep(2)  

# # --- COMPUTE THE FILTER --- #
# fir_lowpass = fir_lowpass_filter(sampling_frequency=8800, cutoff_frequency=low_pass_cutoff, filter_length=33)

# # --- CREATE PLOT FIGURES --- #
# plt.ion() 
# fig, ax = plt.subplots()
# fig2, ax2 = plt.subplots()
# unfiltered_plot, = ax.plot([], [], 'b', alpha=0.2, label='unfiltered')  # Initial empty plot
# lowpass_filtered_plot, = ax.plot([], [], 'r', label=f'Lowpass: fC = {low_pass_cutoff}Hz')
# lowpass_filtered_energy_plot, = ax2.plot([], [], 'g', label='LP signal energy')

# ax.set_xlabel('Sample Number')
# ax.set_ylabel('Data Value')
# ax.legend(loc='upper right')

# ax2.set_xlim(0, signal_energy_buffer_size-1)
# ax2.set_ylim(0, 30)
# ax2.set_xlabel('Sample')
# ax2.set_ylabel('Integrated lowpass-signal')

# try:
#     while True:
#         if ser.in_waiting > 0:
#             serial_read = ser.readline()

#             try:
#                 line = serial_read.decode('utf-8').strip()
#             except UnicodeDecodeError:
#                 print(f"Received invalid data: {serial_read}")
#                 continue

#             if in_data_frame and line != 'stop':
#                 try:
#                     data_storage.append(float(line))
#                 except ValueError:
#                     print(f"Non-numeric data received: {line}")
#                     continue

#             if line == 'start':
#                 in_data_frame = True
#             elif line == 'stop':
#                 in_data_frame = False

#                 if data_storage:
#                     data_buffer.extend(medfilt(data_storage))  

#                     lowpass_filtered_buffer = lowpass_filter(list(data_buffer), cutoff=low_pass_cutoff, fs=8800, order=4)
#                     lowpass_filtered_signal_energy = signal_energy(lowpass_filtered_buffer)

#                     # low_pass_filtered

#                     lowpass_signal_energy_buffer.extend([lowpass_filtered_signal_energy])

#                     unfiltered_plot.set_xdata(range(len(data_buffer)))
#                     unfiltered_plot.set_ydata(data_buffer)

#                     lowpass_filtered_plot.set_xdata(range(len(lowpass_filtered_buffer)))
#                     lowpass_filtered_plot.set_ydata(lowpass_filtered_buffer)

#                     lowpass_filtered_energy_plot.set_xdata(range(len(lowpass_signal_energy_buffer)))
#                     lowpass_filtered_energy_plot.set_ydata(lowpass_signal_energy_buffer)

#                     # print(f'Lowpass filtered signal energy: {lowpass_filtered_signal_energy}')
#                     # print(lowpass_signal_energy_buffer)

#                     ax.relim()  
#                     ax.autoscale_view(True, True, True)  
#                     # ax2.relim()  
#                     ax2.autoscale_view(True, True, True)  
#                     plt.draw()
#                     plt.pause(0.01)
                
#                 data_storage = []

#             elif 'Sample frequency' in line:
#                 frequency = float(line.split(":")[1].strip())
#                 # print(f'fS: {frequency}')

# except KeyboardInterrupt:
#     print("\nProgram terminated by user.")

# finally:
#     if ser.is_open:
#         ser.close()
#     plt.ioff()
#     plt.show() 
#     print("Serial port closed.")












import socket
import math
import time

teleplotAddr = ("127.0.0.1",47269)
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def sendTelemetry(name, value):
    now = time.time() * 1000
    msg = name+":"+str(now)+":"+str(value)+"|g"
    sock.sendto(msg.encode(), teleplotAddr)

i=0
while i < 1000:
    
    sendTelemetry("sin", math.sin(i))
    sendTelemetry("cos", math.cos(i))

    i+=0.1
    time.sleep(0.01)