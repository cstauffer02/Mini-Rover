import pygame

pygame.init()
pygame.joystick.init()

if pygame.joystick.get_count() == 0:
    print("No joystick detected! Ensure your Xbox controller is connected via Bluetooth.")
    exit()

controller = pygame.joystick.Joystick(0)
controller.init()

print(f"Connected Controller: {controller.get_name()}")

while True:
    pygame.event.pump()  # Process events

    left_x = controller.get_axis(0)
    left_y = controller.get_axis(1)
    right_x = controller.get_axis(2)
    right_y = controller.get_axis(3)

    a_button = controller.get_button(0)
    b_button = controller.get_button(1)
    x_button = controller.get_button(2)
    y_button = controller.get_button(3)

    print(f"LeftX: {left_x:.2f}, LeftY: {left_y:.2f}, RightX: {right_x:.2f}, RightY: {right_y:.2f}")
    print(f"A: {a_button}, B: {b_button}, X: {x_button}, Y: {y_button}")