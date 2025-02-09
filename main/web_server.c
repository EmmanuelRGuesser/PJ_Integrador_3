#include "web_server.h"

bool relay_state = false;

static const char *TAG = "WEB_SERVER";
const char *index_html = "<!DOCTYPE html><html><head>"
                         "<style>"
                         "body { font-family: Arial, sans-serif; text-align: center; }"
                         "header, footer { background-color: #4CAF50; color: white; padding: 10px 0; }"
                         ".button-container { display: flex; flex-direction: column; align-items: center; }"
                         ".button-container button, .relay-buttons button { background-color: #4CAF50; color: white; padding: 10px 20px; margin: 10px; border: none; cursor: pointer; }"
                         ".button-container button:hover, .relay-buttons button:hover { background-color: #45a049; }"
                         ".relay-buttons { display: flex; justify-content: center; margin-top: 20px; }"
                         ".relay-buttons button { margin: 0 10px; }"
                         "</style>"
                         "<script>"
                         "function showAlert(message) { alert(message); }"
                         "function sendRequest(url, message) {"
                         "  var xhr = new XMLHttpRequest();"
                         "  xhr.open('GET', url, true);"
                         "  xhr.onload = function () {"
                         "    if (xhr.status === 200) {"
                         "      showAlert(message);"
                         "    }"
                         "  };"
                         "  xhr.send();"
                         "}"
                         "</script>"
                         "</head><body>"
                         "<header><h1>Monitor de Energia</h1></header>"
                         "<h2>Bem-vindo ao Monitor de Energia</h2>"
                         "<div class='button-container'>"
                         "<button onclick=\"location.href='/config'\">Configurar MQTT</button>"
                         "<button onclick=\"location.href='/alerts'\">Configurar Alertas</button>"
                         "<button onclick=\"location.href='/energy'\">Consumo Acumulado</button>"
                         "</div>"
                         "<div class='relay-buttons'>"
                         "<button onclick=\"sendRequest('/on_relay', 'Rele Ligado')\">Ligar</button>"
                         "<button onclick=\"sendRequest('/off_relay', 'Rele Desligado')\">Desligar</button>"
                         "</div>"
                         "<footer><p>&copy; 2025 Monitor de Energia</p></footer>"
                         "</body></html>";

// Função para decodificar a URL
void url_decode(char *dst, const char *src) {
    char a, b;
    while (*src) {
        if ((*src == '%') && ((a = src[1]) && (b = src[2])) && (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a') a -= 'a'-'A';
            if (a >= 'A') a -= ('A' - 10);
            else a -= '0';
            if (b >= 'a') b -= 'a'-'A';
            if (b >= 'A') b -= ('A' - 10);
            else b -= '0';
            *dst++ = 16*a+b;
            src+=3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst++ = '\0';
}

esp_err_t get_handler(httpd_req_t *req) {
    char query[128];
    char username[32];
    char password[32];
    char decoded_username[32];
    char decoded_password[32];

    // Obter a string de consulta
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        ESP_LOGI(TAG, "Query: %s", query);

        // Extrair o valor do parâmetro 'username'
        if (httpd_query_key_value(query, "username", username, sizeof(username)) == ESP_OK) {
            ESP_LOGI(TAG, "Username: %s", username);
            url_decode(decoded_username, username);
        } else {
            ESP_LOGE(TAG, "Failed to get 'username' parameter");
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid query parameters");
            return ESP_FAIL;
        }

        // Extrair o valor do parâmetro 'password'
        if (httpd_query_key_value(query, "password", password, sizeof(password)) == ESP_OK) {
            ESP_LOGI(TAG, "Password: %s", password);
            url_decode(decoded_password, password);
        } else {
            ESP_LOGE(TAG, "Failed to get 'password' parameter");
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid query parameters");
            return ESP_FAIL;
        }

        // Salvar as credenciais no NVS
        nvs_handle_t nvs_handle;
        esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
        if (err == ESP_OK) {
            nvs_set_str(nvs_handle, "username", decoded_username);
            nvs_set_str(nvs_handle, "password", decoded_password);
            nvs_commit(nvs_handle);
            nvs_close(nvs_handle);
            httpd_resp_send(req, "Credentials saved", HTTPD_RESP_USE_STRLEN);
        } else {
            httpd_resp_send_500(req);
        }
    } else {
        ESP_LOGE(TAG, "No query string found");
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid query parameters");
        return ESP_FAIL;
    }
    esp_restart();
    return ESP_OK;
}

httpd_uri_t uri_get = {
    .uri      = "/get",
    .method   = HTTP_GET,
    .handler  = get_handler,
    .user_ctx = NULL
};

esp_err_t index_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, index_html, HTTPD_RESP_USE_STRLEN);
}

httpd_uri_t uri_index = {
    .uri      = "/",
    .method   = HTTP_GET,
    .handler  = index_handler,
    .user_ctx = NULL
};

esp_err_t config_handler(httpd_req_t *req) {
    char username[32] = "";
    char password[32] = "";
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);
    if (err == ESP_OK) {
        size_t length = sizeof(username);
        nvs_get_str(nvs_handle, "username", username, &length);
        length = sizeof(password);
        nvs_get_str(nvs_handle, "password", password, &length);
        nvs_close(nvs_handle);
    }

    char config_page[2048];
    snprintf(config_page, sizeof(config_page),
             "<!DOCTYPE html><html><head>"
             "<style>"
             "body { font-family: Arial, sans-serif; text-align: center; }"
             "header, footer { background-color: #4CAF50; color: white; padding: 10px 0; }"
             "form { display: inline-block; text-align: left; margin-top: 20px; }"
             "input[type=text], input[type=submit] { padding: 10px; margin: 5px 0; width: 100%%; }"
             "input[type=submit] { background-color: #4CAF50; color: white; border: none; cursor: pointer; }"
             "input[type=submit]:hover { background-color: #45a049; }"
             "</style>"
             "<script>"
             "function showAlert() { alert('Credentials saved'); }"
             "function submitForm(event) {"
             "  event.preventDefault();"
             "  var xhr = new XMLHttpRequest();"
             "  xhr.open('GET', '/get?' + new URLSearchParams(new FormData(event.target)).toString(), true);"
             "  xhr.onload = function () {"
             "    if (xhr.status === 200) {"
             "      showAlert();"
             "    }"
             "  };"
             "  xhr.send();"
             "}"
             "</script>"
             "</head><body>"
             "<header><h1>Configurar MQTT</h1></header>"
             "<form onsubmit='submitForm(event)'>"
             "Username: <input type='text' name='username' value='%s'><br>"
             "Password: <input type='text' name='password' value='%s'><br>"
             "<input type='submit' value='Save'></form>"
             "<footer><p>&copy; 2025 Monitor de Energia</p></footer>"
             "</body></html>", username, password);

    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, config_page, HTTPD_RESP_USE_STRLEN);
}

httpd_uri_t uri_config = {
    .uri      = "/config",
    .method   = HTTP_GET,
    .handler  = config_handler,
    .user_ctx = NULL
};

esp_err_t alerts_handler(httpd_req_t *req) {
    char voltage_min[8] = "";
    char voltage_max[8] = "";
    char current_max[8] = "";
    char power_max[8] = "";
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);
    if (err == ESP_OK) {
        size_t length = sizeof(voltage_min);
        nvs_get_str(nvs_handle, "voltage_min", voltage_min, &length);
        length = sizeof(voltage_max);
        nvs_get_str(nvs_handle, "voltage_max", voltage_max, &length);
        length = sizeof(current_max);
        nvs_get_str(nvs_handle, "current_max", current_max, &length);
        length = sizeof(power_max);
        nvs_get_str(nvs_handle, "power_max", power_max, &length);
        nvs_close(nvs_handle);
    }

    char alerts_page[2048];
    snprintf(alerts_page, sizeof(alerts_page),
             "<!DOCTYPE html><html><head>"
             "<style>"
             "body { font-family: Arial, sans-serif; text-align: center; }"
             "header, footer { background-color: #4CAF50; color: white; padding: 10px 0; }"
             "form { display: inline-block; text-align: left; margin-top: 20px; }"
             "input[type=number], input[type=submit] { padding: 10px; margin: 5px 0; width: 100%%; }"
             "input[type=submit] { background-color: #4CAF50; color: white; border: none; cursor: pointer; }"
             "input[type=submit]:hover { background-color: #45a049; }"
             "</style>"
             "<script>"
             "function showAlert() { alert('Alerts saved'); }"
             "function submitForm(event) {"
             "  event.preventDefault();"
             "  var xhr = new XMLHttpRequest();"
             "  xhr.open('GET', '/set_alerts?' + new URLSearchParams(new FormData(event.target)).toString(), true);"
             "  xhr.onload = function () {"
             "    if (xhr.status === 200) {"
             "      showAlert();"
             "    }"
             "  };"
             "  xhr.send();"
             "}"
             "</script>"
             "</head><body>"
             "<header><h1>Configurar Alertas</h1></header>"
             "<form onsubmit='submitForm(event)'>"
             "Min Voltage : <input type='number' name='voltage_min' value='%s'><br>"
             "Max Voltage: <input type='number' name='voltage_max' value='%s'><br>"
             "Max Current: <input type='number' name='current_max' value='%s'><br>"
             "Max Power: <input type='number' name='power_max' value='%s'><br>"
             "<input type='submit' value='Save'></form>"
             "<footer><p>&copy; 2025 Monitor de Energia</p></footer>"
             "</body></html>", voltage_min, voltage_max, current_max, power_max);

    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, alerts_page, HTTPD_RESP_USE_STRLEN);
}

httpd_uri_t uri_alerts = {
    .uri      = "/alerts",
    .method   = HTTP_GET,
    .handler  = alerts_handler,
    .user_ctx = NULL
};

esp_err_t set_alerts_handler(httpd_req_t *req) {
    char query[128];
    char voltage_min[8], voltage_max[8], current_max[8], power_max[8];

    // Obter a string de consulta
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        ESP_LOGI(TAG, "Query: %s", query);

        // Extrair os valores dos parâmetros
        if (httpd_query_key_value(query, "voltage_min", voltage_min, sizeof(voltage_min)) == ESP_OK &&
            httpd_query_key_value(query, "voltage_max", voltage_max, sizeof(voltage_max)) == ESP_OK &&
            httpd_query_key_value(query, "current_max", current_max, sizeof(current_max)) == ESP_OK &&
            httpd_query_key_value(query, "power_max", power_max, sizeof(power_max)) == ESP_OK) {
            
            ESP_LOGI(TAG, "Voltage Min: %s, Voltage Max: %s", voltage_min, voltage_max);
            ESP_LOGI(TAG, "Current Max: %s", current_max);
            ESP_LOGI(TAG, "Power Max: %s", power_max);

            // Salvar os alertas no NVS
            nvs_handle_t nvs_handle;
            esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
            if (err == ESP_OK) {
                nvs_set_str(nvs_handle, "voltage_min", voltage_min);
                nvs_set_str(nvs_handle, "voltage_max", voltage_max);
                nvs_set_str(nvs_handle, "current_max", current_max);
                nvs_set_str(nvs_handle, "power_max", power_max);
                nvs_commit(nvs_handle);
                nvs_close(nvs_handle);
                httpd_resp_send(req, "Alerts saved", HTTPD_RESP_USE_STRLEN);
            } else {
                httpd_resp_send_500(req);
            }
        } else {
            ESP_LOGE(TAG, "Failed to get alert parameters");
            httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid query parameters");
            return ESP_FAIL;
        }
    } else {
        ESP_LOGE(TAG, "No query string found");
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid query parameters");
        return ESP_FAIL;
    }
    return ESP_OK;
}

httpd_uri_t uri_set_alerts = {
    .uri      = "/set_alerts",
    .method   = HTTP_GET,
    .handler  = set_alerts_handler,
    .user_ctx = NULL
};

esp_err_t on_relay_handler(httpd_req_t *req) {
    relay_flag = false;
    httpd_resp_send(req, "Rele Ligado", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

httpd_uri_t uri_on_relay = {
    .uri      = "/on_relay",
    .method   = HTTP_GET,
    .handler  = on_relay_handler,
    .user_ctx = NULL
};

esp_err_t off_relay_handler(httpd_req_t *req) {
    relay_flag = true;
    httpd_resp_send(req, "Rele Desligado", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

httpd_uri_t uri_off_relay = {
    .uri      = "/off_relay",
    .method   = HTTP_GET,
    .handler  = off_relay_handler,
    .user_ctx = NULL
};

esp_err_t energy_handler(httpd_req_t *req) {
    char price_per_wh_str[8] = "1.0";  // Valor padrão
    float price_per_wh = 1.0;          // Valor padrão
    float energy = pzValues.energy;    // Energia acumulada
    float cost = 0.0;                  // Custo calculado

    char query[128] = {0};
    bool is_ajax = false;  // Flag para verificar se a requisição é AJAX

    // Verificar cabeçalhos para identificar AJAX
    char buf[16];
    if (httpd_req_get_hdr_value_str(req, "X-Requested-With", buf, sizeof(buf)) == ESP_OK) {
        if (strcmp(buf, "XMLHttpRequest") == 0) {
            is_ajax = true;
        }
    }

    // Obter a query string
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        ESP_LOGI(TAG, "Recebendo query: %s", query);
        if (httpd_query_key_value(query, "price_per_wh", price_per_wh_str, sizeof(price_per_wh_str)) == ESP_OK) {
            price_per_wh = atof(price_per_wh_str);
            ESP_LOGI(TAG, "Preço por Wh recebido: %f", price_per_wh);
        } else {
            ESP_LOGW(TAG, "Parâmetro 'price_per_wh' ausente, usando valor padrão: %f", price_per_wh);
        }
    } else {
        ESP_LOGE(TAG, "Nenhuma query string recebida, usando valor padrão: %f", price_per_wh);
    }

    // Calcular o custo
    cost = energy * price_per_wh;
    ESP_LOGI(TAG, "Energia: %f Wh, Custo calculado: R$ %f", energy, cost);

    // Se for uma requisição AJAX, retorna apenas JSON com energia e custo
    if (is_ajax) {
        char response_json[64];
        snprintf(response_json, sizeof(response_json), "{\"energy\": \"%.2f\", \"cost\": \"%.2f\"}", energy, cost);
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, response_json, HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }

    // Caso contrário, retorna a página HTML completa
    httpd_resp_set_type(req, "text/html");
    httpd_resp_sendstr_chunk(req, "<!DOCTYPE html><html><head><style>"
                                   "body { font-family: Arial, sans-serif; text-align: center; }"
                                   "header, footer { background-color: #4CAF50; color: white; padding: 10px 0; }"
                                   "form { display: inline-block; text-align: left; margin-top: 20px; }"
                                   "input[type=number], input[type=submit] { padding: 10px; margin: 5px 0; width: 100%; }"
                                   "input[type=submit] { background-color: #4CAF50; color: white; border: none; cursor: pointer; }"
                                   "input[type=submit]:hover { background-color: #45a049; }"
                                   "button { background-color: #f44336; color: white; padding: 10px 20px; border: none; cursor: pointer; }"
                                   "button:hover { background-color: #d32f2f; }"
                                   "</style><script>"
                                   "function submitForm(event) {"
                                   "  event.preventDefault();"
                                   "  var formData = new FormData(event.target);"
                                   "  var params = new URLSearchParams(formData);"
                                   "  var xhr = new XMLHttpRequest();"
                                   "  xhr.open('GET', '/energy?' + params.toString(), true);"
                                   "  xhr.setRequestHeader('X-Requested-With', 'XMLHttpRequest');"
                                   "  xhr.onload = function () {"
                                   "    if (xhr.status === 200) {"
                                   "      var response = JSON.parse(xhr.responseText);"
                                   "      document.getElementById('costValue').innerText = 'R$ ' + response.cost;"
                                   "      document.getElementById('energyValue').innerText = response.energy + ' Wh';"
                                   "    }"
                                   "  };"
                                   "  xhr.send();"
                                   "}"
                                   "function resetEnergy() {"
                                   "  var xhr = new XMLHttpRequest();"
                                   "  xhr.open('GET', '/reset_energy', true);"
                                   "  xhr.onload = function () {"
                                   "    if (xhr.status === 200) {"
                                   "      document.getElementById('energyValue').innerText = '0.00 Wh';"
                                   "      document.getElementById('costValue').innerText = 'R$ 0.00';"
                                   "    }"
                                   "  };"
                                   "  xhr.send();"
                                   "}"
                                   "</script></head><body><header><h1>Consumo Acumulado</h1></header>");

    char buffer[256];
    snprintf(buffer, sizeof(buffer), "<p>Consumo Acumulado: <span id='energyValue'>%.2f Wh</span></p>", energy);
    httpd_resp_sendstr_chunk(req, buffer);

    snprintf(buffer, sizeof(buffer), "<p>Valor Gasto: <span id='costValue'>R$ %.2f</span></p>", cost);
    httpd_resp_sendstr_chunk(req, buffer);

    snprintf(buffer, sizeof(buffer),
             "<form onsubmit='submitForm(event)'>"
             "R$ por Wh: <input type='number' name='price_per_wh' step='0.01' value='%.2f' required><br>"
             "<input type='submit' value='Calcular'></form>", price_per_wh);
    httpd_resp_sendstr_chunk(req, buffer);

    // Botão para zerar energia acumulada
    httpd_resp_sendstr_chunk(req, "<button onclick='resetEnergy()'>Zerar Energia</button>");

    httpd_resp_sendstr_chunk(req, "<footer><p>&copy; 2025 Monitor de Energia</p></footer></body></html>");
    httpd_resp_sendstr_chunk(req, NULL); 

    ESP_LOGI(TAG, "Página HTML enviada com custo: R$ %.2f", cost);
    return ESP_OK;
}

httpd_uri_t uri_energy = {
    .uri      = "/energy",
    .method   = HTTP_GET,
    .handler  = energy_handler,
    .user_ctx = NULL
};

esp_err_t reset_energy_handler(httpd_req_t *req) {
    if(PzResetEnergy(&pzConf)){ 
        ESP_LOGI(TAG, "Reset energia acumulada");
    } else {
        ESP_LOGI(TAG, "Falha ao zerar energia acumulada");
    }
    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Registrar a URI no servidor
httpd_uri_t uri_reset_energy = {
    .uri      = "/reset_energy",
    .method   = HTTP_GET,
    .handler  = reset_energy_handler,
    .user_ctx = NULL
};

void start_webserver(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 12;

    if (httpd_start(&server, &config) == ESP_OK) {
        ESP_LOGI(TAG, "Servidor iniciado com sucesso.");
    
        httpd_register_uri_handler(server, &uri_index);
        httpd_register_uri_handler(server, &uri_config);
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_alerts);
        httpd_register_uri_handler(server, &uri_set_alerts);
        httpd_register_uri_handler(server, &uri_on_relay);
        httpd_register_uri_handler(server, &uri_off_relay);
        httpd_register_uri_handler(server, &uri_energy);
        httpd_register_uri_handler(server, &uri_reset_energy);
    
        ESP_LOGI(TAG, "Handlers registrados com sucesso.");
    } else {
        ESP_LOGE(TAG, "Falha ao iniciar o servidor.");
    }
}
