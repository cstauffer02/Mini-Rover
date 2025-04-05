#ifndef L298N_H
#define L298N_H

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"

typedef enum{
    MOTOR_STOP = 0,
    MOTOR_FORWARD,
    MOTOR_REVERSE,
    MOTOR_BRAKE
} motorDirection_t;

typedef struct {
    // Motor A
    gpio_num_t in1_pinA;
    gpio_num_t in2_pinA;
    gpio_num_t enA_pin;
    int enA_channel;

    // Motor B
    gpio_num_t in1_pinB;
    gpio_num_t in2_pinB;
    gpio_num_t enB_pin;
    int enB_channel;
} l298n_t;

esp_err_t l298n_init(l298n_t *dev);
esp_err_t l298n_set_motor(l298n_t *dev, int motor, motorDirection_t direction, int16_t speed);
void init_motor_controllers(l298n_t *b1, l298n_t *b2, l298n_t *b3);

#endif