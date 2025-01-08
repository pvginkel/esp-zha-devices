#pragma once

#include "esp_zigbee_core.h"

class ZigBeeApp {
    static ZigBeeApp* instance;

public:
    ZigBeeApp();
    void begin();

protected:
    virtual esp_zb_ep_list_t* build_endpoint() = 0;

private:
    void task();
    esp_err_t action_handler(esp_zb_core_action_callback_id_t callback_id, const void* message);
};
