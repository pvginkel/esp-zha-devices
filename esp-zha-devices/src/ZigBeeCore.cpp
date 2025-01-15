// Adapted from https://github.com/espressif/arduino-esp32/blob/master/LICENSE.md.
/* ZigBee Core Functions */

#include "support.h"

#include "ZigBeeCore.h"

LOG_TAG(ZigBeeCore);

ZigBeeCore ZigBee = ZigBeeCore();

ZigBeeCore::ZigBeeCore() {
    _radio_config.radio_mode = ZB_RADIO_MODE_NATIVE;                   // Use the native 15.4 radio
    _host_config.host_connection_mode = ZB_HOST_CONNECTION_MODE_NONE;  // Disable host connection
    _zb_ep_list = esp_zb_ep_list_create();
    _primary_channel_mask = ESP_ZB_TRANSCEIVER_ALL_CHANNELS_MASK;
    _open_network = 0;
    _scan_status = ZB_SCAN_FAILED;
    _started = false;
    _connected = false;
    _scan_duration = 3;  // default scan duration
    _rx_on_when_idle = true;
}

ZigBeeCore::~ZigBeeCore() {}

esp_err_t ZigBeeCore::begin(esp_zb_cfg_t *role_cfg, bool erase_nvs) {
    auto err = zigbeeInit(role_cfg, erase_nvs);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ZigBeeCore begin failed");
        return err;
    }
    _role = (zigbee_role_t)role_cfg->esp_zb_role;

    return ESP_OK;
}

esp_err_t ZigBeeCore::begin(zigbee_role_t role, bool erase_nvs) {
    switch (role) {
        case ZIGBEE_COORDINATOR: {
            _role = ZIGBEE_COORDINATOR;
            esp_zb_cfg_t zb_nwk_cfg = ZIGBEE_DEFAULT_COORDINATOR_CONFIG();
            return zigbeeInit(&zb_nwk_cfg, erase_nvs);
        }
        case ZIGBEE_ROUTER: {
            _role = ZIGBEE_ROUTER;
            esp_zb_cfg_t zb_nwk_cfg = ZIGBEE_DEFAULT_ROUTER_CONFIG();
            return zigbeeInit(&zb_nwk_cfg, erase_nvs);
        }
        case ZIGBEE_END_DEVICE: {
            _role = ZIGBEE_END_DEVICE;
            esp_zb_cfg_t zb_nwk_cfg = ZIGBEE_DEFAULT_ED_CONFIG();
            return zigbeeInit(&zb_nwk_cfg, erase_nvs);
        }
        default:
            ESP_LOGE(TAG, "Invalid ZigBee Role");
            return ESP_ERR_INVALID_ARG;
    }
}

void ZigBeeCore::addEndpoint(ZigBeeEndpoint *ep) {
    _endpoints.push_back(ep);

    ESP_LOGI(TAG, "Adding endpoint: %d, device ID: 0x%04x", ep->_ep_config.endpoint, ep->_ep_config.app_device_id);
    // Register clusters and ep_list to the ZigBeeCore class's ep_list
    if (ep->_ep_config.endpoint == 0 || ep->_cluster_list == nullptr) {
        ESP_LOGE(TAG, "Endpoint config or Cluster list is not initialized, EP not added to ZigBeeCore's EP list");
        return;
    }

    esp_zb_ep_list_add_ep(_zb_ep_list, ep->_cluster_list, ep->_ep_config);
}

ZigBeeEndpoint *ZigBeeCore::getEndpointById(uint8_t endpointId) {
    for (const auto endpoint : _endpoints) {
        if (endpoint->getEndpointId() == endpointId) {
            return endpoint;
        }
    }
    return nullptr;
}

// ZigBee core init function
esp_err_t ZigBeeCore::zigbeeInit(esp_zb_cfg_t *zb_cfg, bool erase_nvs) {
    // ZigBee platform configuration
    esp_zb_platform_config_t platform_config = {
        .radio_config = _radio_config,
        .host_config = _host_config,
    };

    esp_err_t err = esp_zb_platform_config(&platform_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure ZigBee platform");
        return false;
    }

    // Initialize ZigBee stack
    ESP_LOGD(TAG, "Initialize ZigBee stack");
    esp_zb_init(zb_cfg);

    // Register all ZigBee EPs in list
    if (_endpoints.empty()) {
        ESP_LOGW(TAG, "No ZigBee EPs to register");
    } else {
        ESP_LOGD(TAG, "Register all ZigBee EPs in list");
        err = esp_zb_device_register(_zb_ep_list);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to register ZigBee EPs");
            return err;
        }

        // print the list of ZigBee EPs from ep_objects
        ESP_LOGI(TAG, "List of registered ZigBee EPs:");
        for (const auto endpoint : _endpoints) {
            ESP_LOGI(TAG, "Endpoint: %d, Device ID: 0x%04x", endpoint->_ep_config.endpoint,
                     endpoint->_ep_config.app_device_id);
        }
    }

    // Register ZigBee action & command handlers
    esp_zb_core_action_handler_register([](esp_zb_core_action_callback_id_t callback_id, const void *message) {
        return ZigBee.zbActionHandler(callback_id, message);
    });
    esp_zb_raw_command_handler_register([](uint8_t bufid) { return ZigBee.zbCommandHandler(bufid); });

    err = esp_zb_set_primary_network_channel_set(_primary_channel_mask);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set primary network channel mask");
        return err;
    }

    // Erase NVRAM before creating connection to new Coordinator
    if (erase_nvs) {
        esp_zb_nvram_erase_at_start(true);
    }

    // Create ZigBee task and start ZigBee stack
    xTaskCreate(zbTask, "Zigbee_main", 4096, NULL, 5, NULL);

    return ESP_OK;
}

void ZigBeeCore::zbTask(void *pvParameters) {
#if CONFIG_ZB_DEBUG_MODE
    esp_zb_set_trace_level_mask(ESP_ZB_TRACE_LEVEL_DEBUG, ESP_ZB_TRACE_SUBSYSTEM_MAC | ESP_ZB_TRACE_SUBSYSTEM_APP);
#endif

    esp_zb_bdb_set_scan_duration(ZigBee.getScanDuration());

    /* initialize ZigBee stack */
    ESP_ERROR_CHECK(esp_zb_start(false));

    esp_zb_stack_main_loop();
}

void ZigBeeCore::setRadioConfig(esp_zb_radio_config_t config) { _radio_config = config; }

esp_zb_radio_config_t ZigBeeCore::getRadioConfig() { return _radio_config; }

void ZigBeeCore::setHostConfig(esp_zb_host_config_t config) { _host_config = config; }

esp_zb_host_config_t ZigBeeCore::getHostConfig() { return _host_config; }

void ZigBeeCore::setPrimaryChannelMask(uint32_t mask) { _primary_channel_mask = mask; }

void ZigBeeCore::setScanDuration(uint8_t duration) {
    if (duration < 1 || duration > 4) {
        ESP_LOGE(TAG, "Invalid scan duration, must be between 1 and 4");
        return;
    }
    _scan_duration = duration;
}

void ZigBeeCore::setRebootOpenNetwork(uint8_t time) { _open_network = time; }

void ZigBeeCore::openNetwork(uint8_t time) {
    ESP_ERROR_ASSERT(started());

    ESP_LOGV(TAG, "Opening network for joining for %d seconds", time);
    esp_zb_bdb_open_network(time);
}

// This is declared by the ZigBee stack as an undefined external
// reference. It's called by the ZigBee stack.
extern "C" void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_struct) {
    const auto p_sg_p = signal_struct->p_app_signal;
    const auto err_status = signal_struct->esp_err_status;
    const auto sig_type = (esp_zb_app_signal_type_t)*p_sg_p;

    // main switch
    switch (sig_type) {
        case ESP_ZB_ZDO_SIGNAL_SKIP_STARTUP:  // Common
            ESP_LOGI(TAG, "ZigBee stack initialized");
            esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_INITIALIZATION);
            break;

        case ESP_ZB_BDB_SIGNAL_DEVICE_FIRST_START:  // Common
        case ESP_ZB_BDB_SIGNAL_DEVICE_REBOOT:       // Common
            if (err_status == ESP_OK) {
                if (esp_zb_bdb_is_factory_new()) {
                    ESP_LOGI(TAG, "Device started up in factory-reset mode");

                    // Role specific code
                    if ((zigbee_role_t)ZigBee.getRole() == ZIGBEE_COORDINATOR) {
                        ESP_LOGI(TAG, "Start network formation");
                        esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_FORMATION);
                    } else {
                        ESP_LOGI(TAG, "Start network steering");
                        esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);

                        ZigBee._started = true;
                        ZigBee._has_connected.call();
                    }
                } else {
                    ESP_LOGI(TAG, "Device rebooted");

                    ZigBee._started = true;
                    ZigBee._has_connected.call();

                    if ((zigbee_role_t)ZigBee.getRole() == ZIGBEE_COORDINATOR && ZigBee._open_network > 0) {
                        ESP_LOGI(TAG, "Opening network for joining for %d seconds", ZigBee._open_network);
                        esp_zb_bdb_open_network(ZigBee._open_network);
                    } else {
                        ZigBee._connected = true;
                    }
                }
            } else {
                /* commissioning failed */
                ESP_LOGW(TAG, "Commissioning failed, error %s, trying again...", esp_err_to_name(err_status));
                esp_zb_scheduler_alarm(
                    [](uint8_t param) { ESP_ERROR_CHECK(esp_zb_bdb_start_top_level_commissioning(param)); },
                    ESP_ZB_BDB_MODE_INITIALIZATION, 500);
            }
            break;

        case ESP_ZB_BDB_SIGNAL_FORMATION:  // Coordinator
            if ((zigbee_role_t)ZigBee.getRole() == ZIGBEE_COORDINATOR) {
                if (err_status == ESP_OK) {
                    esp_zb_ieee_addr_t extended_pan_id;
                    esp_zb_get_extended_pan_id(extended_pan_id);
                    ESP_LOGI(
                        TAG,
                        "Formed network successfully (Extended PAN ID: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x, PAN "
                        "ID: 0x%04hx, Channel:%d, Short Address: 0x%04hx)",
                        extended_pan_id[7], extended_pan_id[6], extended_pan_id[5], extended_pan_id[4],
                        extended_pan_id[3], extended_pan_id[2], extended_pan_id[1], extended_pan_id[0],
                        esp_zb_get_pan_id(), esp_zb_get_current_channel(), esp_zb_get_short_address());
                    esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);
                } else {
                    ESP_LOGI(TAG, "Restart network formation (status: %s)", esp_err_to_name(err_status));
                    esp_zb_scheduler_alarm(
                        [](uint8_t param) { ESP_ERROR_CHECK(esp_zb_bdb_start_top_level_commissioning(param)); },
                        ESP_ZB_BDB_MODE_NETWORK_FORMATION, 1000);
                }
            }
            break;

        case ESP_ZB_BDB_SIGNAL_STEERING:  // Router and End Device
            if ((zigbee_role_t)ZigBee.getRole() == ZIGBEE_COORDINATOR) {
                if (err_status == ESP_OK) {
                    ESP_LOGI(TAG, "Network steering started");
                }
                ZigBee._started = true;
                ZigBee._has_connected.call();
            } else {
                if (err_status == ESP_OK) {
                    esp_zb_ieee_addr_t extended_pan_id;
                    esp_zb_get_extended_pan_id(extended_pan_id);
                    ESP_LOGI(
                        TAG,
                        "Joined network successfully (Extended PAN ID: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x, PAN "
                        "ID: 0x%04hx, Channel:%d, Short Address: 0x%04hx)",
                        extended_pan_id[7], extended_pan_id[6], extended_pan_id[5], extended_pan_id[4],
                        extended_pan_id[3], extended_pan_id[2], extended_pan_id[1], extended_pan_id[0],
                        esp_zb_get_pan_id(), esp_zb_get_current_channel(), esp_zb_get_short_address());
                    ZigBee._started = true;
                    ZigBee._connected = true;
                    ZigBee._has_connected.call();
                } else {
                    ESP_LOGI(TAG, "Network steering was not successful (status: %s)", esp_err_to_name(err_status));
                    esp_zb_scheduler_alarm(
                        [](uint8_t param) { ESP_ERROR_CHECK(esp_zb_bdb_start_top_level_commissioning(param)); },
                        ESP_ZB_BDB_MODE_NETWORK_STEERING, 1000);
                }
            }
            break;

        case ESP_ZB_ZDO_SIGNAL_DEVICE_ANNCE: {  // Coordinator
            ESP_LOGW(TAG, "ESP_ZB_ZDO_SIGNAL_DEVICE_ANNCE has not been implemented");
            break;
        }

        case ESP_ZB_NWK_SIGNAL_PERMIT_JOIN_STATUS:  // Coordinator
            if ((zigbee_role_t)ZigBee.getRole() == ZIGBEE_COORDINATOR) {
                if (err_status == ESP_OK) {
                    if (*(uint8_t *)esp_zb_app_signal_get_params(p_sg_p)) {
                        ESP_LOGI(TAG, "Network(0x%04hx) is open for %d seconds", esp_zb_get_pan_id(),
                                 *(uint8_t *)esp_zb_app_signal_get_params(p_sg_p));
                    } else {
                        ESP_LOGI(TAG, "Network(0x%04hx) closed, devices joining not allowed.", esp_zb_get_pan_id());
                    }
                }
            }
            break;

        case ESP_ZB_ZDO_SIGNAL_LEAVE:  // End Device + Router
            // Device was removed from the network, factory reset the device
            if (ZigBee.getRole() != ZIGBEE_COORDINATOR) {
                ZigBee.factoryReset();
            }
            break;

        default:
            ESP_LOGV(TAG, "ZDO signal: %s (0x%x), status: %s", esp_zb_zdo_signal_to_string(sig_type), sig_type,
                     esp_err_to_name(err_status));
            break;
    }
}

void ZigBeeCore::factoryReset() {
    ESP_LOGV(TAG, "Factory resetting ZigBee stack, device will reboot");
    esp_zb_factory_reset();
}

void ZigBeeCore::scanCompleteCallback(esp_zb_zdp_status_t zdo_status, uint8_t count,
                                      esp_zb_network_descriptor_t *nwk_descriptor) {
    ESP_LOGV(TAG, "ZigBee network scan complete");
    if (zdo_status == ESP_ZB_ZDP_STATUS_SUCCESS) {
        ESP_LOGV(TAG, "Found %d networks", count);
        // print ZigBee networks
        for (int i = 0; i < count; i++) {
            ESP_LOGV(TAG,
                     "Network %d: PAN ID: 0x%04hx, Permit Joining: %s, Extended PAN ID: "
                     "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x, Channel: %d, Router Capacity: %s, End "
                     "Device Capacity: %s",
                     i, nwk_descriptor[i].short_pan_id, nwk_descriptor[i].permit_joining ? "Yes" : "No",
                     nwk_descriptor[i].extended_pan_id[7], nwk_descriptor[i].extended_pan_id[6],
                     nwk_descriptor[i].extended_pan_id[5], nwk_descriptor[i].extended_pan_id[4],
                     nwk_descriptor[i].extended_pan_id[3], nwk_descriptor[i].extended_pan_id[2],
                     nwk_descriptor[i].extended_pan_id[1], nwk_descriptor[i].extended_pan_id[0],
                     nwk_descriptor[i].logic_channel, nwk_descriptor[i].router_capacity ? "Yes" : "No",
                     nwk_descriptor[i].end_device_capacity ? "Yes" : "No");
        }
        // save scan result and update scan status
        // copy network descriptor to _scan_result to keep the data after the callback
        ZigBee._scan_result = (esp_zb_network_descriptor_t *)malloc(count * sizeof(esp_zb_network_descriptor_t));
        memcpy(ZigBee._scan_result, nwk_descriptor, count * sizeof(esp_zb_network_descriptor_t));
        ZigBee._scan_status = count;
    } else {
        ESP_LOGE(TAG, "Failed to scan ZigBee network (status: 0x%x)", zdo_status);
        ZigBee._scan_status = ZB_SCAN_FAILED;
        ZigBee._scan_result = nullptr;
    }
}

void ZigBeeCore::scanNetworks(uint32_t channel_mask, uint8_t scan_duration) {
    if (!started()) {
        ESP_LOGE(TAG, "ZigBee stack is not started, cannot scan networks");
        return;
    }
    ESP_LOGV(TAG, "Scanning ZigBee networks");
    esp_zb_zdo_active_scan_request(channel_mask, scan_duration, scanCompleteCallback);
    _scan_status = ZB_SCAN_RUNNING;
}

int16_t ZigBeeCore::scanComplete() { return _scan_status; }

zigbee_scan_result_t *ZigBeeCore::getScanResult() { return _scan_result; }

void ZigBeeCore::scanDelete() {
    if (_scan_result != nullptr) {
        free(_scan_result);
        _scan_result = nullptr;
    }
    _scan_status = ZB_SCAN_FAILED;
}

esp_err_t ZigBeeCore::zbActionHandler(esp_zb_core_action_callback_id_t callback_id, const void *message) {
    ESP_LOGI(TAG, "Received action %d", (int)callback_id);

    switch (callback_id) {
        case ESP_ZB_CORE_SET_ATTR_VALUE_CB_ID:
            return zbAttributeSetHandler((esp_zb_zcl_set_attr_value_message_t *)message);
        case ESP_ZB_CORE_REPORT_ATTR_CB_ID:
            return zbAttributeReportingHandler((esp_zb_zcl_report_attr_message_t *)message);
        case ESP_ZB_CORE_CMD_READ_ATTR_RESP_CB_ID:
            return zbCmdReadAttributeResponseHandler((esp_zb_zcl_cmd_read_attr_resp_message_t *)message);
        case ESP_ZB_CORE_CMD_REPORT_CONFIG_RESP_CB_ID:
            return zbConfigureReportingResponseHandler((esp_zb_zcl_cmd_config_report_resp_message_t *)message);
        case ESP_ZB_CORE_CMD_DEFAULT_RESP_CB_ID:
            return zbCmdDefaultResponseHandler((esp_zb_zcl_cmd_default_resp_message_t *)message);
        default:
            ESP_LOGW(TAG, "Receive unhandled ZigBee action(0x%x) callback", callback_id);
            return ESP_OK;
    }
}

esp_err_t ZigBeeCore::zbAttributeSetHandler(const esp_zb_zcl_set_attr_value_message_t *message) {
    if (!message) {
        ESP_LOGE(TAG, "Empty message");
    }
    if (message->info.status != ESP_ZB_ZCL_STATUS_SUCCESS) {
        ESP_LOGE(TAG, "Received message: error status(%d)", message->info.status);
    }

    ESP_LOGV(TAG, "Received message: endpoint(%d), cluster(0x%x), attribute(0x%x), data size(%d)",
             message->info.dst_endpoint, message->info.cluster, message->attribute.id, message->attribute.data.size);

    // List through all ZigBee EPs and call the callback function, with the message
    const auto endpoint = getEndpointById(message->info.dst_endpoint);
    if (endpoint) {
        return endpoint->zbAttributeSet(message);
    }

    return ESP_OK;
}

esp_err_t ZigBeeCore::zbAttributeReportingHandler(const esp_zb_zcl_report_attr_message_t *message) {
    if (!message) {
        ESP_LOGE(TAG, "Empty message");
    }
    if (message->status != ESP_ZB_ZCL_STATUS_SUCCESS) {
        ESP_LOGE(TAG, "Received message: error status(%d)", message->status);
    }
    ESP_LOGV(TAG, "Received report from address(0x%x) src endpoint(%d) to dst endpoint(%d) cluster(0x%x)",
             message->src_address.u.short_addr, message->src_endpoint, message->dst_endpoint, message->cluster);

    const auto endpoint = getEndpointById(message->dst_endpoint);
    if (endpoint) {
        return endpoint->zbAttributeRead(message->cluster, &message->attribute);
    }

    return ESP_OK;
}

esp_err_t ZigBeeCore::zbCmdReadAttributeResponseHandler(const esp_zb_zcl_cmd_read_attr_resp_message_t *message) {
    if (!message) {
        ESP_LOGE(TAG, "Empty message");
    }
    if (message->info.status != ESP_ZB_ZCL_STATUS_SUCCESS) {
        ESP_LOGE(TAG, "Received message: error status(%d)", message->info.status);
    }
    ESP_LOGV(TAG, "Read attribute response: from address(0x%x) src endpoint(%d) to dst endpoint(%d) cluster(0x%x)",
             message->info.src_address.u.short_addr, message->info.src_endpoint, message->info.dst_endpoint,
             message->info.cluster);

    const auto endpoint = getEndpointById(message->info.dst_endpoint);
    if (endpoint) {
        esp_zb_zcl_read_attr_resp_variable_t *variable = message->variables;

        while (variable) {
            ESP_LOGV(TAG, "Read attribute response: status(%d), cluster(0x%x), attribute(0x%x), type(0x%x), value(%d)",
                     variable->status, message->info.cluster, variable->attribute.id, variable->attribute.data.type,
                     variable->attribute.data.value ? *(uint8_t *)variable->attribute.data.value : 0);

            if (variable->status == ESP_ZB_ZCL_STATUS_SUCCESS) {
                return endpoint->zbAttributeRead(message->info.cluster, &variable->attribute);
            }
            variable = variable->next;
        }
    }

    return ESP_OK;
}

esp_err_t ZigBeeCore::zbConfigureReportingResponseHandler(const esp_zb_zcl_cmd_config_report_resp_message_t *message) {
    if (!message) {
        ESP_LOGE(TAG, "Empty message");
    }
    if (message->info.status != ESP_ZB_ZCL_STATUS_SUCCESS) {
        ESP_LOGE(TAG, "Received message: error status(%d)", message->info.status);
    }
    esp_zb_zcl_config_report_resp_variable_t *variable = message->variables;
    while (variable) {
        ESP_LOGV(TAG, "Configure report response: status(%d), cluster(0x%x), direction(0x%x), attribute(0x%x)",
                 variable->status, message->info.cluster, variable->direction, variable->attribute_id);
        variable = variable->next;
    }
    return ESP_OK;
}

esp_err_t ZigBeeCore::zbCmdDefaultResponseHandler(const esp_zb_zcl_cmd_default_resp_message_t *message) {
    if (!message) {
        ESP_LOGE(TAG, "Empty message");
    }
    if (message->info.status != ESP_ZB_ZCL_STATUS_SUCCESS) {
        ESP_LOGE(TAG, "Received message: error status(%d)", message->info.status);
    }
    ESP_LOGV(TAG,
             "Received default response: from address(0x%x), src_endpoint(%d) to dst_endpoint(%d), cluster(0x%x) with "
             "status 0x%x",
             message->info.src_address.u.short_addr, message->info.src_endpoint, message->info.dst_endpoint,
             message->info.cluster, message->status_code);
    return ESP_OK;
}

bool ZigBeeCore::zbCommandHandler(uint8_t bufid) {
    uint8_t response_buf[MAX_FRAME_DATA_SIZE];

    zb_zcl_parsed_hdr_t *cmd_info = ZB_BUF_GET_PARAM(bufid, zb_zcl_parsed_hdr_t);

    ZigBeeStream request((uint8_t *)zb_buf_begin(bufid), zb_buf_len(bufid));
    ZigBeeStream response(response_buf, MAX_FRAME_DATA_SIZE);

    ESP_LOGV(TAG, "Received command: cluster(0x%x), command(0x%x)", cmd_info->cluster_id, cmd_info->cmd_id);

    // List through all ZigBee EPs and call the callback function, with the message
    const auto endpoint = getEndpointById(cmd_info->addr_data.common_data.dst_endpoint);
    if (endpoint) {
        auto err = endpoint->zbCommand(cmd_info, request, response);
        if (err == ESP_ERR_NOT_SUPPORTED) {
            return false;
        }
        ESP_ERROR_CHECK_WITHOUT_ABORT(err);
        return true;
    }

    return false;
}
