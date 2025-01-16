#include "support.h"

#include "ZigBeeApp.h"

#include "ZigBeeStream.h"

LOG_TAG(ZigBeeApp);

#define INSTALL_CODE_POLICY false                       /* enable the install code policy for security */
#define ZED_AGING_TIMEOUT ESP_ZB_ED_AGING_TIMEOUT_64MIN /* aging timeout of device */
#define ZED_KEEP_ALIVE 3000                             /* 3000 millisecond */
#define ESP_ZB_PRIMARY_CHANNEL_MASK \
    ESP_ZB_TRANSCEIVER_ALL_CHANNELS_MASK /* Zigbee primary channel mask use in the example */

static ZigBeeApp* APP_INSTANCE = nullptr;

ZigBeeApp::ZigBeeApp() {
    ESP_ERROR_ASSERT(APP_INSTANCE == nullptr);

    APP_INSTANCE = this;
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

void ZigBeeApp::add_endpoint(ZigBeeEndpoint* endpoint) { _endpoints.push_back(endpoint); }

void ZigBeeApp::task() {
#if CONFIG_ZB_DEBUG_MODE
    esp_zb_set_trace_level_mask(ESP_ZB_TRACE_LEVEL_CRITICAL,
                                ESP_ZB_TRACE_SUBSYSTEM_ZDO | ESP_ZB_TRACE_SUBSYSTEM_ZCL | ESP_ZB_TRACE_SUBSYSTEM_APP);
#endif

    // Initialize ZigBee stack.

    esp_zb_cfg_t zigbee_config = {
        .esp_zb_role = ESP_ZB_DEVICE_TYPE_ED,
        .install_code_policy = INSTALL_CODE_POLICY,
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

    for (const auto endpoint : _endpoints) {
        auto ep_list = endpoint->build_endpoint();

        ESP_ERROR_CHECK(esp_zb_device_register(ep_list));

        ESP_ERROR_CHECK(endpoint->initialize_endpoint(ep_list));
    }

    esp_zb_core_action_handler_register([](esp_zb_core_action_callback_id_t callback_id, const void* message) {
        return APP_INSTANCE->action_handler(callback_id, message);
    });
    esp_zb_raw_command_handler_register([](uint8_t bufid) { return APP_INSTANCE->command_handler(bufid); });
    ESP_ERROR_CHECK(esp_zb_set_primary_network_channel_set(ESP_ZB_PRIMARY_CHANNEL_MASK));

    ESP_ERROR_CHECK(esp_zb_start(false));

    esp_zb_stack_main_loop();
}

esp_err_t ZigBeeApp::deferred_driver_init() {
    for (const auto endpoint : _endpoints) {
        const auto err = endpoint->deferred_driver_init();
        if (err != ESP_OK) {
            return err;
        }
    }
    return ESP_OK;
}

esp_err_t ZigBeeApp::action_handler(esp_zb_core_action_callback_id_t callback_id, const void* message) {
    switch (callback_id) {
        case ESP_ZB_CORE_SET_ATTR_VALUE_CB_ID:
            return attribute_handler((esp_zb_zcl_set_attr_value_message_t*)message);

        default:
            ESP_LOGW(TAG, "Receive Zigbee action(0x%x) callback", callback_id);
            return ESP_OK;
    }
}

esp_err_t ZigBeeApp::attribute_handler(const esp_zb_zcl_set_attr_value_message_t* message) {
    if (!message) {
        ESP_LOGE(TAG, "Empty attribute message");
        return ESP_FAIL;
    }
    if (message->info.status != ESP_ZB_ZCL_STATUS_SUCCESS) {
        ESP_LOGE(TAG, "Received message: error status(%d)", message->info.status);
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Received message: endpoint(%d), cluster(0x%x), attribute(0x%x), data size(%d)",
             message->info.dst_endpoint, message->info.cluster, message->attribute.id, message->attribute.data.size);

    const auto endpoint = get_endpoint_by_id(message->info.dst_endpoint);
    if (endpoint) {
        return endpoint->attribute_handler(message);
    }

    return ESP_OK;
}

ZigBeeEndpoint* ZigBeeApp::get_endpoint_by_id(uint8_t endpoint_id) {
    for (const auto endpoint : _endpoints) {
        if (endpoint->get_endpoint_id() == endpoint_id) {
            return endpoint;
        }
    }

    return nullptr;
}

#define MAX_FRAME_DATA_SIZE 110

bool ZigBeeApp::command_handler(uint8_t bufid) {
    uint8_t response_buf[MAX_FRAME_DATA_SIZE];

    zb_zcl_parsed_hdr_t* cmd_info = ZB_BUF_GET_PARAM(bufid, zb_zcl_parsed_hdr_t);

    ZigBeeStream request((uint8_t*)zb_buf_begin(bufid), zb_buf_len(bufid));
    ZigBeeStream response(response_buf, MAX_FRAME_DATA_SIZE);

    ESP_LOGV(TAG, "Received command: cluster(0x%x), command(0x%x)", cmd_info->cluster_id, cmd_info->cmd_id);

    // List through all ZigBee EPs and call the callback function, with the message
    const auto endpoint = get_endpoint_by_id(cmd_info->addr_data.common_data.dst_endpoint);
    if (endpoint) {
        auto err = endpoint->handle_command(cmd_info, request, response);
        if (err == ESP_ERR_NOT_SUPPORTED) {
            return false;
        }
        ESP_ERROR_CHECK_WITHOUT_ABORT(err);
        return true;
    }

    return false;
}

extern "C" void esp_zb_app_signal_handler(esp_zb_app_signal_t* signal_struct) {
    auto err_status = signal_struct->esp_err_status;
    auto sig_type = (esp_zb_app_signal_type_t)*signal_struct->p_app_signal;

    switch (sig_type) {
        case ESP_ZB_ZDO_SIGNAL_SKIP_STARTUP:
            ESP_LOGI(TAG, "Initialize Zigbee stack");
            ESP_ERROR_CHECK(esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_INITIALIZATION));
            break;

        case ESP_ZB_BDB_SIGNAL_DEVICE_FIRST_START:
        case ESP_ZB_BDB_SIGNAL_DEVICE_REBOOT:
            if (err_status == ESP_OK) {
                ESP_LOGI(TAG, "Deferred driver initialization");
                ESP_ERROR_CHECK(APP_INSTANCE->deferred_driver_init());

                if (esp_zb_bdb_is_factory_new()) {
                    ESP_LOGI(TAG, "Starting from factory reset mode");
                    esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);
                } else {
                    ESP_LOGI(TAG, "Device rebooted");
                }

                APP_INSTANCE->_has_connected.call();
            } else {
                /* commissioning failed */
                ESP_LOGW(TAG, "Initialization failed (status: %s); retrying", esp_err_to_name(err_status));

                esp_zb_scheduler_alarm(
                    [](uint8_t mode_mask) { ESP_ERROR_CHECK(esp_zb_bdb_start_top_level_commissioning(mode_mask)); },
                    ESP_ZB_BDB_MODE_INITIALIZATION, 1000);
            }
            break;

        case ESP_ZB_BDB_SIGNAL_STEERING:
            if (err_status == ESP_OK) {
                esp_zb_ieee_addr_t extended_pan_id;
                esp_zb_get_extended_pan_id(extended_pan_id);
                ESP_LOGI(TAG,
                         "Joined network successfully (Extended PAN ID: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x, PAN "
                         "ID: 0x%04hx, Channel:%d, Short Address: 0x%04hx)",
                         extended_pan_id[7], extended_pan_id[6], extended_pan_id[5], extended_pan_id[4],
                         extended_pan_id[3], extended_pan_id[2], extended_pan_id[1], extended_pan_id[0],
                         esp_zb_get_pan_id(), esp_zb_get_current_channel(), esp_zb_get_short_address());
            } else {
                ESP_LOGI(TAG, "Network steering was not successful (status: %s)", esp_err_to_name(err_status));
                esp_zb_scheduler_alarm(
                    [](uint8_t mode_mask) {
                        ESP_LOGI(TAG, "Start ZigBee commissioning");

                        ESP_ERROR_CHECK(esp_zb_bdb_start_top_level_commissioning(mode_mask));
                    },
                    ESP_ZB_BDB_MODE_NETWORK_STEERING, 1000);
            }
            break;

        case ESP_ZB_ZDO_SIGNAL_PRODUCTION_CONFIG_READY:
            // We're not burning a ZDO config. See
            // https://github.com/espressif/esp-zigbee-sdk/blob/c2d1836/tools/mfg_tool/README.md.
            break;

        default:
            ESP_LOG_LEVEL_LOCAL((err_status == ESP_OK ? ESP_LOG_INFO : ESP_LOG_WARN), TAG,
                                "ZDO signal: %s (0x%x), status: %s", esp_zb_zdo_signal_to_string(sig_type), sig_type,
                                esp_err_to_name(err_status));
            break;
    }
}
