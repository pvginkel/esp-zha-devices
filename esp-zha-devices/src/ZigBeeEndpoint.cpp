// Adapted from https://github.com/espressif/arduino-esp32/blob/master/LICENSE.md.
/* Common Class for Zigbee End Point */

#include "support.h"

#include "ZigBeeEndpoint.h"

#include "esp_zigbee_cluster.h"

LOG_TAG(ZigBeeEndpoint);

ZigBeeEndpoint::ZigBeeEndpoint(uint8_t endpoint, uint8_t version) {
    _ep_config = {
        .endpoint = endpoint,
        .app_profile_id = ESP_ZB_AF_HA_PROFILE_ID,
        .app_device_id = ESP_ZB_HA_CUSTOM_ATTR_DEVICE_ID,
        .app_device_version = version,
    };

    ESP_LOGV(TAG, "Endpoint: %d", endpoint);

    _cluster_list = esp_zb_zcl_cluster_list_create();
}

void ZigBeeEndpoint::addCluster(Cluster* cluster) {
    _clusters.push_back(cluster);

    esp_zb_cluster_list_add_custom_cluster(_cluster_list, cluster->_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
}
