// Adapted from https://github.com/espressif/arduino-esp32/blob/master/LICENSE.md.
/* Common Class for Zigbee End point */

#pragma once

#include "Cluster.h"
#include "ZigBeeCore.h"
#include "ZigBeeStream.h"

// See https://github.com/espressif/arduino-esp32/issues/9745#issuecomment-2165478493.
#ifdef __cplusplus
extern "C" {
#endif
#include "zboss_api.h"
#ifdef __cplusplus
};
#endif

class ZigBeeEndpoint {
    friend class ZigBeeCore;

    esp_zb_endpoint_config_t _ep_config;
    esp_zb_cluster_list_t *_cluster_list{};
    std::vector<Cluster *> _clusters;

public:
    ZigBeeEndpoint(uint8_t endpoint, uint8_t version);
    virtual ~ZigBeeEndpoint() {}

    uint8_t getEndpointId() { return _ep_config.endpoint; }

    void addCluster(Cluster *cluster);

    virtual void zbAttributeSet(const esp_zb_zcl_set_attr_value_message_t *message) {};
    virtual void zbAttributeRead(uint16_t cluster_id, const esp_zb_zcl_attribute_t *attribute) {};
    virtual void zbReadBasicCluster(const esp_zb_zcl_attribute_t *attribute);
    virtual esp_err_t zbCommand(const zb_zcl_parsed_hdr_t *cmd_info, ZigBeeStream &request, ZigBeeStream &response) {
        return ESP_ERR_NOT_SUPPORTED;
    }
};
