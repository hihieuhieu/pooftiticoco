import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque
import numpy as np
from scipy.signal import medfilt


port = '/dev/ttyACM0'
baud_rate = 115200
max_data_points = 1000

ser = serial.Serial(port, baud_rate)

sensor_data = deque([0] * max_data_points, maxlen=max_data_points)
sampling_frequency = deque([0] * max_data_points, maxlen=max_data_points)

fig, ax1 = plt.subplots()

sensor_data_line, = ax1.plot(sensor_data, label='Sensor Data')
ax1.set_ylim(0, 5)
ax1.set_ylabel('Sensor Data (V)')
ax1.set_xlabel('Time')
ax1.grid(True)

text_display = ax1.text(0.7, 0.9, '', transform=ax1.transAxes)

def update(frame):
    try:
        values = (ser.readline().decode('utf-8').strip()).split('~')

        sensor_data.append(float(values[0]))
        sampling_frequency.append(float(values[1]))
        sensor_data_median_filtered = medfilt(sensor_data, kernel_size=5)
        sensor_data_line.set_ydata(sensor_data_median_filtered)
        text_display.set_text(f'fS [Hz]: {round(np.average(sampling_frequency),2)}')
        ax1.set_ylim(min(sensor_data_median_filtered) - 0.1, 
                        max(sensor_data_median_filtered) + 0.1)

    except Exception as e:
        print(f"Error: {e}")

    return sensor_data_line, text_display

ani = animation.FuncAnimation(fig, update, interval=1, blit=True, cache_frame_data=False)

plt.legend()
plt.show()

ser.close()
