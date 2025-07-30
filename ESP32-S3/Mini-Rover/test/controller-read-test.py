import pygame
import time

pygame.init()
pygame.joystick.init()

if pygame.joystick.get_count() == 0:
    print("No joystick detected! Ensure your Xbox controller is connected via Bluetooth.")
    exit()

controller = pygame.joystick.Joystick(0)
controller.init()

print(f"Connected Controller: {controller.get_name()}")

running = True
while running:
    pygame.event.pump()  # Process events

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        if event.type == pygame.JOYBUTTONDOWN:
            if event.button == 1:  # 'B' button on Xbox controllers
                print("B button pressed! Exiting script.")
                running = False

    left_x = controller.get_axis(0)
    left_y = controller.get_axis(1)
    right_x = controller.get_axis(2)
    right_y = controller.get_axis(3)
    lt = controller.get_axis(4)
    rt = controller.get_axis(5)

    a_button = controller.get_button(0)
    b_button = controller.get_button(1)
    x_button = controller.get_button(2)
    y_button = controller.get_button(3)

    print(f"LeftX: {left_x:.2f}, LeftY: {left_y:.2f}, RightX: {right_x:.2f}, RightY: {right_y:.2f}")
    print(f"A: {a_button}, B: {b_button}, X: {x_button}, Y: {y_button}")
    print(f"Left Trigger: {lt:.4f}, Right Trigger: {rt:.4f}")
    time.sleep(0.25)

