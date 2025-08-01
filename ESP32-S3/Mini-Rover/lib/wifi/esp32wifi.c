#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "lwip/sockets.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp32wifi.h"

static const char *TAG = "Wi-Fi";

void wifi_init() {
    ESP_LOGI(TAG, "Initializing Wi-Fi\n");

    // Initialize Non-Volatile Storage and Network Interface
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();

    // Create default station (STA) network interface
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    if (sta_netif == NULL) {
        ESP_LOGE(TAG, "Failed to create default Wi-Fi station interface");
        return;
    }

    // Initialize Wi-Fi
    //esp_wifi_set_ps(WIFI_PS_NONE);  // Disable power-saving mode for lower latency
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    // Configure Wi-Fi with SSID and password
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);  // Set mode to Station (STA)
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();  // Start Wi-Fi
    esp_wifi_connect();  // Connect to configured SSID

    ESP_LOGI(TAG, "Wi-Fi Initialized\n");
}

void print_ip_address() {
    // Get the default Wi-Fi interface
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    
    if (netif == NULL) {
        ESP_LOGE("Wi-Fi", "Failed to get network interface.");
        return;
    }

    // Get the IP information
    esp_netif_ip_info_t ip_info;
    if (esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
        ESP_LOGI("Wi-Fi", "ESP32 IP Address: " IPSTR, IP2STR(&ip_info.ip));
    } else {
        ESP_LOGE("Wi-Fi", "Failed to retrieve IP address.");
    }
}