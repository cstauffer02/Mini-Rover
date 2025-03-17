/* Author: Cameron Stauffer
** Last Edited: March 15, 2025
**
** Description: This program writes a sinewave to 6 srevo motors, phase-shifted
** by 30 degrees. The program serves as a starting point to what will eventually 
** control the servo motors of the rover's steering assembly.
*/

#include <stdio.h>
#include "pca9685.h"
#include "i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/i2c.h"
#include "driver/i2c_master.h"

static const char *TAG = "PCA9685";

void app_main() {

    printf("Starting application...\n");
    vTaskDelay(pdMS_TO_TICKS(2000));

    esp_log_level_set("*", ESP_LOG_DEBUG);
    ESP_LOGI(TAG, "Initializing I2C and PCA9685");
    i2c_master_init();
    pca9685_init();

    ESP_LOGI(TAG, "Starting sinewave servo task");
    xTaskCreate(sinewave_servo_task, "sinewave_servo", 4096, NULL, 5, NULL);

}