import serial
import numpy as np
import time
from selenium import webdriver
from selenium.webdriver.common.keys import Keys


serialPort = serial.Serial(port="COM10", baudrate=19200, parity=serial.PARITY_NONE,
                           bytesize=serial.EIGHTBITS, timeout=3, stopbits=serial.STOPBITS_ONE)

driver = webdriver.Chrome('C:\\Python\\Selenium\\chromedriver.exe')
driver.implicitly_wait(15)
driver.get('https://tuneform.com/tools/color/rgb-color-creator')

time.sleep(10)

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

    for i in range(1, 4):
        driver.find_element_by_xpath("//*[@id='app']/div[2]/main/div/div/div[2]/div/div[2]/div/div/div[2]/div/div[%d]/div/div/div/div/div[1]/div/input" % i).send_keys(Keys.CONTROL + 'A')
        driver.find_element_by_xpath("//*[@id='app']/div[2]/main/div/div/div[2]/div/div[2]/div/div/div[2]/div/div[%d]/div/div/div/div/div[1]/div/input" % i).send_keys('%d' % color[i-1])
        driver.find_element_by_xpath("//*[@id='app']/div[2]/main/div/div/div[2]/div/div[2]/div/div/div[2]/div/div[%d]/div/div/div/div/div[1]/div/input" % i).send_keys(Keys.RETURN)
