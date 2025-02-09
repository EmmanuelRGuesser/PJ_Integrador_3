#include "wifi_connect.h"
#include "mdns.h"

#define DEBOUNCE_TIME_MS 3000 // Tempo de debounce em milissegundos

/* Signal Wi-Fi events on this event-group */
const int WIFI_CONNECTED_EVENT = BIT0;
static EventGroupHandle_t wifi_event_group;
esp_mqtt_client_handle_t mqtt_client;

#define PROV_TRANSPORT_BLE      "ble"
#define RESET_FLAG_KEY "reset_flag"
bool provisioned = false;
static TimerHandle_t debounce_timer;

// Função de callback para resetar as credenciais de Wi-Fi
void IRAM_ATTR button_isr_handler(void* arg) {
    if (gpio_get_level(BUTTON_GPIO) == 0) { // Verifica se o botão está pressionado
        if (xTimerIsTimerActive(debounce_timer) == pdFALSE) {
            xTimerStartFromISR(debounce_timer, NULL);
        }
    } else {
        if (xTimerIsTimerActive(debounce_timer) == pdTRUE) {
            xTimerStopFromISR(debounce_timer, NULL);
        }
    }
}

void debounce_timer_callback(TimerHandle_t xTimer) {
    // Marca a flag de reset na NVS
    nvs_handle_t nvs_handle;
    nvs_open("storage", NVS_READWRITE, &nvs_handle);
    uint8_t reset_flag = 1;
    nvs_set_u8(nvs_handle, RESET_FLAG_KEY, reset_flag);
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);

    ESP_LOGI(TAG, "Botão pressionado por 3 segundos, reiniciando o dispositivo");
    esp_restart(); // Reinicia o dispositivo para aplicar as mudanças
}

// Função para inicializar o botão
void init_button() {
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_ANYEDGE,  // Interrupção em qualquer borda
        .mode = GPIO_MODE_INPUT,         // Configura como entrada
        .pin_bit_mask = (1ULL << BUTTON_GPIO), // Seleciona o pino
        .pull_up_en = GPIO_PULLUP_ENABLE, // Habilita o pull-up
    };
    gpio_config(&io_conf);

    // Instala o serviço de interrupção do GPIO
    gpio_install_isr_service(0);
    // Adiciona a função de callback para o pino do botão
    gpio_isr_handler_add(BUTTON_GPIO, button_isr_handler, NULL);

    // Cria o timer de debounce
    debounce_timer = xTimerCreate("debounce_timer", pdMS_TO_TICKS(DEBOUNCE_TIME_MS), pdFALSE, (void*)0, debounce_timer_callback);
}

// Função para inicializar o LED
void init_leds() {
    gpio_config_t io_conf_wifi = {
        .intr_type = GPIO_INTR_DISABLE,  // Sem interrupção
        .mode = GPIO_MODE_OUTPUT,        // Configura como saída
        .pin_bit_mask = (1ULL << LED_WIFI), // Seleciona o pino
        .pull_down_en = GPIO_PULLDOWN_DISABLE, // Desabilita o pull-down
        .pull_up_en = GPIO_PULLUP_DISABLE, // Desabilita o pull-up
    };
    gpio_config(&io_conf_wifi);

    gpio_set_level(LED_WIFI, 0);
}

// Função para apagar as credenciais de Wi-Fi e MQTT
void check_and_reset_provisioning() {
    nvs_handle_t nvs_handle;
    nvs_open("storage", NVS_READWRITE, &nvs_handle);
    uint8_t reset_flag = 0;
    nvs_get_u8(nvs_handle, RESET_FLAG_KEY, &reset_flag);

    if (reset_flag == 1) {
        ESP_LOGI(TAG, "Resetando as credenciais de Wi-Fi");
        wifi_prov_mgr_reset_provisioning();
        reset_credentials();
        reset_flag = 0;
        nvs_set_u8(nvs_handle, RESET_FLAG_KEY, reset_flag);
        nvs_commit(nvs_handle);
    } else {
        /* Let's find out if the device is provisioned */
        ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));
    }

    nvs_close(nvs_handle);
}

void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    switch (event_id) {
        case MQTT_EVENT_CONNECTED:
            printf("MQTT conectado!\n");
            gpio_set_level(LED_WIFI, 1);
            break;
            
        case MQTT_EVENT_DISCONNECTED:
            load_credentials();
            ESP_LOGI(TAG, "Username: %s", username_ha);
            ESP_LOGI(TAG, "Password: %s", password_ha);
            printf("MQTT desconectado!\n");
            gpio_set_level(LED_WIFI, 0);
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

void start_mdns_service(void) {
    // Initialize mDNS service
    ESP_ERROR_CHECK(mdns_init());
    ESP_ERROR_CHECK(mdns_hostname_set("monitor-energia"));
    ESP_ERROR_CHECK(mdns_instance_name_set("Monitor de Energia - Web Server"));

    // Set mDNS service
    ESP_ERROR_CHECK(mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0));

    ESP_LOGI(TAG, "mDNS service started with hostname: monitor-energia");
}

static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{

    static int retries;

    if (event_base == WIFI_PROV_EVENT) {
        switch (event_id) {
            case WIFI_PROV_START:
                ESP_LOGI(TAG, "Provisioning started");
                hd44780_clear(&lcd);
                hd44780_gotoxy(&lcd, 0, 0);
                hd44780_puts(&lcd, "Conecte ao WIFI");
                hd44780_gotoxy(&lcd, 0, 1);
                hd44780_puts(&lcd, "Baixe o APP: ");
                hd44780_gotoxy(&lcd, 0, 2);
                hd44780_puts(&lcd, "ESP BLE Provisioning");
                hd44780_gotoxy(&lcd, 0, 3);
                hd44780_puts(&lcd, "Leia o QR Code");
                break;
            case WIFI_PROV_CRED_RECV: {
                wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
                ESP_LOGI(TAG, "Received Wi-Fi credentials"
                         "\n\tSSID     : %s\n\tPassword : %s",
                         (const char *) wifi_sta_cfg->ssid,
                         (const char *) wifi_sta_cfg->password);
                break;
            }
            case WIFI_PROV_CRED_FAIL: {
                hd44780_clear(&lcd);
                hd44780_gotoxy(&lcd, 0, 0);
                hd44780_puts(&lcd, "Falha na conexao");
                hd44780_gotoxy(&lcd, 0, 2);
                hd44780_puts(&lcd, "Reset o dispositivo");
                
                wifi_prov_sta_fail_reason_t *reason = (wifi_prov_sta_fail_reason_t *)event_data;
                ESP_LOGE(TAG, "Provisioning failed!\n\tReason : %s"
                         "\n\tPlease reset to factory and retry provisioning",
                         (*reason == WIFI_PROV_STA_AUTH_ERROR) ?
                         "Wi-Fi station authentication failed" : "Wi-Fi access-point not found");

                retries++;
                if (retries >= CONFIG_EXAMPLE_PROV_MGR_MAX_RETRY_CNT) {
                    ESP_LOGI(TAG, "Failed to connect with provisioned AP, resetting provisioned credentials");
                    wifi_prov_mgr_reset_sm_state_on_failure();
                    retries = 0;
                }

                break;
            }
            case WIFI_PROV_CRED_SUCCESS:
                ESP_LOGI(TAG, "Provisioning successful");

                retries = 0;
                break;
            case WIFI_PROV_END:
                /* De-initialize manager once provisioning is finished */
                wifi_prov_mgr_deinit();
                break;
            default:
                break;
        }
    } else if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                esp_wifi_connect();
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGI(TAG, "Disconnected. Connecting to the AP again...");

                hd44780_clear(&lcd);
                hd44780_gotoxy(&lcd, 0, 0);
                hd44780_puts(&lcd, "Conectando ao WIFI");	
                esp_wifi_connect();
                break;
            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(TAG, "Connected to the AP");
                hd44780_clear(&lcd);
                start_webserver();
                mqtt_init();
                start_mdns_service(); 
                break;
            default:
                break;
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Connected with IP Address:" IPSTR, IP2STR(&event->ip_info.ip));
        /* Signal main application to continue execution */
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_EVENT);
    } else if (event_base == PROTOCOMM_TRANSPORT_BLE_EVENT) {
        switch (event_id) {
            case PROTOCOMM_TRANSPORT_BLE_CONNECTED:
                ESP_LOGI(TAG, "BLE transport: Connected!");
                break;
            case PROTOCOMM_TRANSPORT_BLE_DISCONNECTED:
                ESP_LOGI(TAG, "BLE transport: Disconnected!");
                break;
            default:
                break;
        }

    } else if (event_base == PROTOCOMM_SECURITY_SESSION_EVENT) {
        switch (event_id) {
            case PROTOCOMM_SECURITY_SESSION_SETUP_OK:
                ESP_LOGI(TAG, "Secured session established!");
                break;
            case PROTOCOMM_SECURITY_SESSION_INVALID_SECURITY_PARAMS:
                ESP_LOGE(TAG, "Received invalid security parameters for establishing secure session!");
                break;
            case PROTOCOMM_SECURITY_SESSION_CREDENTIALS_MISMATCH:
                ESP_LOGE(TAG, "Received incorrect username and/or PoP for establishing secure session!");
                break;
            default:
                break;
        }
    }
}

static void wifi_init_sta(void)
{
    /* Start Wi-Fi in station mode */
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

/* Handler for the optional provisioning endpoint registered by the application.
 * The data format can be chosen by applications. Here, we are using plain ascii text.
 * Applications can choose to use other formats like protobuf, JSON, XML, etc.
 * Note that memory for the response buffer must be allocated using heap as this buffer
 * gets freed by the protocomm layer once it has been sent by the transport layer.
 */
esp_err_t custom_prov_data_handler(uint32_t session_id, const uint8_t *inbuf, ssize_t inlen,
                                          uint8_t **outbuf, ssize_t *outlen, void *priv_data)
{
    if (inbuf) {
        ESP_LOGI(TAG, "Received data: %.*s", inlen, (char *)inbuf);
    }
    char response[] = "SUCCESS";
    *outbuf = (uint8_t *)strdup(response);
    if (*outbuf == NULL) {
        ESP_LOGE(TAG, "System out of memory");
        return ESP_ERR_NO_MEM;
    }
    *outlen = strlen(response) + 1; /* +1 for NULL terminating byte */

    return ESP_OK;
}

void wifi_init(void)
{
    // Inicializa o botão
    init_button();
    // Inicializa o LED
    init_leds();
    
    /* Initialize TCP/IP */
    ESP_ERROR_CHECK(esp_netif_init());

    /* Initialize the event loop */
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_event_group = xEventGroupCreate();

    /* Register our event handler for Wi-Fi, IP and Provisioning related events */
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(PROTOCOMM_TRANSPORT_BLE_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));

    ESP_ERROR_CHECK(esp_event_handler_register(PROTOCOMM_SECURITY_SESSION_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    /* Initialize Wi-Fi including netif with default config */
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /* Configuration for the provisioning manager */
    wifi_prov_mgr_config_t config = {
        /* What is the Provisioning Scheme that we want ?
        * wifi_prov_scheme_softap or wifi_prov_scheme_ble */
        .scheme = wifi_prov_scheme_ble,
        .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM
    };

    ESP_ERROR_CHECK(wifi_prov_mgr_init(config));

    check_and_reset_provisioning();

       /* If device is not yet provisioned start provisioning service */
    if (!provisioned) {
        ESP_LOGI(TAG, "Starting provisioning");

        /* What is the Device Service Name that we want
         * This translates to :
         *     - Wi-Fi SSID when scheme is wifi_prov_scheme_softap
         *     - device name when scheme is wifi_prov_scheme_ble
         */
        const char service_name[] = "Monitor de energia";
        

        /* What is the security level that we want (0, 1, 2):
         *      - WIFI_PROV_SECURITY_0 is simply plain text communication.
         *      - WIFI_PROV_SECURITY_1 is secure communication which consists of secure handshake
         *          using X25519 key exchange and proof of possession (pop) and AES-CTR
         *          for encryption/decryption of messages.
         *      - WIFI_PROV_SECURITY_2 SRP6a based authentication and key exchange
         *        + AES-GCM encryption/decryption of messages
         */
        wifi_prov_security_t security = WIFI_PROV_SECURITY_1;

        /* Do we want a proof-of-possession (ignored if Security 0 is selected):
         *      - this should be a string with length > 0
         *      - NULL if not used
         */
        const char *pop = "abcd1234";

        /* This is the structure for passing security parameters
         * for the protocomm security 1.
         */
        wifi_prov_security1_params_t *sec_params = pop;

        /* What is the service key (could be NULL)
         * This translates to :
         *     - Wi-Fi password when scheme is wifi_prov_scheme_softap
         *          (Minimum expected length: 8, maximum 64 for WPA2-PSK)
         *     - simply ignored when scheme is wifi_prov_scheme_ble
         */
        const char *service_key = NULL;

       /* This step is only useful when scheme is wifi_prov_scheme_ble. This will
         * set a custom 128 bit UUID which will be included in the BLE advertisement
         * and will correspond to the primary GATT service that provides provisioning
         * endpoints as GATT characteristics. Each GATT characteristic will be
         * formed using the primary service UUID as base, with different auto assigned
         * 12th and 13th bytes (assume counting starts from 0th byte). The client side
         * applications must identify the endpoints by reading the User Characteristic
         * Description descriptor (0x2901) for each characteristic, which contains the
         * endpoint name of the characteristic */
        uint8_t custom_service_uuid[] = {
            /* LSB <---------------------------------------
             * ---------------------------------------> MSB */
            0xb4, 0xdf, 0x5a, 0x1c, 0x3f, 0x6b, 0xf4, 0xbf,
            0xea, 0x4a, 0x82, 0x03, 0x04, 0x90, 0x1a, 0x02,
        };

        /* If your build fails with linker errors at this point, then you may have
         * forgotten to enable the BT stack or BTDM BLE settings in the SDK (e.g. see
         * the sdkconfig.defaults in the example project) */
        wifi_prov_scheme_ble_set_service_uuid(custom_service_uuid);

        /* An optional endpoint that applications can create if they expect to
         * get some additional custom data during provisioning workflow.
         * The endpoint name can be anything of your choice.
         * This call must be made before starting the provisioning.
         */
        wifi_prov_mgr_endpoint_create("custom-data");

        /* Do not stop and de-init provisioning even after success,
         * so that we can restart it later. */

       /* Start provisioning service */
        ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(security, (const void *) sec_params, service_name, service_key));

        /* The handler for the optional endpoint created above.
         * This call must be made after starting the provisioning, and only if the endpoint
         * has already been created above.
         */
        wifi_prov_mgr_endpoint_register("custom-data", custom_prov_data_handler, NULL);
    } else {
        ESP_LOGI(TAG, "Already provisioned, starting Wi-Fi STA");

        /* We don't need the manager as device is already provisioned,
         * so let's release it's resources */
        wifi_prov_mgr_deinit();

        /* Start Wi-Fi station */
        wifi_init_sta();
    }

    /* Wait for Wi-Fi connection */
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_EVENT, true, true, portMAX_DELAY);

}