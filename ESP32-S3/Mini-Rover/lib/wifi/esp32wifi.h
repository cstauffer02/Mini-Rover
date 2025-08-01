#ifndef ESP32WIFI_H
#define ESP32WIFI_H

// esp32wifi.c
#define WIFI_SSID "HyruleCastle"
#define WIFI_PASS "RubyRubyDoo122519"
#define SERVER_PORT 8080

// esp32wifiap.c
#define WIFI_AP_SSID "Eagle-5"
#define WIFI_AP_PASS "12345druidia"
#define WIFI_CHANNEL 1
#define MAX_STA_CONN 1

// esp32wifi.c
void wifi_init();
void print_ip_address();

// esp32wifiap.c
void wifi_init_softap(void);
void print_softap_ip_udp();

#endif