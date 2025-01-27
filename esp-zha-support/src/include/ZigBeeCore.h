#pragma once

#include <vector>

#include "Callback.h"
#include "esp_zigbee_core.h"
#include "zdo/esp_zigbee_zdo_common.h"

class ZigBeeEndpoint;

typedef esp_zb_network_descriptor_t zigbee_scan_result_t;

// enum of ZigBee Roles
typedef enum {
    ZIGBEE_COORDINATOR = 0,
    ZIGBEE_ROUTER = 1,
    ZIGBEE_END_DEVICE = 2,
} zigbee_role_t;

#define ZB_SCAN_RUNNING (-1)
#define ZB_SCAN_FAILED (-2)

#define ZIGBEE_DEFAULT_ED_CONFIG()                                   \
    {                                                                \
        .esp_zb_role = ESP_ZB_DEVICE_TYPE_ED,                        \
        .install_code_policy = false,                                \
        .nwk_cfg =                                                   \
            {                                                        \
                .zed_cfg =                                           \
                    {                                                \
                        .ed_timeout = ESP_ZB_ED_AGING_TIMEOUT_64MIN, \
                        .keep_alive = 3000,                          \
                    },                                               \
            },                                                       \
    }

#define ZIGBEE_DEFAULT_ROUTER_CONFIG()            \
    {                                             \
        .esp_zb_role = ESP_ZB_DEVICE_TYPE_ROUTER, \
        .install_code_policy = false,             \
        .nwk_cfg =                                \
            {                                     \
                .zczr_cfg =                       \
                    {                             \
                        .max_children = 10,       \
                    },                            \
            },                                    \
    }

#define ZIGBEE_DEFAULT_COORDINATOR_CONFIG()            \
    {                                                  \
        .esp_zb_role = ESP_ZB_DEVICE_TYPE_COORDINATOR, \
        .install_code_policy = false,                  \
        .nwk_cfg =                                     \
            {                                          \
                .zczr_cfg =                            \
                    {                                  \
                        .max_children = 10,            \
                    },                                 \
            },                                         \
    }

class ZigBeeCore {
    esp_zb_radio_config_t _radio_config;
    esp_zb_host_config_t _host_config;
    uint32_t _primary_channel_mask;
    int16_t _scan_status;
    uint8_t _scan_duration;
    bool _rx_on_when_idle;
    std::vector<ZigBeeEndpoint *> _ep_objects;

    esp_zb_ep_list_t *_zb_ep_list;
    zigbee_role_t _role;
    bool _started;
    Callback<void> _started_cb;

    uint8_t _open_network;
    zigbee_scan_result_t *_scan_result;

    esp_err_t zigbeeInit(esp_zb_cfg_t *zb_cfg, bool erase_nvs);
    void startupComplete();
    void scanCompleteCallback(esp_zb_zdp_status_t zdo_status, uint8_t count,
                              esp_zb_network_descriptor_t *nwk_descriptor);
    void searchBindings();
    void bindingTableCb(const esp_zb_zdo_binding_table_info_t *table_info, void *user_ctx);
    esp_err_t zb_action_handler(esp_zb_core_action_callback_id_t callback_id, const void *message);
    esp_err_t zb_attribute_set_handler(const esp_zb_zcl_set_attr_value_message_t *message);
    esp_err_t zb_attribute_reporting_handler(const esp_zb_zcl_report_attr_message_t *message);
    esp_err_t zb_cmd_read_attr_resp_handler(const esp_zb_zcl_cmd_read_attr_resp_message_t *message);
    esp_err_t zb_configure_report_resp_handler(const esp_zb_zcl_cmd_config_report_resp_message_t *message);
    esp_err_t zb_cmd_default_resp_handler(const esp_zb_zcl_cmd_default_resp_message_t *message);
    void esp_zb_task();
    void zb_app_signal_handler(esp_zb_app_signal_t *signal_struct);

public:
    ZigBeeCore();
    ~ZigBeeCore() {}

    esp_err_t begin(zigbee_role_t role = ZIGBEE_END_DEVICE, bool erase_nvs = false);
    esp_err_t begin(esp_zb_cfg_t *role_cfg, bool erase_nvs = false);

    bool started() { return _started; }
    void onStarted(std::function<void(void)> func) { _started_cb.add(func); }
    zigbee_role_t getRole() { return _role; }

    void addEndpoint(ZigBeeEndpoint *ep);

    void setRadioConfig(esp_zb_radio_config_t config);
    esp_zb_radio_config_t getRadioConfig();

    void setHostConfig(esp_zb_host_config_t config);
    esp_zb_host_config_t getHostConfig();

    void setPrimaryChannelMask(uint32_t mask);  // By default all channels are scanned (11-26) -> mask 0x07FFF800
    void setScanDuration(uint8_t duration);     // Can be set from 1 - 4. 1 is fastest, 4 is slowest
    uint8_t getScanDuration() { return _scan_duration; }

    void setRxOnWhenIdle(bool rx_on_when_idle) { _rx_on_when_idle = rx_on_when_idle; }
    bool getRxOnWhenIdle() { return _rx_on_when_idle; }

    void setRebootOpenNetwork(uint8_t time);
    void openNetwork(uint8_t time);

    // scan_duration Time spent scanning each channel, in units of ((1 << scan_duration) + 1) * a beacon time. (15.36
    // microseconds)
    void scanNetworks(uint32_t channel_mask = ESP_ZB_TRANSCEIVER_ALL_CHANNELS_MASK, uint8_t scan_duration = 5);
    // ZigBee scan complete status check, -2: failed or not started, -1: running, 0: no networks found, >0: number of
    // networks found
    int16_t scanComplete();
    zigbee_scan_result_t *getScanResult();
    void scanDelete();

    void factoryReset();

    // Friend function declaration to allow access to private members
    friend void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_struct);
};
