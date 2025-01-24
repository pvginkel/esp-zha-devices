#pragma once

#include "esp_zigbee_core.h"

class ZigBeeEndpoint {
public:
    virtual uint8_t get_endpoint_id() { return 10; }
    virtual esp_zb_ep_list_t* build_endpoint() = 0;
    virtual esp_err_t deferred_driver_init() { return ESP_OK; }
    virtual esp_err_t attribute_handler(const esp_zb_zcl_set_attr_value_message_t* message) { return ESP_OK; }
    virtual esp_err_t initialize_endpoint(esp_zb_ep_list_t* ep_list) { return ESP_OK; }

protected:
};
