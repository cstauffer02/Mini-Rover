#ifndef PCA9685_H
#define PCA9685_H

#include "driver/i2c.h"
#include "esp_log.h"

#define I2C_MASTER_SCL_IO 17
#define I2C_MASTER_SDA_IO 18
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_FREQ 100000
#define I2C_PCA9685_ADDR 0x40

#define PCA9685_MODE1 0x00
#define PCA9685_PRESCALE 0xFE
#define PCA9685_LED0_ON_L 0x06

#define SERVO_MIN 100
#define SERVO_MAX 500
#define SERVO_MIN_ANGLE 0
#define SERVO_MAX_ANGLE 180
#define WAVE_PERIOD_MS 5000 // 5 seconds for sinewave period
#define SERVO_STEP 50

#define NUM_SERVOS 6
#define SERVO1 0
#define SERVO2 1
#define SERVO3 2
#define SERVO4 3
#define SERVO5 4
#define SERVO6 5

void i2c_master_init();
void pca9685_init();
void pca9685_set_servo_angle(uint8_t channel, float angle);
void set_full_pwm(uint8_t channel);
uint8_t read_pca9685_mode1();
void force_wake_up();
esp_err_t pca9685_read_register(i2c_port_t i2c_num, uint8_t device_addr, uint8_t reg, uint8_t *data);
esp_err_t pca9685_write_register(i2c_port_t i2c_num, uint8_t device_addr, uint8_t reg, uint8_t value);
void sinewave_servo_task(void *arg);

#endif