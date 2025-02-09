#ifndef WIFI_CONNECT_H
#define WIFI_CONNECT_H

#include <stdio.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <mqtt_client.h>

#include <wifi_provisioning/manager.h>
#include <wifi_provisioning/scheme_ble.h>

#include <driver/gpio.h>
#include <nvs_flash.h>
#include "config.h"
#include "credentials.h"
#include "web_server.h"
#include "hd44780.h"
#include "mdns.h"

#define BUTTON_GPIO GPIO_NUM_7 // Define o pino do botão
#define LED_WIFI    GPIO_NUM_4 // Define o pino do LED de status do Wi-Fi
#define LED_MQTT    GPIO_NUM_6 // Define o pino do LED de status do MQTT
#define CONFIG_EXAMPLE_PROV_MGR_MAX_RETRY_CNT 5

static const char *TAG = "wifi_provisioning";
extern esp_mqtt_client_handle_t mqtt_client;
extern hd44780_t lcd;

void wifi_init(void);

#endif // WIFI_CONNECT_H