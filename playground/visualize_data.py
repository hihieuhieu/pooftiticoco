import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque
import numpy as np
from scipy.signal import medfilt

# Configuration
port = '/dev/ttyACM0'
baud_rate = 115200
max_data_points = 1000

ser = serial.Serial(port, baud_rate)

sensor_data = deque([0] * max_data_points, maxlen=max_data_points)
instantaneous_sampling_frequency = deque([0] * max_data_points, maxlen=max_data_points)

# Create figure
fig, ax1 = plt.subplots()
sensor_data_line, = ax1.plot(sensor_data)
ax1.set_ylim(0, 5) 

def update(frame):
    try:
        values = (ser.readline().decode('utf-8').strip()).split('~')

        if values:
            sensor_data.append(float(values[0]))
            instantaneous_sampling_frequency.append(float(values[1]))

        sensor_data_filtered = medfilt(sensor_data, kernel_size=5)

        sensor_data_line.set_ydata(sensor_data_filtered)

        print(f'min: {min(sensor_data)}, max: {max(sensor_data)}')

        ax1.set_ylim(min(sensor_data_filtered) - 0.1, max(sensor_data_filtered) + 0.1)

    except Exception as e:
        print(f"Error: {e}")

    return sensor_data_line,

# interval in ms
ani = animation.FuncAnimation(fig, update, interval=1, blit=True, cache_frame_data=False)
plt.show()

ser.close()

