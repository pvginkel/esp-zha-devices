#pragma once

#include <vector>

#include "Callback.h"
#include "ZigBeeEndpoint.h"
#include "esp_zigbee_core.h"

class ZigBeeApp {
    std::vector<ZigBeeEndpoint*> _endpoints;
    Callback<void> _has_connected;

public:
    ZigBeeApp();
    void begin();
    void add_endpoint(ZigBeeEndpoint* endpoint);
    void on_has_connected(std::function<void(void)> func) { _has_connected.add(func); }

private:
    esp_err_t deferred_driver_init();
    esp_err_t action_handler(esp_zb_core_action_callback_id_t callback_id, const void* message);
    esp_err_t attribute_handler(const esp_zb_zcl_set_attr_value_message_t* message);

    void task();

    friend void esp_zb_app_signal_handler(esp_zb_app_signal_t* signal_struct);
};
