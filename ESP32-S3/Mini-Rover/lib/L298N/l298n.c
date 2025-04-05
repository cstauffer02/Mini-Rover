#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "l298n.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "esp_rom_gpio.h"

static const char *TAG = "L298N";

// Use LEDC timer 0 in high-speed mode with 8-bit resolution (0–255 duty)
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_8_BIT  
#define LEDC_FREQUENCY          5000              // 5 kHz
#define gpio_pad_select_gpio esp_rom_gpio_pad_select_gpio

// Helper function to configure one LEDC channel for a given pin
static esp_err_t setup_ledc_channel(int channel, gpio_num_t gpio_pin) {
    ledc_channel_config_t ledc_channel = {
        .channel    = channel,
        .duty       = 0,
        .gpio_num   = gpio_pin,
        .speed_mode = LEDC_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_TIMER,
    };
    return ledc_channel_config(&ledc_channel);
}

// Configure the LEDC timer (will be common for all channels) 
static esp_err_t setup_ledc_timer() {
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK,
    };
    return ledc_timer_config(&ledc_timer);
}

esp_err_t l298n_init(l298n_t *dev){
    if (dev == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    // Configure Motor A direction pins
    gpio_pad_select_gpio(dev->in1_pinA);
    gpio_pad_select_gpio(dev->in2_pinA);
    gpio_set_direction(dev->in1_pinA, GPIO_MODE_OUTPUT);
    gpio_set_direction(dev->in2_pinA, GPIO_MODE_OUTPUT);

    // Configure Motor B direction pins
    gpio_pad_select_gpio(dev->in1_pinB);
    gpio_pad_select_gpio(dev->in2_pinB);
    gpio_set_direction(dev->in1_pinB, GPIO_MODE_OUTPUT);
    gpio_set_direction(dev->in2_pinB, GPIO_MODE_OUTPUT);

    // Configure enable pins (PWM) for Motor A and Motor B
    gpio_pad_select_gpio(dev->enA_pin);
    gpio_set_direction(dev->enA_pin, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(dev->enB_pin);
    gpio_set_direction(dev->enB_pin, GPIO_MODE_OUTPUT);

    // Setup LEDC timer (call once)
    ESP_ERROR_CHECK(setup_ledc_timer());

    // Setup LEDC channels for enable pins
    ESP_ERROR_CHECK(setup_ledc_channel(dev->enA_channel, dev->enA_pin));
    ESP_ERROR_CHECK(setup_ledc_channel(dev->enB_channel, dev->enB_pin));

    // Initialize motors to a stopped state
    //l298n_set_motor(dev, 0, MOTOR_STOP, 0);
    //l298n_set_motor(dev, 1, MOTOR_STOP, 0);

    ESP_LOGI(TAG, "L298N controller initialized.");
    return ESP_OK;
}

void init_motor_controllers(l298n_t *b1, l298n_t *b2, l298n_t *b3){
    // Create three L298N board instances with your chosen pin assignments.
    // Adjust the pin numbers and LEDC channel assignments as needed.

    // Initialize Board 1 for two motors
    *b1 = (l298n_t){
        .in1_pinA = GPIO_NUM_4,
        .in2_pinA = GPIO_NUM_5,
        .enA_pin    = GPIO_NUM_6,
        .enA_channel= 0,  // LEDC channel 0

        .in1_pinB = GPIO_NUM_7,
        .in2_pinB = GPIO_NUM_15,
        .enB_pin    = GPIO_NUM_16,
        .enB_channel= 1,  // LEDC channel 1
    };

    // Initialize Board 2 for two motors
    *b2 = (l298n_t){
        .in1_pinA = GPIO_NUM_9,
        .in2_pinA = GPIO_NUM_10,
        .enA_pin    = GPIO_NUM_11,
        .enA_channel= 2,  // LEDC channel 2

        .in1_pinB = GPIO_NUM_12,
        .in2_pinB = GPIO_NUM_13,
        .enB_pin    = GPIO_NUM_14,
        .enB_channel= 3,  // LEDC channel 3
    };

    // Initialize Board 3 for two motors
    *b3 = (l298n_t){
        .in1_pinA = GPIO_NUM_1,
        .in2_pinA = GPIO_NUM_2,
        .enA_pin    = GPIO_NUM_42,
        .enA_channel= 4,  // LEDC channel 4

        .in1_pinB = GPIO_NUM_41,
        .in2_pinB = GPIO_NUM_40,
        .enB_pin    = GPIO_NUM_39,
        .enB_channel= 5,  // LEDC channel 5
    };

    // Initialize each L298N board.
    ESP_LOGI(TAG, "Initializing motor boards...");
    l298n_init(b1);
    l298n_init(b2);
    l298n_init(b3);
    ESP_LOGI(TAG, "Motor boards initialized.");
}


esp_err_t l298n_set_motor(l298n_t *dev, int motor, motorDirection_t direction, int16_t speed){
    if (dev == NULL || (motor != 0 && motor != 1)) {
        return ESP_ERR_INVALID_ARG;
    }

    // Select pins and LEDC channel for the specified motor.
    gpio_num_t in1_pin, in2_pin;
    int en_channel;
    if (motor == 0) {
        in1_pin = dev->in1_pinA;
        in2_pin = dev->in2_pinA;
        en_channel = dev->enA_channel;
    } else {
        in1_pin = dev->in1_pinB;
        in2_pin = dev->in2_pinB;
        en_channel = dev->enB_channel;
    }

    // Set the motor direction pins according to desired direction.
    switch(direction) {
        case MOTOR_FORWARD:
            gpio_set_level(in1_pin, 1);
            gpio_set_level(in2_pin, 0);
            break;
        case MOTOR_REVERSE:
            gpio_set_level(in1_pin, 0);
            gpio_set_level(in2_pin, 1);
            break;
        case MOTOR_BRAKE:
            gpio_set_level(in1_pin, 1);
            gpio_set_level(in2_pin, 1);
            break;
        case MOTOR_STOP:
        default:
            gpio_set_level(in1_pin, 0);
            gpio_set_level(in2_pin, 0);
            break;
    }

    // Clamp speed to maximum (0-255 with 8-bit resolution)
    if (speed > 255) {
        speed = 255;
    }

    // Set PWM duty (speed) for the given enable channel.
    int pwmDC = 0;
    esp_err_t err = ledc_set_duty(LEDC_MODE, en_channel, speed);
    if (err == ESP_OK) {
        err = ledc_update_duty(LEDC_MODE, en_channel);
    }
    if (err == ESP_OK) {
        pwmDC = ledc_get_duty(LEDC_MODE, en_channel);
        ESP_LOGI("PWM_CHECK", "Channel %d duty is %d", en_channel, pwmDC);
        ESP_LOGI(TAG, "Set motor %d: direction %d, speed %d, PWM %d on LEDC channel %d", motor, direction, speed, pwmDC, en_channel);
    }
    return err;
}


/*
// L298N Task
void motor_control_task(void *arg){
    while (1) {
        ESP_LOGI(TAG, "Running motors...");

        // For demonstration purposes, run a simple pattern:
        // Board 1: Motor A forward at speed 200, Motor B reverse at speed 180
        l298n_set_motor(&board1, 0, MOTOR_FORWARD, 200);
        l298n_set_motor(&board1, 1, MOTOR_REVERSE, 180);
        
        // Board 2: Both motors forward at speed 220
        l298n_set_motor(&board2, 0, MOTOR_FORWARD, 220);
        l298n_set_motor(&board2, 1, MOTOR_FORWARD, 220);
        
        // Board 3: Both motors reverse at speed 150
        l298n_set_motor(&board3, 0, MOTOR_REVERSE, 150);
        l298n_set_motor(&board3, 1, MOTOR_REVERSE, 150);
        
        vTaskDelay(pdMS_TO_TICKS(2000));  // Run for 2 seconds

        // Then stop all motors
        l298n_set_motor(&board1, 0, MOTOR_STOP, 0);
        l298n_set_motor(&board1, 1, MOTOR_STOP, 0);
        l298n_set_motor(&board2, 0, MOTOR_STOP, 0);
        l298n_set_motor(&board2, 1, MOTOR_STOP, 0);
        l298n_set_motor(&board3, 0, MOTOR_STOP, 0);
        l298n_set_motor(&board3, 1, MOTOR_STOP, 0);
        
        vTaskDelay(pdMS_TO_TICKS(2000));  // Stop for 2 seconds
    }
}
*/