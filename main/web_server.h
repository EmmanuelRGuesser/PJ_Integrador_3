#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "credentials.h"
#include "pzem004tv3.h"

extern bool relay_flag;
extern _current_values_t pzValues;
extern pzem_setup_t pzConf;
void start_webserver(void);