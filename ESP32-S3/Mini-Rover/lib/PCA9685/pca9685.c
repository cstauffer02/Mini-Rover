#include "PCA9685.h"
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "PCA9685";

/* Write to PCA9685 register */
esp_err_t pca9685_write(uint8_t reg, uint8_t value){
    uint8_t data[2] = {reg, value};
    return i2c_master_write_to_device(I2C_MASTER_NUM, I2C_PCA9685_ADDR, data, 2, 1000/portTICK_PERIOD_MS);
}

esp_err_t pca9685_read(uint8_t reg, uint8_t *data) {
    return i2c_master_read_from_device(I2C_MASTER_NUM, I2C_PCA9685_ADDR, data, 1, 1000 / portTICK_PERIOD_MS);
}

/* Function to initialize I2C */
void i2c_master_init(){
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .master.clk_speed = I2C_FREQ,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

void pca9685_init(){
    ESP_LOGI("PCA9685", "Initializing PCA9685");

    // Put PCA9685 into sleep mode to set prescaler (set MODE1 to 0x10)
    pca9685_write(PCA9685_MODE1, 0x10);
    vTaskDelay(pdMS_TO_TICKS(10)); // arbitrary delay to ensure register is written to

    // Read back the MODE1 register
    uint8_t mode1 = 0;
    esp_err_t ret = pca9685_read(PCA9685_MODE1, &mode1);
    if (ret == ESP_OK) {
        ESP_LOGI("PCA9685", "MODE1 register value: 0x%02X", mode1);
    } else {
        ESP_LOGE("PCA9685", "Failed to read MODE1 register");
    }

    // Set the prescaler for 50Hz PWM frequency
    uint8_t prescale = (uint8_t)(25000000.0 / (4096 * 50) - 1);  // datasheet page 25
    if (pca9685_write(PCA9685_PRESCALE, prescale) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write PCA9685 prescaler");
        return;
    }

    // Wake up PCA9685 and enable auto-increment
    pca9685_write(PCA9685_MODE1, 0xA1); // datasheet page 14 for Mode Register 1 values
    vTaskDelay(pdMS_TO_TICKS(10)); // arbitrary delay to ensure register is written to

    ESP_LOGI(TAG, "PCA9685 initialized.");
    vTaskDelay(pdMS_TO_TICKS(10)); // arbitrary delay to ensure register is written to
}

void pca9685_set_servo_angle(uint8_t channel, float angle) {
    // Servo value were found experimentally.
    uint16_t pulseMin = SERVO_MIN;  // 1ms pulse width for 0 degrees
    uint16_t pulseMax = SERVO_MAX;  // 2ms pulse width for 180 degrees
    uint16_t pulse = pulseMin + (angle / 180.0) * (pulseMax - pulseMin);

    ESP_LOGI(TAG, "Setting servo channel %d to angle %.2f (pulse: %d)", channel, angle, pulse);

    uint8_t reg = PCA9685_LED0_ON_L + 4 * channel;
    uint8_t data[4] = {
        0x00,               // ON time low byte (always 0)
        0x00,               // ON time high byte (always 0)
        pulse & 0xFF,       // OFF time low byte
        (pulse >> 8) & 0xFF // OFF time high byte
    };

    // Debugging the pulse width value
    ESP_LOGI("PCA9685", "Setting servo %d to angle %.2f, pulse width: %d", channel, angle, pulse);

    // Send register address + data in one transaction
    uint8_t write_buffer[5] = {reg, data[0], data[1], data[2], data[3]};
    esp_err_t ret = i2c_master_write_to_device(I2C_MASTER_NUM, I2C_PCA9685_ADDR, write_buffer, sizeof(write_buffer), 1000 / portTICK_PERIOD_MS);

    if (ret != ESP_OK) {
        ESP_LOGE("PCA9685", "Failed to write PWM data for servo %d", channel);
    }
}

void set_full_pwm(uint8_t channel) {
    uint8_t reg = PCA9685_LED0_ON_L + 4 * channel; // LEDx_ON_L register
    uint8_t data[5] = {
        reg,    // Register address
        0x00,   // ON time low byte (always 0)
        0x00,   // ON time high byte (always 0)
        0xFF,   // OFF time low byte (0x0FFF for full duty cycle)
        0x0F    // OFF time high byte
    };

    esp_err_t ret = i2c_master_write_to_device(I2C_MASTER_NUM, I2C_PCA9685_ADDR, data, sizeof(data), 1000 / portTICK_PERIOD_MS);

    if (ret != ESP_OK) {
        ESP_LOGE("PCA9685", "Failed to set full PWM on channel %d", channel);
    } else {
        ESP_LOGI("PCA9685", "Set channel %d to full PWM (always HIGH)", channel);
    }
}

uint8_t read_pca9685_mode1() {
    uint8_t mode1;
    i2c_master_write_to_device(I2C_MASTER_NUM, I2C_PCA9685_ADDR, (uint8_t[]){PCA9685_MODE1}, 1, 1000 / portTICK_PERIOD_MS);
    i2c_master_read_from_device(I2C_MASTER_NUM, I2C_PCA9685_ADDR, &mode1, 1, 1000 / portTICK_PERIOD_MS);
    return mode1;
}

void force_wake_up(){
    uint8_t mode1;
    pca9685_read_register(I2C_MASTER_NUM, I2C_PCA9685_ADDR, PCA9685_MODE1, &mode1);
    mode1 &= ~(1 << 4); // Clear sleep bit
    pca9685_write_register(I2C_MASTER_NUM, I2C_PCA9685_ADDR, PCA9685_MODE1, mode1);
    vTaskDelay(pdMS_TO_TICKS(10)); // Give time to restart PWM generator
    printf("PCA9685 MODE1 register after wake-up: 0x%02X\n", mode1);
}

esp_err_t pca9685_read_register(i2c_port_t i2c_num, uint8_t device_addr, uint8_t reg, uint8_t *data) {
    return i2c_master_write_read_device(i2c_num, device_addr, &reg, 1, data, 1, 1000 / portTICK_PERIOD_MS);
}

esp_err_t pca9685_write_register(i2c_port_t i2c_num, uint8_t device_addr, uint8_t reg, uint8_t value) {
    uint8_t data[2] = {reg, value};
    return i2c_master_write_to_device(i2c_num, device_addr, data, 2, 1000 / portTICK_PERIOD_MS);
}

void sinewave_servo_task(void *arg) {
    const float frequency = 2 * M_PI / (WAVE_PERIOD_MS / 1000.0); 
    const float amplitude = (SERVO_MAX_ANGLE - SERVO_MIN_ANGLE) / 2.0;
    const float midpoint = (SERVO_MAX_ANGLE + SERVO_MIN_ANGLE) / 2.0;

    int64_t start_time = esp_timer_get_time();  // start time in microseconds

    while (1) { 
        int64_t current_time = esp_timer_get_time();  // current time in microseconds
        int elapsed_time_ms = (current_time - start_time) / 1000;  // convert to milliseconds

        for (int i = 0; i < 6; i++) { // apply phase shift to each servo
            float phase_shift = (i * 30.0) * (M_PI / 180.0); 
            float angle = midpoint + amplitude * sin(frequency * (elapsed_time_ms / 1000.0) + phase_shift);
            pca9685_set_servo_angle(i, angle);
        }

        vTaskDelay(pdMS_TO_TICKS(5)); 
    }
}