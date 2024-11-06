#include "wifi_connect.h"

static void mqtt_init(void);
esp_mqtt_client_handle_t mqtt_client;

void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        printf("Reconectando ao Wi-Fi...\n");
        gpio_set_level(GPIO_NUM_4, 0); 
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        printf("Wi-Fi conectado!\n");
        gpio_set_level(GPIO_NUM_4, 1); 
        mqtt_init();
    }
}
void wifi_init(void) {
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS
        },
    };
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
}

void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    switch (event_id) {
        case MQTT_EVENT_CONNECTED:
            printf("MQTT conectado!\n");
            break;
        case MQTT_EVENT_DISCONNECTED:
            printf("MQTT desconectado!\n");
            break;
        default:
            break;
    }
}

static void mqtt_init(void) {
    esp_mqtt_client_config_t mqtt_cfg = {
       .broker.address.uri = MQTT_BROKER_URI, 
       .broker.address.port = 1883,
       .credentials.username = username_ha,
       .credentials.authentication.password = password_ha,
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
}