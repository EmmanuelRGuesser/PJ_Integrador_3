#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mqtt_client.h"

#include "pzem004tv3.h"
#include "hd44780.h"
#include "pcf8574.h"
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
TaskHandle_t LcdHandle = NULL;
_current_values_t pzValues;            /* Measured values */

static i2c_dev_t pcf8574;

static esp_err_t write_lcd_data(const hd44780_t *lcd, uint8_t data)
{
    return pcf8574_port_write(&pcf8574, data);
}

void PMonTask( void * pvParameters );
void lcdTask( void * pvParameters );

void app_main()
{
    // Configurar o GPIOs
    gpio_set_direction(GPIO_NUM_6, GPIO_MODE_OUTPUT); // Rele
    gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT); // led wifi

    nvs_flash_init();
    wifi_init();

    xTaskCreate( PMonTask, "PowerMon", ( 512 * 8 ), NULL, tskIDLE_PRIORITY, &PMonTHandle );
    ESP_ERROR_CHECK(i2cdev_init());
    xTaskCreate( lcdTask, "LCD", (512 * 8), NULL, tskIDLE_PRIORITY + 1, &LcdHandle );
}

void PMonTask( void * pvParameters )
{
    PzemInit( &pzConf );
    while(1)
    {        
        if (mqtt_client != NULL) {
            printf( "Vrms: %.1fV - Irms: %.3fA - P: %.1fW - E: %.2fWh\n", pzValues.voltage, pzValues.current, pzValues.power, pzValues.energy );
            printf( "Freq: %.1fHz - PF: %.2f\n", pzValues.frequency, pzValues.pf );
            
            char message[10];
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
        //ESP_LOGI( TAG, "Stack High Water Mark: %ld Bytes free", ( unsigned long int ) uxTaskGetStackHighWaterMark( NULL ) );     /* Show's what's left of the specified stacksize */
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
    vTaskDelete( NULL );
}

void lcdTask(void *pvParameters)
{
    hd44780_t lcd = {
        .write_cb = write_lcd_data, // use callback to send data to LCD by I2C GPIO expander
        .font = HD44780_FONT_5X8,
        .lines = 4,
        .pins = {
            .rs = 0,
            .e  = 2,
            .d4 = 4,
            .d5 = 5,
            .d6 = 6,
            .d7 = 7,
            .bl = 3
        }
    };

    memset(&pcf8574, 0, sizeof(i2c_dev_t));
    ESP_ERROR_CHECK(pcf8574_init_desc(&pcf8574, 0x3f, 0, 36, 37));
    ESP_ERROR_CHECK(hd44780_init(&lcd));
    hd44780_switch_backlight(&lcd, true);

    hd44780_gotoxy(&lcd, 0, 0);
    hd44780_puts(&lcd, "Projeto de PIII");
    hd44780_gotoxy(&lcd, 0, 2);
    hd44780_puts(&lcd, "Medidor de Energia");
    vTaskDelay(pdMS_TO_TICKS(3000));
    hd44780_clear(&lcd);

    while(1){
        if(PzemGetValues( &pzConf, &pzValues ))
        {
            char message[24];

            hd44780_gotoxy(&lcd, 0, 0);
            snprintf(message, sizeof(message), "Voltage: %.2f V", pzValues.voltage);
            hd44780_puts(&lcd, message);

            hd44780_gotoxy(&lcd, 0, 1);
            snprintf(message, sizeof(message), "Current: %.2f A", pzValues.current);
            hd44780_puts(&lcd, message);

            hd44780_gotoxy(&lcd, 0, 2);
            snprintf(message, sizeof(message), "Power: %.2f W", pzValues.power);
            hd44780_puts(&lcd, message);

            hd44780_gotoxy(&lcd, 0, 3);
            snprintf(message, sizeof(message), "Energy: %.2f Wh", pzValues.energy);
            hd44780_puts(&lcd, message);

            //ESP_LOGI( TAG, "Stack High Water Mark: %ld Bytes free", ( unsigned long int ) uxTaskGetStackHighWaterMark( NULL ) );
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    };
    vTaskDelete( NULL );
}