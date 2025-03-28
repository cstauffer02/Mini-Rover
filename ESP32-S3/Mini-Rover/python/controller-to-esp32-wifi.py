import pygame
import socket
import time
import math

WHEELBASE = 16.0  # in cm
TRACK_WIDTH = 2.4  # in cm
R_MIN = 20.0  # Tightest turn radius (cm)
R_MAX = 200.0  # Largest turn radius (cm)

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

# Sends the command variable to the socket
def send_command(command):
    print(f"Sending: {command}")
    sock.sendto(command.encode(), (ESP32_IP, ESP32_PORT))

# Binds the angle to the min and max values
def clamp(angle, min_angle=45, max_angle=135):  # 45 < theta < 135
    return max(min_angle, min(max_angle, angle)) 

# Determines Ackerman angles of steering assembly servos
def ackerman_angles(steeringInput):
    if steeringInput == 0:
        return [90, 90, 90, 90, 90, 90]  # Neutral positions for all six servos

    R = R_MIN + (R_MAX - R_MIN) * (1 - abs(steeringInput))  # Scale joystick input to radius

    # Calculate Ackerman angles
    front_inner_angle = math.degrees(math.atan(WHEELBASE / (R - TRACK_WIDTH / 2)))
    front_outer_angle = math.degrees(math.atan(WHEELBASE / (R + TRACK_WIDTH / 2)))
    rear_inner_angle = -front_inner_angle  # Rear wheels countersteer
    rear_outer_angle = -front_outer_angle


    if steeringInput > 0:  # Turning right
        # Combine all six servo angles into a single list
        return [
            clamp(90 + front_inner_angle),  # Front-left
            clamp(90 + front_outer_angle),  # Front-right
            clamp(90),                      # Middle-left stays neutral
            clamp(90),                      # Middle-right stays neutral
            clamp(90 + rear_inner_angle),   # Rear-left
            clamp(90 + rear_outer_angle)    # Rear-right
        ]
    else:  # Turning left
        # Combine all six servo angles into a single list
        return [
            clamp(90 - front_outer_angle),  # Front-left
            clamp(90 - front_inner_angle),  # Front-right
            clamp(90),                      # Middle-left stays neutral
            clamp(90),                      # Middle-right stays neutral
            clamp(90 - rear_outer_angle),   # Rear-left
            clamp(90 - rear_inner_angle)    # Rear-right
        ]

running = True

while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        if event.type == pygame.JOYBUTTONDOWN:
            if event.button == 1:  # 'B' button on Xbox controllers
                print("B button pressed! Exiting script.")
                running = False

    # Read joystick axes
    steeringInput = controller.get_axis(0)  # Left thumbstick
    steeringAngles = ackerman_angles(steeringInput)
    
    command = f"S,{int(round(steeringAngles[0]))},{int(round(steeringAngles[1]))},{int(round(steeringAngles[2]))},{int(round(steeringAngles[3]))},{int(round(steeringAngles[4]))},{int(round(steeringAngles[5]))}"
    send_command(command)

    time.sleep(0.05)

pygame.quit()
sock.close()
print("Program exited successfully.")