/* Author: Cameron Stauffer
** Last Edited: April 4, 2025
**
** Description: This program writes a sinewave to 6 srevo motors, phase-shifted
** by 30 degrees. The program serves as a starting point to what will eventually 
** control the servo motors of the rover's steering assembly.
*/

#include <stdio.h>
#include "esp_wifi.h"
#include "esp_log.h"
#include "lwip/sockets.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pca9685.h"
#include "esp32wifi.h"
#include "l298n.h"

static const char *TAG = "MAIN";

l298n_t board1, board2, board3;

void udp_server_task(void *arg) {
    struct sockaddr_in server_addr, client_addr;
    char rx_buffer[128];
    int speedVal = 0;
    int16_t absSpeed = 0;
    socklen_t addr_len = sizeof(client_addr);

    // Create UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        ESP_LOGE("UDP", "Failed to create socket: errno %d", errno);
        speedVal = 0;
        vTaskDelete(NULL);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        ESP_LOGE(TAG, "Socket bind failed: errno %d", errno);
        speedVal = 0;
        close(sock);
        vTaskDelete(NULL);
    }

    ESP_LOGI(TAG, "UDP server listening on port %d", SERVER_PORT);

    while (1) {
        int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (len > 0) {
            rx_buffer[len] = 0;  // Null-terminate the received string
            ESP_LOGI(TAG, "Received: %s", rx_buffer);

            if (rx_buffer[0] == 'S'){
                // Parse received angles
                int angles[NUM_SERVOS];
                if (sscanf(rx_buffer, "S,%d,%d,%d,%d,%d,%d", 
                        &angles[0], &angles[1], &angles[2], 
                        &angles[3], &angles[4], &angles[5]) == NUM_SERVOS) {
                    ESP_LOGI(TAG, "Parsed angles: %d, %d, %d, %d, %d, %d", 
                        angles[0], angles[1], angles[2],
                        angles[3], angles[4], angles[5]);
                } else {
                    ESP_LOGE(TAG, "Failed to parse UDP command.");
                }

                // Set servo angles
                for (int i = 0; i < NUM_SERVOS; i++) {
                    pca9685_set_servo_angle(i, angles[i]);
                }// end for
            }// end if
            else if (rx_buffer[0] == 'M'){
                if (sscanf(rx_buffer, "M,%d", &speedVal) == 1){
                    ESP_LOGI(TAG, "Parsed motor speed: %d", speedVal);
                    motorDirection_t dir;
                    absSpeed = 0;
                    if (speedVal > 0){
                        dir = MOTOR_FORWARD;
                        absSpeed = speedVal;
                    } else if (speedVal < 0){
                        dir = MOTOR_REVERSE;
                        absSpeed = -speedVal;
                    } else{
                        dir = MOTOR_STOP;
                        absSpeed = 0;
                    }
                    if (absSpeed > 255){
                        absSpeed = 255;
                    }
                    // Apply the same drive command to all motors on the three boards.
                    l298n_set_motor(&board1, 0, dir, absSpeed);
                    l298n_set_motor(&board1, 1, dir, absSpeed);
                    l298n_set_motor(&board2, 0, dir, absSpeed);
                    l298n_set_motor(&board2, 1, dir, absSpeed);
                    l298n_set_motor(&board3, 0, dir, absSpeed);
                    l298n_set_motor(&board3, 1, dir, absSpeed);
                } else {
                    ESP_LOGE(TAG, "Failed to parse motor command.");
                } 
            } else {
                ESP_LOGE(TAG, "Unknown command received.");
            }
        }// end if
    }// end while

    close(sock);
    vTaskDelete(NULL);
}

void app_main() {
    vTaskDelay(pdMS_TO_TICKS(5000)); // This delay allows time for the monitor to launch
    printf("\n\nStarting application...\n");
    i2c_master_init();
    printf("i2c master initialized\n");
    pca9685_init();
    init_motor_controllers(&board1, &board2, &board3);
    wifi_init_softap();
    print_softap_ip_udp();

    xTaskCreate(udp_server_task, "udp_server_task", 4096, NULL, 5, NULL);
}