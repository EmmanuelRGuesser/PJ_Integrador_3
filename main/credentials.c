#include "credentials.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"

static const char *TAG = "CREDENTIALS";

char username_ha[32] = {0};
char password_ha[32] = {0};

void load_credentials() {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);
    if (err == ESP_OK) {
        size_t required_size;
        err = nvs_get_str(nvs_handle, "username", NULL, &required_size);
        if (err == ESP_OK && required_size <= sizeof(username_ha)) {
            nvs_get_str(nvs_handle, "username", username_ha, &required_size);
        }
        err = nvs_get_str(nvs_handle, "password", NULL, &required_size);
        if (err == ESP_OK && required_size <= sizeof(password_ha)) {
            nvs_get_str(nvs_handle, "password", password_ha, &required_size);
        }
        nvs_close(nvs_handle);
    } else {
        ESP_LOGI(TAG, "No credentials found in NVS");
    }
}

void reset_credentials() {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (err == ESP_OK) {
        nvs_erase_key(nvs_handle, "username");
        nvs_erase_key(nvs_handle, "password");
        nvs_commit(nvs_handle);
        nvs_close(nvs_handle);
    } else {
        ESP_LOGE(TAG, "Error (%s) erasing Wi-Fi password!", esp_err_to_name(err));
    }
}