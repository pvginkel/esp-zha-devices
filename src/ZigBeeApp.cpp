#include "support.h"

#include "ZigBeeApp.h"

#define INSTALLCODE_POLICY_ENABLE false                 /* enable the install code policy for security */
#define ZED_AGING_TIMEOUT ESP_ZB_ED_AGING_TIMEOUT_64MIN /* aging timeout of device */
#define ZED_KEEP_ALIVE 3000                             /* 3000 millisecond */
#define ESP_ZB_PRIMARY_CHANNEL_MASK \
    ESP_ZB_TRANSCEIVER_ALL_CHANNELS_MASK /* Zigbee primary channel mask use in the example */

ZigBeeApp* ZigBeeApp::instance = nullptr;

ZigBeeApp::ZigBeeApp() {
    ESP_ERROR_ASSERT(instance == nullptr);

    instance = this;
}

void ZigBeeApp::begin() {
    esp_zb_platform_config_t config = {
        .radio_config =
            {
                .radio_mode = ZB_RADIO_MODE_NATIVE,
            },
        .host_config =
            {
                .host_connection_mode = ZB_HOST_CONNECTION_MODE_NONE,
            },
    };

    ESP_ERROR_CHECK(esp_zb_platform_config(&config));

    xTaskCreate([](void* param) { ((ZigBeeApp*)param)->task(); }, "ZigBeeApp::task", 4096, this, 5, nullptr);
}

void ZigBeeApp::task() {
    // Initialize ZigBee stack.

    esp_zb_cfg_t zigbee_config = {
        .esp_zb_role = ESP_ZB_DEVICE_TYPE_ED,
        .install_code_policy = INSTALLCODE_POLICY_ENABLE,
        .nwk_cfg =
            {
                .zed_cfg =
                    {
                        .ed_timeout = ZED_AGING_TIMEOUT,
                        .keep_alive = ZED_KEEP_ALIVE,
                    },
            },
    };
    esp_zb_init(&zigbee_config);

    auto endpoint = build_endpoint();

    ESP_ERROR_CHECK(esp_zb_device_register(endpoint));
    esp_zb_core_action_handler_register([](esp_zb_core_action_callback_id_t callback_id, const void* message) {
        return instance->action_handler(callback_id, message);
    });
    ESP_ERROR_CHECK(esp_zb_set_primary_network_channel_set(ESP_ZB_PRIMARY_CHANNEL_MASK));
    ESP_ERROR_CHECK(esp_zb_start(false));

    esp_zb_stack_main_loop();
}

esp_err_t ZigBeeApp::action_handler(esp_zb_core_action_callback_id_t callback_id, const void* message) {
    return ESP_OK;
}