#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mqtt_client.h"

#include "esp_log.h"
#include "driver/i2c.h"
#include "pzem004tv3.h"
#include "wifi_connect.h"

/* @brief Set ESP32  Serial Configuration */
pzem_setup_t pzConf =
{
    .pzem_uart   = UART_NUM_1,              /*  <== Specify the UART you want to use, UART_NUM_0, UART_NUM_1, UART_NUM_2 (ESP32 specific) */
    .pzem_rx_pin = GPIO_NUM_18,             /*  <== GPIO for RX */
    .pzem_tx_pin = GPIO_NUM_17,             /*  <== GPIO for TX */
    .pzem_addr   = PZ_DEFAULT_ADDRESS,      /*  If your module has a different address, specify here or update the variable in pzem004tv3.h */
};

static const char * TAG = "APP_MAIN";
TaskHandle_t PMonTHandle = NULL;
_current_values_t pzValues;            /* Measured values */

void PMonTask( void * pz );

void app_main()
{
    // Configurar o GPIOs
    gpio_set_direction(GPIO_NUM_5, GPIO_MODE_OUTPUT); // Rele
    gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT); // led wifi

    nvs_flash_init();
    wifi_init();

    xTaskCreate( PMonTask, "PowerMon", ( 512 * 8 ), NULL, tskIDLE_PRIORITY, &PMonTHandle );
}

void PMonTask( void * pz )
{
    PzemInit( &pzConf );
    while(1)
    {        
        if (mqtt_client != NULL && PzemGetValues( &pzConf, &pzValues ) == true) {
            printf( "Vrms: %.1fV - Irms: %.3fA - P: %.1fW - E: %.2fWh\n", pzValues.voltage, pzValues.current, pzValues.power, pzValues.energy );
            printf( "Freq: %.1fHz - PF: %.2f\n", pzValues.frequency, pzValues.pf );
            
            char message[20];
            snprintf(message, sizeof(message), "%.2f", pzValues.voltage);
            esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC_VOLTAGE, message, 0, 1, 0);
            snprintf(message, sizeof(message), "%.2f", pzValues.current);
            esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC_CURRENT, message, 0, 1, 0);
            snprintf(message, sizeof(message), "%.2f", pzValues.power);
            esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC_POWER, message, 0, 1, 0);
            snprintf(message, sizeof(message), "%.2f", pzValues.energy);
            esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC_ENERGY, message, 0, 1, 0);
            snprintf(message, sizeof(message), "%.2f", pzValues.frequency);
            esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC_FREQUENCY, message, 0, 1, 0);
            snprintf(message, sizeof(message), "%.2f", pzValues.pf);
            esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC_PF, message, 0, 1, 0);
        }

        ESP_LOGI( TAG, "Stack High Water Mark: %ld Bytes free", ( unsigned long int ) uxTaskGetStackHighWaterMark( NULL ) );     /* Show's what's left of the specified stacksize */

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
    vTaskDelete( NULL );
}
