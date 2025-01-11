#pragma once

#include "esp_zigbee_core.h"

esp_err_t zb_action_handler(esp_zb_core_action_callback_id_t callback_id, const void *message);
