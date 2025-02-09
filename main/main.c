#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mqtt_client.h"
#include <nvs_flash.h>

#include "pzem004tv3.h"
#include "hd44780.h"
#include "pcf8574.h"
#include "wifi_connect.h"
#include "web_server.h"
#include "esp_spiffs.h"

/* @brief Set ESP32  Serial Configuration */
pzem_setup_t pzConf =
{
    .pzem_uart   = UART_NUM_1,              /*  <== Specify the UART you want to use, UART_NUM_0, UART_NUM_1, UART_NUM_2 (ESP32 specific) */
    .pzem_rx_pin = GPIO_NUM_18,             /*  <== GPIO for RX */
    .pzem_tx_pin = GPIO_NUM_17,             /*  <== GPIO for TX */
    .pzem_addr   = PZ_DEFAULT_ADDRESS,      /*  If your module has a different address, specify here or update the variable in pzem004tv3.h */
};

TaskHandle_t PMonTHandle = NULL;
TaskHandle_t LcdHandle = NULL;
TaskHandle_t AlertHandle = NULL;
_current_values_t pzValues;
bool relay_flag = false;
float voltage_min = 0.0;
float voltage_max = 0.0;
float current_max = 0.0;
float power_max = 0.0;
static i2c_dev_t pcf8574;

static esp_err_t write_lcd_data(const hd44780_t *lcd, uint8_t data)
{
    return pcf8574_port_write(&pcf8574, data);
}

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

void PMonTask( void * pvParameters );
void lcdTask( void * pvParameters );

void alertTask( void * pvParameters );

void app_main()
{   
    /* Initialize NVS partition */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        /* NVS partition was truncated
         * and needs to be erased */
        ESP_ERROR_CHECK(nvs_flash_erase());
        /* Retry nvs_flash_init */
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    /* Initialize LCD */
    ESP_ERROR_CHECK(i2cdev_init());
    xTaskCreate( lcdTask, "LCD", (512 * 8), NULL, tskIDLE_PRIORITY + 1, &LcdHandle );
    load_credentials(); // Verifica se a credenciais HA salva
    xTaskCreate( PMonTask, "PowerMon", ( 512 * 8 ), NULL, tskIDLE_PRIORITY + 1, &PMonTHandle );
    xTaskCreate( alertTask, "AlertTask", ( 512 * 8 ), NULL, tskIDLE_PRIORITY + 1, &AlertHandle );
}

void PMonTask( void * pvParameters )
{
    PzemInit( &pzConf );
    PzResetEnergy( &pzConf );
    while(1)
    {        
        if (mqtt_client != NULL && PzemGetValues( &pzConf, &pzValues )) {
            printf( "Vrms: %.1fV - Irms: %.3fA - P: %.1fW - E: %.2fWh\n", pzValues.voltage, pzValues.current, pzValues.power, pzValues.energy );
            printf( "Freq: %.1fHz - PF: %.2f\n", pzValues.frequency, pzValues.pf );
            
            // Publish MQTT messages
            char message[24];
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



            // Atualiza o LCD
            hd44780_gotoxy(&lcd, 0, 0);
            snprintf(message, sizeof(message), "Voltage: %6.2f V", pzValues.voltage);
            hd44780_puts(&lcd, message);

            hd44780_gotoxy(&lcd, 0, 1);
            snprintf(message, sizeof(message), "Current:  %05.2f A", pzValues.current);
            hd44780_puts(&lcd, message);

            hd44780_gotoxy(&lcd, 0, 2);
            snprintf(message, sizeof(message), "Power:  %07.2f W", pzValues.power);
            hd44780_puts(&lcd, message);

            hd44780_gotoxy(&lcd, 0, 3);
            snprintf(message, sizeof(message), "Energy: %07.3f kWh", pzValues.energy);
            hd44780_puts(&lcd, message);

            // Compara os valores dos alertas com os valores medidos
            if (voltage_min != 0.0 && pzValues.voltage < voltage_min){
                relay_flag = true;
                hd44780_clear(&lcd);
                hd44780_gotoxy(&lcd, 0, 0);
                hd44780_puts(&lcd, "Alerta!");
                hd44780_gotoxy(&lcd, 0, 1);
                hd44780_puts(&lcd, "Tensao baixa!");
                vTaskSuspend(PMonTHandle); // Suspende a tarefa PMonTask
            } else if (voltage_max != 0.0 && pzValues.voltage > voltage_max){
                relay_flag = true;
                hd44780_clear(&lcd);
                hd44780_gotoxy(&lcd, 0, 0);
                hd44780_puts(&lcd, "Alerta!");
                hd44780_gotoxy(&lcd, 0, 1);
                hd44780_puts(&lcd, "Tensao alta!");
                vTaskSuspend(PMonTHandle); // Suspende a tarefa PMonTask
            } else if (current_max != 0.0 && pzValues.current > current_max) {
                relay_flag = true;
                hd44780_clear(&lcd);
                hd44780_gotoxy(&lcd, 0, 0);
                hd44780_puts(&lcd, "Alerta!");
                hd44780_gotoxy(&lcd, 0, 1);
                hd44780_puts(&lcd, "Corrente alta!");
                vTaskSuspend(PMonTHandle); // Suspende a tarefa PMonTask
            } else if (power_max != 0.0 && pzValues.power > power_max) {
                relay_flag = true;
                hd44780_clear(&lcd);
                hd44780_gotoxy(&lcd, 0, 0);
                hd44780_puts(&lcd, "Alerta!");
                hd44780_gotoxy(&lcd, 0, 1);
                hd44780_puts(&lcd, "Potencia alta!");
                vTaskSuspend(PMonTHandle); // Suspende a tarefa PMonTask
            }
        }
        ESP_LOGI( TAG, "Stack High Water Mark: %ld Bytes free", ( unsigned long int ) uxTaskGetStackHighWaterMark( NULL ) );     /* Show's what's left of the specified stacksize */
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    vTaskDelete( NULL );
}

void lcdTask(void *pvParameters)
{
    memset(&pcf8574, 0, sizeof(i2c_dev_t));
    ESP_ERROR_CHECK(pcf8574_init_desc(&pcf8574, 0x3f, 0, 36, 37));
    ESP_ERROR_CHECK(hd44780_init(&lcd));
    hd44780_switch_backlight(&lcd, true);

    hd44780_gotoxy(&lcd, 0, 0);
    hd44780_puts(&lcd, "Projeto de PI3");
    hd44780_gotoxy(&lcd, 0, 2);
    hd44780_puts(&lcd, "Medidor de Energia");
    vTaskDelay(pdMS_TO_TICKS(3000));
    hd44780_clear(&lcd);
    wifi_init();
    start_webserver();
    
    vTaskDelete( NULL );
}

void alertTask(void *pvParameters) {

    gpio_config_t io_conf_rele = {
        .intr_type = GPIO_INTR_DISABLE,       // Sem interrupção
        .mode = GPIO_MODE_OUTPUT,             // Configura como saída
        .pin_bit_mask = (1ULL << GPIO_NUM_6), // Seleciona o pino
        .pull_down_en = GPIO_PULLDOWN_DISABLE,// Desabilita o pull-down
        .pull_up_en = GPIO_PULLUP_DISABLE,    // Desabilita o pull-up
    };
    gpio_config(&io_conf_rele);
    gpio_set_level(GPIO_NUM_6, 0);

    while (1) {
        // Ler os valores dos alertas da NVS
        nvs_handle_t nvs_handle;
        esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);
        if (err == ESP_OK) {
            size_t required_size;
            nvs_get_str(nvs_handle, "voltage_min", NULL, &required_size);
            char* voltage_min_str = malloc(required_size);
            nvs_get_str(nvs_handle, "voltage_min", voltage_min_str, &required_size);
            voltage_min = atof(voltage_min_str);
            free(voltage_min_str);

            nvs_get_str(nvs_handle, "voltage_max", NULL, &required_size);
            char* voltage_max_str = malloc(required_size);
            nvs_get_str(nvs_handle, "voltage_max", voltage_max_str, &required_size);
            voltage_max = atof(voltage_max_str);
            free(voltage_max_str);

            nvs_get_str(nvs_handle, "current_max", NULL, &required_size);
            char* current_max_str = malloc(required_size);
            nvs_get_str(nvs_handle, "current_max", current_max_str, &required_size);
            current_max = atof(current_max_str);
            free(current_max_str);

            nvs_get_str(nvs_handle, "power_max", NULL, &required_size);
            char* power_max_str = malloc(required_size);
            nvs_get_str(nvs_handle, "power_max", power_max_str, &required_size);
            power_max = atof(power_max_str);
            free(power_max_str);

            nvs_close(nvs_handle);
        }

        if(relay_flag) {
            vTaskSuspend(PMonTHandle); // Suspende a tarefa PMonTask
            gpio_set_level(GPIO_NUM_6, 1); //Desliga

        } else {
            gpio_set_level(GPIO_NUM_6, 0); //Liga
            vTaskDelay(pdMS_TO_TICKS(5000));
            vTaskResume(PMonTHandle); // Retoma a tarefa PMonTask
        }       
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    vTaskDelete(NULL);
}