import serial
import numpy as np


serialPort = serial.Serial(port="COM10", baudrate=19200, parity=serial.PARITY_NONE,
                           bytesize=serial.EIGHTBITS, timeout=3, stopbits=serial.STOPBITS_ONE)


while 1:
    color_str = serialPort.readline().decode('ascii')
    color_str = color_str.strip('\n')

    # Convert to RGB 255 value
    color = np.array(list(map(int, color_str.split(',')))[1:])
    color[0] = color[0] / 2770 * 255
    color[1] = color[1] / 2060 * 255
    color[2] = color[2] / 1410 * 255

    # 7200, 3500, 2600, 1800
    # 17100, 7800, 6100, 4400

    # 2770, 2060, 1410

    print(color)
