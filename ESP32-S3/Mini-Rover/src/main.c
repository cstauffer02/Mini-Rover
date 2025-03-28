/* Author: Cameron Stauffer
** Last Edited: March 26, 2025
**
** Description: This program writes a sinewave to 6 srevo motors, phase-shifted
** by 30 degrees. The program serves as a starting point to what will eventually 
** control the servo motors of the rover's steering assembly.
*/

#include "esp_wifi.h"
#include "lwip/sockets.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pca9685.h"
#include "esp32wifi.h"

void udp_server_task(void *arg) {
    struct sockaddr_in server_addr, client_addr;
    char rx_buffer[128];
    socklen_t addr_len = sizeof(client_addr);

    // Create UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        ESP_LOGE("UDP", "Failed to create socket: errno %d", errno);
        vTaskDelete(NULL);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        ESP_LOGE("UDP", "Socket bind failed: errno %d", errno);
        close(sock);
        vTaskDelete(NULL);
    }

    ESP_LOGI("UDP", "UDP server listening on port %d", SERVER_PORT);

    while (1) {
        int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (len > 0) {
            rx_buffer[len] = 0;  // Null-terminate the received string
            ESP_LOGI("UDP", "Received: %s", rx_buffer);

            // Parse received angles
            int angles[NUM_SERVOS];
            if (sscanf(rx_buffer, "S,%d,%d,%d,%d,%d,%d", 
                    &angles[0], &angles[1], &angles[2], 
                    &angles[3], &angles[4], &angles[5]) == 6) {
                ESP_LOGI("UDP", "Parsed angles: %d, %d, %d, %d, %d, %d", 
                    angles[0], angles[1], angles[2],
                    angles[3], angles[4], angles[5]);
            } else {
                ESP_LOGE("UDP", "Failed to parse UDP command.");
            }
 
            // Set servo angles
            for (int i = 0; i < NUM_SERVOS; i++) {
                pca9685_set_servo_angle(i, angles[i]);
            }
        }
    }

    close(sock);
    vTaskDelete(NULL);
}

void app_main() {
    vTaskDelay(pdMS_TO_TICKS(5000)); // This delay allows time for the monitor to launch
    printf("\n\nStarting application...\n");
    i2c_master_init();
    printf("i2c master initialized\n");
    pca9685_init();
    wifi_init();
    print_ip_address();

    xTaskCreate(udp_server_task, "udp_server_task", 4096, NULL, 5, NULL);
}