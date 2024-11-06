#ifndef WIFI_CONNECT_H
#define WIFI_CONNECT_H

#include <stdio.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "mqtt_client.h"
#include "driver/gpio.h"

#include "config.h"

void wifi_init(void);
extern esp_mqtt_client_handle_t mqtt_client;

#endif // WIFI_CONNECT_H