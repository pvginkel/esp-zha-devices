#pragma once

#include "esp_zigbee_core.h"

class ZigBeeApp {
public:
    ZigBeeApp();
    void begin();

protected:
    virtual esp_zb_ep_list_t* build_endpoint() = 0;
    virtual esp_err_t deferred_driver_init() { return ESP_OK; }
    virtual esp_err_t action_handler(esp_zb_core_action_callback_id_t callback_id, const void* message);
    virtual esp_err_t attribute_handler(const esp_zb_zcl_set_attr_value_message_t* message);

private:
    void task();

    friend void esp_zb_app_signal_handler(esp_zb_app_signal_t* signal_struct);
};
