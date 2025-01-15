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

void ZigBeeEndpoint::addCluster(Cluster *cluster) {
    _clusters.push_back(cluster);

    // Fake the cluster ID to get around library checks.

    for (auto attribute = cluster->_cluster; attribute; attribute = attribute->next) {
        attribute->cluster_id = 0xff00;
    }

    ESP_ERROR_CHECK(
        esp_zb_cluster_list_add_custom_cluster(_cluster_list, cluster->_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));

    for (auto attribute = cluster->_cluster; attribute; attribute = attribute->next) {
        attribute->cluster_id = cluster->getClusterId();
    }
    for (auto zbcluster = _cluster_list; zbcluster; zbcluster = zbcluster->next) {
        if (zbcluster->cluster.cluster_id == 0xff00) {
            zbcluster->cluster.cluster_id = cluster->getClusterId();
        }
    }
}

Cluster *ZigBeeEndpoint::getClusterById(uint16_t clusterId) {
    for (const auto cluster : _clusters) {
        if (cluster->getClusterId() == clusterId) {
            return cluster;
        }
    }

    return nullptr;
}

esp_err_t ZigBeeEndpoint::zbAttributeSet(const esp_zb_zcl_set_attr_value_message_t *message) {
    const auto cluster = getClusterById(message->info.cluster);
    if (cluster) {
        return cluster->zbAttributeSet(message);
    }

    return ESP_OK;
}

esp_err_t ZigBeeEndpoint::zbAttributeRead(uint16_t cluster_id, const esp_zb_zcl_attribute_t *attribute) {
    ESP_LOGW(TAG, "zbAttributeRead has not been implemented");

    return ESP_OK;
}
