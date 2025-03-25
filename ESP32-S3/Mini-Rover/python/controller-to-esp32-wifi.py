import pygame
import socket
import time

# ESP32 Wi-Fi settings
ESP32_IP = "192.168.1.92"  # Update to the ESP32's actual IP
ESP32_PORT = 8080           # Port the ESP32 server listens on

# Initialize UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

pygame.init()
pygame.joystick.init()

if pygame.joystick.get_count() == 0:
    print("No joystick detected! Ensure your Xbox controller is connected via Bluetooth.")
    exit()

controller = pygame.joystick.Joystick(0)
controller.init()

print(f"Xbox Controller Connected: {controller.get_name()}")

def send_command(command):
    print(f"Sending: {command}")
    sock.sendto(command.encode(), (ESP32_IP, ESP32_PORT))

running = True

while running:
    # Process events
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        if event.type == pygame.JOYBUTTONDOWN:
            if event.button == 1:  # 'B' button on Xbox controllers
                print("B button pressed! Exiting script.")
                running = False

    # Read joystick axes
    left_x = controller.get_axis(0)
    left_y = controller.get_axis(1)
    right_x = controller.get_axis(2)
    right_y = controller.get_axis(3)

    # Convert joystick values (-1 to 1) to servo angles (0 to 180)
    servo_1_angle = int((left_x + 1) * 90)
    servo_2_angle = int((left_y + 1) * 90)
    servo_3_angle = int((right_x + 1) * 90)
    servo_4_angle = int((right_y + 1) * 90)

    # Create a command string
    command = f"{servo_1_angle},{servo_2_angle},{servo_3_angle},{servo_4_angle}"
    send_command(command)

    time.sleep(0.05)

pygame.quit()
sock.close()
print("Program exited successfully.")