import pygame
import socket
import time
import math
import threading

# Configuration variables
WHEELBASE = 16.0  # in cm
TRACK_WIDTH = 2.4  # in cm
R_MIN = 20.0  # Tightest turn radius (cm)
R_MAX = 200.0  # Largest turn radius (cm)
THRESHOLD = 0.1  # Small threshold to avoid accidental trigger activation
ESP32_IP = "192.168.4.1"  # Update to the ESP32's actual IP
ESP32_PORT = 8080           # Port the ESP32 server listens on

# Initialize UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Initialize Pygame and the Xbox controller
pygame.init()
pygame.joystick.init()

if pygame.joystick.get_count() == 0:
    print("No joystick detected! Ensure your Xbox controller is connected via Bluetooth.")
    exit()

controller = pygame.joystick.Joystick(0)
controller.init()

print(f"Xbox Controller Connected: {controller.get_name()}")

# Functions
def send_command(command):
    print(f"Sending: {command}")
    sock.sendto(command.encode(), (ESP32_IP, ESP32_PORT))

def clamp(angle, min_angle=45, max_angle=135):  # 45 < theta < 135
    return max(min_angle, min(max_angle, angle))

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
        return [
            clamp(90 + front_inner_angle),  # Front-left
            clamp(90 + front_outer_angle),  # Front-right
            clamp(90),                      # Middle-left stays neutral
            clamp(90),                      # Middle-right stays neutral
            clamp(90 + rear_inner_angle),   # Rear-left
            clamp(90 + rear_outer_angle)    # Rear-right
        ]
    else:  # Turning left
        return [
            clamp(90 - front_outer_angle),  # Front-left
            clamp(90 - front_inner_angle),  # Front-right
            clamp(90),                      # Middle-left stays neutral
            clamp(90),                      # Middle-right stays neutral
            clamp(90 - rear_outer_angle),   # Rear-left
            clamp(90 - rear_inner_angle)    # Rear-right
        ]

def normalize_triggers(rtRaw, ltRaw):
    rt = (rtRaw + 1) / 2.0
    lt = (ltRaw + 1) / 2.0
    return [rt, lt]

# Thread for Motor Commands
def motor_thread():
    while running:
        rtRaw = controller.get_axis(5)
        ltRaw = controller.get_axis(4)

        rt, lt = normalize_triggers(rtRaw, ltRaw)

        if rt < THRESHOLD and lt < THRESHOLD:
            motorSpeed = 0
        else:
            motorSpeed = int((rt - lt) * 255)

        driveCommand = f"M,{int(round(motorSpeed))}"
        send_command(driveCommand)
        time.sleep(0.01)  # 10ms delay for smoother updates

# Thread for Servo Commands
def servo_thread():
    while running:
        steeringInput = controller.get_axis(0)  # Left thumbstick
        steeringAngles = ackerman_angles(steeringInput)

        steerCommand = f"S,{int(round(steeringAngles[0]))},{int(round(steeringAngles[1]))},{int(round(steeringAngles[2]))},{int(round(steeringAngles[3]))},{int(round(steeringAngles[4]))},{int(round(steeringAngles[5]))}"
        send_command(steerCommand)
        time.sleep(0.05)  # 50ms delay for smoother updates

# Main Program (Event Handling)
running = True

# Start threads for motor and servo commands
motor_thread_obj = threading.Thread(target=motor_thread)
servo_thread_obj = threading.Thread(target=servo_thread)

motor_thread_obj.start()
servo_thread_obj.start()

# Event Handling in Main Thread
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        if event.type == pygame.JOYBUTTONDOWN:
            print(f"Button {event.button} pressed")
            if event.button == 1:  # B button
                print("B button pressed. Exiting.")
                running = False

# Wait for threads to finish
motor_thread_obj.join()
servo_thread_obj.join()

pygame.quit()
sock.close()
print("Program exited successfully.")