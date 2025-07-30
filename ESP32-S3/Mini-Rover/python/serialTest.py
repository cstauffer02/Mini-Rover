import serial

esp32 = serial.Serial(port="COM3", baudrate=115200, timeout=1)

while True:
    if esp32.in_waiting > 0:
        line = esp32.readline().decode().strip()
        print("ESP32 says:", line)
