import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque
import numpy as np

# Configuration
port = '/dev/ttyACM0'
baud_rate = 9600
max_data_points = 1000

ser = serial.Serial(port, baud_rate)

data = deque([0] * max_data_points, maxlen=max_data_points)

# Create figure
fig, ax = plt.subplots()
line, = ax.plot(data)
ax.set_ylim(0, 5) 

def update(frame):
    try:
        value = float(ser.readline().decode('utf-8').strip())
        data.append(value)
        line.set_ydata(data)

        line.set_ydata(data)
        ax.set_ylim(min(data) - 0.1, max(data) + 0.1)

    except Exception as e:
        print(f"Error: {e}")

    return line,

# interval in ms
ani = animation.FuncAnimation(fig, update, interval=1, blit=True, cache_frame_data=False)
plt.show()

ser.close()

