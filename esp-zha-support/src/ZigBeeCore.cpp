#include "support.h"

#include "ZigBeeCore.h"

LOG_TAG(ZigBeeCore);

static esp_err_t zb_attribute_set_handler(const esp_zb_zcl_set_attr_value_message_t *message);
static esp_err_t zb_attribute_reporting_handler(const esp_zb_zcl_report_attr_message_t *message);
static esp_err_t zb_cmd_read_attr_resp_handler(const esp_zb_zcl_cmd_read_attr_resp_message_t *message);
static esp_err_t zb_configure_report_resp_handler(const esp_zb_zcl_cmd_config_report_resp_message_t *message);
static esp_err_t zb_cmd_default_resp_handler(const esp_zb_zcl_cmd_default_resp_message_t *message);

// ZigBee action handlers
[[maybe_unused]]
static esp_err_t zb_action_handler(esp_zb_core_action_callback_id_t callback_id, const void *message) {
    esp_err_t ret = ESP_OK;
    switch (callback_id) {
        case ESP_ZB_CORE_SET_ATTR_VALUE_CB_ID:
            ret = zb_attribute_set_handler((esp_zb_zcl_set_attr_value_message_t *)message);
            break;
        case ESP_ZB_CORE_REPORT_ATTR_CB_ID:
            ret = zb_attribute_reporting_handler((esp_zb_zcl_report_attr_message_t *)message);
            break;
        case ESP_ZB_CORE_CMD_READ_ATTR_RESP_CB_ID:
            ret = zb_cmd_read_attr_resp_handler((esp_zb_zcl_cmd_read_attr_resp_message_t *)message);
            break;
        case ESP_ZB_CORE_CMD_REPORT_CONFIG_RESP_CB_ID:
            ret = zb_configure_report_resp_handler((esp_zb_zcl_cmd_config_report_resp_message_t *)message);
            break;
        case ESP_ZB_CORE_CMD_DEFAULT_RESP_CB_ID:
            ret = zb_cmd_default_resp_handler((esp_zb_zcl_cmd_default_resp_message_t *)message);
            break;
        default:
            ESP_LOGW(TAG, "Receive unhandled ZigBee action(0x%x) callback", callback_id);
            break;
    }
    return ret;
}

static esp_err_t zb_attribute_set_handler(const esp_zb_zcl_set_attr_value_message_t *message) {
    if (!message) {
        ESP_LOGE(TAG, "Empty message");
        return ESP_FAIL;
    }
    if (message->info.status != ESP_ZB_ZCL_STATUS_SUCCESS) {
        ESP_LOGE(TAG, "Received message: error status(%d)", message->info.status);
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGV(TAG, "Received message: endpoint(%d), cluster(0x%x), attribute(0x%x), data size(%d)",
             message->info.dst_endpoint, message->info.cluster, message->attribute.id, message->attribute.data.size);

    // List through all ZigBee EPs and call the callback function, with the message
    for (std::list<ZigBeeEndpoint *>::iterator it = ZigBee.ep_objects.begin(); it != ZigBee.ep_objects.end(); ++it) {
        if (message->info.dst_endpoint == (*it)->getEndpoint()) {
            if (message->info.cluster == ESP_ZB_ZCL_CLUSTER_ID_IDENTIFY) {
                (*it)->zbIdentify(message);  // method zbIdentify implemented in the common EP class
            } else {
                (*it)->zbAttributeSet(message);  // method zbAttributeSet must be implemented in specific EP class
            }
        }
    }
    return ESP_OK;
}

static esp_err_t zb_attribute_reporting_handler(const esp_zb_zcl_report_attr_message_t *message) {
    if (!message) {
        ESP_LOGE(TAG, "Empty message");
        return ESP_FAIL;
    }
    if (message->status != ESP_ZB_ZCL_STATUS_SUCCESS) {
        ESP_LOGE(TAG, "Received message: error status(%d)", message->status);
        return ESP_ERR_INVALID_ARG;
    }
    ESP_LOGV(TAG, "Received report from address(0x%x) src endpoint(%d) to dst endpoint(%d) cluster(0x%x)",
             message->src_address.u.short_addr, message->src_endpoint, message->dst_endpoint, message->cluster);
    // List through all ZigBee EPs and call the callback function, with the message
    for (std::list<ZigBeeEndpoint *>::iterator it = ZigBee.ep_objects.begin(); it != ZigBee.ep_objects.end(); ++it) {
        if (message->dst_endpoint == (*it)->getEndpoint()) {
            (*it)->zbAttributeRead(
                message->cluster,
                &message->attribute);  // method zbAttributeRead must be implemented in specific EP class
        }
    }
    return ESP_OK;
}

static esp_err_t zb_cmd_read_attr_resp_handler(const esp_zb_zcl_cmd_read_attr_resp_message_t *message) {
    if (!message) {
        ESP_LOGE(TAG, "Empty message");
        return ESP_FAIL;
    }
    if (message->info.status != ESP_ZB_ZCL_STATUS_SUCCESS) {
        ESP_LOGE(TAG, "Received message: error status(%d)", message->info.status);
        return ESP_ERR_INVALID_ARG;
    }
    ESP_LOGV(TAG, "Read attribute response: from address(0x%x) src endpoint(%d) to dst endpoint(%d) cluster(0x%x)",
             message->info.src_address.u.short_addr, message->info.src_endpoint, message->info.dst_endpoint,
             message->info.cluster);

    for (std::list<ZigBeeEndpoint *>::iterator it = ZigBee.ep_objects.begin(); it != ZigBee.ep_objects.end(); ++it) {
        if (message->info.dst_endpoint == (*it)->getEndpoint()) {
            esp_zb_zcl_read_attr_resp_variable_t *variable = message->variables;
            while (variable) {
                ESP_LOGV(TAG,
                         "Read attribute response: status(%d), cluster(0x%x), attribute(0x%x), type(0x%x), value(%d)",
                         variable->status, message->info.cluster, variable->attribute.id, variable->attribute.data.type,
                         variable->attribute.data.value ? *(uint8_t *)variable->attribute.data.value : 0);
                if (variable->status == ESP_ZB_ZCL_STATUS_SUCCESS) {
                    if (message->info.cluster == ESP_ZB_ZCL_CLUSTER_ID_BASIC) {
                        (*it)->zbReadBasicCluster(
                            &variable->attribute);  // method zbReadBasicCluster implemented in the common EP class
                    } else {
                        (*it)->zbAttributeRead(
                            message->info.cluster,
                            &variable->attribute);  // method zbAttributeRead must be implemented in specific EP class
                    }
                }
                variable = variable->next;
            }
        }
    }
    return ESP_OK;
}

static esp_err_t zb_configure_report_resp_handler(const esp_zb_zcl_cmd_config_report_resp_message_t *message) {
    if (!message) {
        ESP_LOGE(TAG, "Empty message");
        return ESP_FAIL;
    }
    if (message->info.status != ESP_ZB_ZCL_STATUS_SUCCESS) {
        ESP_LOGE(TAG, "Received message: error status(%d)", message->info.status);
        return ESP_ERR_INVALID_ARG;
    }
    esp_zb_zcl_config_report_resp_variable_t *variable = message->variables;
    while (variable) {
        ESP_LOGV(TAG, "Configure report response: status(%d), cluster(0x%x), direction(0x%x), attribute(0x%x)",
                 variable->status, message->info.cluster, variable->direction, variable->attribute_id);
        variable = variable->next;
    }
    return ESP_OK;
}

static esp_err_t zb_cmd_default_resp_handler(const esp_zb_zcl_cmd_default_resp_message_t *message) {
    if (!message) {
        ESP_LOGE(TAG, "Empty message");
        return ESP_FAIL;
    }
    if (message->info.status != ESP_ZB_ZCL_STATUS_SUCCESS) {
        ESP_LOGE(TAG, "Received message: error status(%d)", message->info.status);
        return ESP_ERR_INVALID_ARG;
    }
    ESP_LOGV(TAG,
             "Received default response: from address(0x%x), src_endpoint(%d) to dst_endpoint(%d), cluster(0x%x) with "
             "status 0x%x",
             message->info.src_address.u.short_addr, message->info.src_endpoint, message->info.dst_endpoint,
             message->info.cluster, message->status_code);
    return ESP_OK;
}

#define ZB_INIT_TIMEOUT 30000  // 30 seconds

extern "C" void zb_set_ed_node_descriptor(bool power_src, bool rx_on_when_idle, bool alloc_addr);

static bool edBatteryPowered = false;

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
    if (!lock) {
        lock = xSemaphoreCreateBinary();
        if (lock == NULL) {
            ESP_LOGE(TAG, "Semaphore creation failed");
        }
    }
}

// forward declaration
static esp_err_t zb_action_handler(esp_zb_core_action_callback_id_t callback_id, const void *message);

bool ZigBeeCore::begin(esp_zb_cfg_t *role_cfg, bool erase_nvs) {
    if (!zigbeeInit(role_cfg, erase_nvs)) {
        ESP_LOGE(TAG, "ZigBeeCore begin failed");
        return false;
    }
    _role = (zigbee_role_t)role_cfg->esp_zb_role;
    if (xSemaphoreTake(lock, ZB_INIT_TIMEOUT) != pdTRUE) {
        ESP_LOGE(TAG, "ZigBeeCore begin timeout");
    }
    return started();
}

bool ZigBeeCore::begin(zigbee_role_t role, bool erase_nvs) {
    bool status = true;
    switch (role) {
        case ZIGBEE_COORDINATOR: {
            _role = ZIGBEE_COORDINATOR;
            esp_zb_cfg_t zb_nwk_cfg = ZIGBEE_DEFAULT_COORDINATOR_CONFIG();
            status = zigbeeInit(&zb_nwk_cfg, erase_nvs);
            break;
        }
        case ZIGBEE_ROUTER: {
            _role = ZIGBEE_ROUTER;
            esp_zb_cfg_t zb_nwk_cfg = ZIGBEE_DEFAULT_ROUTER_CONFIG();
            status = zigbeeInit(&zb_nwk_cfg, erase_nvs);
            break;
        }
        case ZIGBEE_END_DEVICE: {
            _role = ZIGBEE_END_DEVICE;
            esp_zb_cfg_t zb_nwk_cfg = ZIGBEE_DEFAULT_ED_CONFIG();
            status = zigbeeInit(&zb_nwk_cfg, erase_nvs);
            break;
        }
        default:
            ESP_LOGE(TAG, "Invalid ZigBee Role");
            return false;
    }
    if (!status || xSemaphoreTake(lock, ZB_INIT_TIMEOUT) != pdTRUE) {
        ESP_LOGE(TAG, "ZigBeeCore begin failed or timeout");
    }
    return started();
}

void ZigBeeCore::addEndpoint(ZigBeeEndpoint *ep) {
    ep_objects.push_back(ep);

    ESP_LOGD(TAG, "Endpoint: %d, Device ID: 0x%04x", ep->_endpoint, ep->_device_id);
    // Register clusters and ep_list to the ZigBeeCore class's ep_list
    if (ep->_ep_config.endpoint == 0 || ep->_cluster_list == nullptr) {
        ESP_LOGE(TAG, "Endpoint config or Cluster list is not initialized, EP not added to ZigBeeCore's EP list");
        return;
    }

    esp_zb_ep_list_add_ep(_zb_ep_list, ep->_cluster_list, ep->_ep_config);
}

static void esp_zb_task(void *pvParameters) {
    esp_zb_bdb_set_scan_duration(ZigBee.getScanDuration());

    /* initialize ZigBee stack */
    ESP_ERROR_CHECK(esp_zb_start(false));

    // NOTE: This is a workaround to make battery powered devices to be discovered as battery powered
    if (((zigbee_role_t)ZigBee.getRole() == ZIGBEE_END_DEVICE) && edBatteryPowered) {
        zb_set_ed_node_descriptor(0, ZigBee.getRxOnWhenIdle(), 1);
    }

    esp_zb_stack_main_loop();
}

// ZigBee core init function
bool ZigBeeCore::zigbeeInit(esp_zb_cfg_t *zb_cfg, bool erase_nvs) {
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
    if (ep_objects.empty()) {
        ESP_LOGW(TAG, "No ZigBee EPs to register");
    } else {
        ESP_LOGD(TAG, "Register all ZigBee EPs in list");
        err = esp_zb_device_register(_zb_ep_list);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to register ZigBee EPs");
            return false;
        }

        // print the list of ZigBee EPs from ep_objects
        ESP_LOGI(TAG, "List of registered ZigBee EPs:");
        for (std::list<ZigBeeEndpoint *>::iterator it = ep_objects.begin(); it != ep_objects.end(); ++it) {
            ESP_LOGI(TAG, "Device type: %s, Endpoint: %d, Device ID: 0x%04x", getDeviceTypeString((*it)->_device_id),
                     (*it)->_endpoint, (*it)->_device_id);
            if ((*it)->_power_source == ZB_POWER_SOURCE_BATTERY) {
                edBatteryPowered = true;
            }
        }
    }
    // Register ZigBee action handler
    esp_zb_core_action_handler_register(zb_action_handler);
    err = esp_zb_set_primary_network_channel_set(_primary_channel_mask);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set primary network channel mask");
        return false;
    }

    // Erase NVRAM before creating connection to new Coordinator
    if (erase_nvs) {
        esp_zb_nvram_erase_at_start(true);
    }

    // Create ZigBee task and start ZigBee stack
    xTaskCreate(esp_zb_task, "ZigBee_main", 4096, NULL, 5, NULL);

    return true;
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
    if (started()) {
        ESP_LOGV(TAG, "Opening network for joining for %d seconds", time);
        esp_zb_bdb_open_network(time);
    }
}

static void bdb_start_top_level_commissioning_cb(uint8_t mode_mask) {
    ESP_ERROR_CHECK(esp_zb_bdb_start_top_level_commissioning(mode_mask));
}

void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_struct) {
    // common variables
    uint32_t *p_sg_p = signal_struct->p_app_signal;
    esp_err_t err_status = signal_struct->esp_err_status;
    esp_zb_app_signal_type_t sig_type = (esp_zb_app_signal_type_t)*p_sg_p;
    // coordinator variables
    esp_zb_zdo_signal_device_annce_params_t *dev_annce_params = NULL;

    // main switch
    switch (sig_type) {
        case ESP_ZB_ZDO_SIGNAL_SKIP_STARTUP:  // Common
            ESP_LOGI(TAG, "ZigBee stack initialized");
            esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_INITIALIZATION);
            break;
        case ESP_ZB_BDB_SIGNAL_DEVICE_FIRST_START:  // Common
        case ESP_ZB_BDB_SIGNAL_DEVICE_REBOOT:       // Common
            if (err_status == ESP_OK) {
                ESP_LOGI(TAG, "Device started up in %s factory-reset mode", esp_zb_bdb_is_factory_new() ? "" : "non");
                if (esp_zb_bdb_is_factory_new()) {
                    // Role specific code
                    if ((zigbee_role_t)ZigBee.getRole() == ZIGBEE_COORDINATOR) {
                        ESP_LOGI(TAG, "Start network formation");
                        esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_FORMATION);
                    } else {
                        ESP_LOGI(TAG, "Start network steering");
                        esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);
                        ZigBee._started = true;
                        xSemaphoreGive(ZigBee.lock);
                    }
                } else {
                    ESP_LOGI(TAG, "Device rebooted");
                    ZigBee._started = true;
                    xSemaphoreGive(ZigBee.lock);
                    if ((zigbee_role_t)ZigBee.getRole() == ZIGBEE_COORDINATOR && ZigBee._open_network > 0) {
                        ESP_LOGI(TAG, "Opening network for joining for %d seconds", ZigBee._open_network);
                        esp_zb_bdb_open_network(ZigBee._open_network);
                    } else {
                        ZigBee._connected = true;
                    }
                    ZigBee.searchBindings();
                }
            } else {
                /* commissioning failed */
                ESP_LOGW(TAG, "Commissioning failed (%s), trying again...", esp_err_to_name(err_status));
                esp_zb_scheduler_alarm((esp_zb_callback_t)bdb_start_top_level_commissioning_cb,
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
                    esp_zb_scheduler_alarm((esp_zb_callback_t)bdb_start_top_level_commissioning_cb,
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
                xSemaphoreGive(ZigBee.lock);
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
                    ZigBee._connected = true;
                } else {
                    ESP_LOGI(TAG, "Network steering was not successful (status: %s)", esp_err_to_name(err_status));
                    esp_zb_scheduler_alarm((esp_zb_callback_t)bdb_start_top_level_commissioning_cb,
                                           ESP_ZB_BDB_MODE_NETWORK_STEERING, 1000);
                }
            }
            break;
        case ESP_ZB_ZDO_SIGNAL_DEVICE_ANNCE:  // Coordinator
            if ((zigbee_role_t)ZigBee.getRole() == ZIGBEE_COORDINATOR) {
                dev_annce_params = (esp_zb_zdo_signal_device_annce_params_t *)esp_zb_app_signal_get_params(p_sg_p);
                ESP_LOGI(TAG, "New device commissioned or rejoined (short: 0x%04hx)",
                         dev_annce_params->device_short_addr);
                esp_zb_zdo_match_desc_req_param_t cmd_req;
                cmd_req.dst_nwk_addr = dev_annce_params->device_short_addr;
                cmd_req.addr_of_interest = dev_annce_params->device_short_addr;
                ESP_LOGV(TAG, "Device capabilities: 0x%02x", dev_annce_params->capability);
                /*
                    capability:
                    Bit 0 – Alternate PAN Coordinator
                    Bit 1 – Device type: 1- ZigBee Router; 0 – End Device
                    Bit 2 – Power Source: 1 Main powered
                    Bit 3 – Receiver on when Idle
                    Bit 4 – Reserved
                    Bit 5 – Reserved
                    Bit 6 – Security capability
                    Bit 7 – Reserved
                */
                // for each endpoint in the list call the findEndpoint function if not bounded or allowed to bind
                // multiple devices
                for (std::list<ZigBeeEndpoint *>::iterator it = ZigBee.ep_objects.begin();
                     it != ZigBee.ep_objects.end(); ++it) {
                    if (!(*it)->bound() || (*it)->epAllowMultipleBinding()) {
                        // Check if the device is already bound
                        bool found = false;
                        // Get the list of devices bound to the EP
                        std::list<zb_device_params_t *> bound_devices = (*it)->getBoundDevices();
                        for (std::list<zb_device_params_t *>::iterator device = bound_devices.begin();
                             device != bound_devices.end(); ++device) {
                            if (((*device)->short_addr == dev_annce_params->device_short_addr) ||
                                (memcmp((*device)->ieee_addr, dev_annce_params->ieee_addr, 8) == 0)) {
                                found = true;
                                ESP_LOGD(TAG, "Device already bound to endpoint %d", (*it)->getEndpoint());
                                break;
                            }
                        }
                        if (!found) {
                            (*it)->findEndpoint(&cmd_req);
                        }
                    }
                }
            }
            break;
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
            if ((zigbee_role_t)ZigBee.getRole() != ZIGBEE_COORDINATOR) {
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

// Recall bounded devices from the binding table after reboot
void ZigBeeCore::bindingTableCb(const esp_zb_zdo_binding_table_info_t *table_info, void *user_ctx) {
    bool done = true;
    esp_zb_zdo_mgmt_bind_param_t *req = (esp_zb_zdo_mgmt_bind_param_t *)user_ctx;
    esp_zb_zdp_status_t zdo_status = (esp_zb_zdp_status_t)table_info->status;
    ESP_LOGD(TAG, "Binding table callback for address 0x%04x with status %d", req->dst_addr, zdo_status);
    if (zdo_status == ESP_ZB_ZDP_STATUS_SUCCESS) {
        // Print binding table log simple
        ESP_LOGD(TAG, "Binding table info: total %d, index %d, count %d", table_info->total, table_info->index,
                 table_info->count);

        if (table_info->total == 0) {
            ESP_LOGD(TAG, "No binding table entries found");
            free(req);
            return;
        }

        esp_zb_zdo_binding_table_record_t *record = table_info->record;
        for (int i = 0; i < table_info->count; i++) {
            ESP_LOGD(TAG, "Binding table record: src_endp %d, dst_endp %d, cluster_id 0x%04x, dst_addr_mode %d",
                     record->src_endp, record->dst_endp, record->cluster_id, record->dst_addr_mode);

            zb_device_params_t *device = (zb_device_params_t *)calloc(1, sizeof(zb_device_params_t));
            device->endpoint = record->dst_endp;
            if (record->dst_addr_mode == ESP_ZB_APS_ADDR_MODE_16_ENDP_PRESENT ||
                record->dst_addr_mode == ESP_ZB_APS_ADDR_MODE_16_GROUP_ENDP_NOT_PRESENT) {
                device->short_addr = record->dst_address.addr_short;
            } else {  // ESP_ZB_APS_ADDR_MODE_64_ENDP_PRESENT
                memcpy(device->ieee_addr, record->dst_address.addr_long, sizeof(esp_zb_ieee_addr_t));
            }

            // Add to list of bound devices of proper endpoint
            for (std::list<ZigBeeEndpoint *>::iterator it = ZigBee.ep_objects.begin(); it != ZigBee.ep_objects.end();
                 ++it) {
                if ((*it)->getEndpoint() == record->src_endp) {
                    (*it)->addBoundDevice(device);
                    ESP_LOGD(TAG,
                             "Device bound to EP %d -> device endpoint: %d, short addr: 0x%04x, ieee addr: "
                             "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
                             record->src_endp, device->endpoint, device->short_addr, device->ieee_addr[7],
                             device->ieee_addr[6], device->ieee_addr[5], device->ieee_addr[4], device->ieee_addr[3],
                             device->ieee_addr[2], device->ieee_addr[1], device->ieee_addr[0]);
                }
            }
            record = record->next;
        }

        // Continue reading the binding table
        if (table_info->index + table_info->count < table_info->total) {
            /* There are unreported binding table entries, request for them. */
            req->start_index = table_info->index + table_info->count;
            esp_zb_zdo_binding_table_req(req, bindingTableCb, req);
            done = false;
        }
    }

    if (done) {
        // Print bound devices
        ESP_LOGD(TAG, "Filling bounded devices finished");
        free(req);
    }
}

void ZigBeeCore::searchBindings() {
    esp_zb_zdo_mgmt_bind_param_t *mb_req = (esp_zb_zdo_mgmt_bind_param_t *)malloc(sizeof(esp_zb_zdo_mgmt_bind_param_t));
    mb_req->dst_addr = esp_zb_get_short_address();
    mb_req->start_index = 0;
    ESP_LOGD(TAG, "Requesting binding table for address 0x%04x", mb_req->dst_addr);
    esp_zb_zdo_binding_table_req(mb_req, bindingTableCb, (void *)mb_req);
}

// Function to convert enum value to string
const char *ZigBeeCore::getDeviceTypeString(esp_zb_ha_standard_devices_t deviceId) {
    switch (deviceId) {
        case ESP_ZB_HA_ON_OFF_SWITCH_DEVICE_ID:
            return "General On/Off switch";
        case ESP_ZB_HA_LEVEL_CONTROL_SWITCH_DEVICE_ID:
            return "Level Control Switch";
        case ESP_ZB_HA_ON_OFF_OUTPUT_DEVICE_ID:
            return "General On/Off output";
        case ESP_ZB_HA_LEVEL_CONTROLLABLE_OUTPUT_DEVICE_ID:
            return "Level Controllable Output";
        case ESP_ZB_HA_SCENE_SELECTOR_DEVICE_ID:
            return "Scene Selector";
        case ESP_ZB_HA_CONFIGURATION_TOOL_DEVICE_ID:
            return "Configuration Tool";
        case ESP_ZB_HA_REMOTE_CONTROL_DEVICE_ID:
            return "Remote Control";
        case ESP_ZB_HA_COMBINED_INTERFACE_DEVICE_ID:
            return "Combined Interface";
        case ESP_ZB_HA_RANGE_EXTENDER_DEVICE_ID:
            return "Range Extender";
        case ESP_ZB_HA_MAINS_POWER_OUTLET_DEVICE_ID:
            return "Mains Power Outlet";
        case ESP_ZB_HA_DOOR_LOCK_DEVICE_ID:
            return "Door lock client";
        case ESP_ZB_HA_DOOR_LOCK_CONTROLLER_DEVICE_ID:
            return "Door lock controller";
        case ESP_ZB_HA_SIMPLE_SENSOR_DEVICE_ID:
            return "Simple Sensor device";
        case ESP_ZB_HA_CONSUMPTION_AWARENESS_DEVICE_ID:
            return "Consumption Awareness Device";
        case ESP_ZB_HA_HOME_GATEWAY_DEVICE_ID:
            return "Home Gateway";
        case ESP_ZB_HA_SMART_PLUG_DEVICE_ID:
            return "Smart plug";
        case ESP_ZB_HA_WHITE_GOODS_DEVICE_ID:
            return "White Goods";
        case ESP_ZB_HA_METER_INTERFACE_DEVICE_ID:
            return "Meter Interface";
        case ESP_ZB_HA_ON_OFF_LIGHT_DEVICE_ID:
            return "On/Off Light Device";
        case ESP_ZB_HA_DIMMABLE_LIGHT_DEVICE_ID:
            return "Dimmable Light Device";
        case ESP_ZB_HA_COLOR_DIMMABLE_LIGHT_DEVICE_ID:
            return "Color Dimmable Light Device";
        case ESP_ZB_HA_DIMMER_SWITCH_DEVICE_ID:
            return "Dimmer Switch Device";
        case ESP_ZB_HA_COLOR_DIMMER_SWITCH_DEVICE_ID:
            return "Color Dimmer Switch Device";
        case ESP_ZB_HA_LIGHT_SENSOR_DEVICE_ID:
            return "Light Sensor";
        case ESP_ZB_HA_SHADE_DEVICE_ID:
            return "Shade";
        case ESP_ZB_HA_SHADE_CONTROLLER_DEVICE_ID:
            return "Shade controller";
        case ESP_ZB_HA_WINDOW_COVERING_DEVICE_ID:
            return "Window Covering client";
        case ESP_ZB_HA_WINDOW_COVERING_CONTROLLER_DEVICE_ID:
            return "Window Covering controller";
        case ESP_ZB_HA_HEATING_COOLING_UNIT_DEVICE_ID:
            return "Heating/Cooling Unit device";
        case ESP_ZB_HA_THERMOSTAT_DEVICE_ID:
            return "Thermostat Device";
        case ESP_ZB_HA_TEMPERATURE_SENSOR_DEVICE_ID:
            return "Temperature Sensor";
        case ESP_ZB_HA_IAS_CONTROL_INDICATING_EQUIPMENT_ID:
            return "IAS Control and Indicating Equipment";
        case ESP_ZB_HA_IAS_ANCILLARY_CONTROL_EQUIPMENT_ID:
            return "IAS Ancillary Control Equipment";
        case ESP_ZB_HA_IAS_ZONE_ID:
            return "IAS Zone";
        case ESP_ZB_HA_IAS_WARNING_DEVICE_ID:
            return "IAS Warning Device";
        case ESP_ZB_HA_TEST_DEVICE_ID:
            return "Custom HA device for test";
        case ESP_ZB_HA_CUSTOM_TUNNEL_DEVICE_ID:
            return "Custom Tunnel device";
        case ESP_ZB_HA_CUSTOM_ATTR_DEVICE_ID:
            return "Custom Attributes Device";
        default:
            return "Unknown device type";
    }
}

ZigBeeCore ZigBee;