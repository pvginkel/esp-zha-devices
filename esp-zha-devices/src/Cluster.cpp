#include "support.h"

#include "Cluster.h"

#include "ZigBeeEndpoint.h"

esp_err_t Cluster::sendMessage(uint8_t endpointId, uint16_t commandId, const uint8_t* buf, uint16_t buf_len) {
    esp_zb_zcl_custom_cluster_cmd_req_t req{};
    if (endpointId != Attribute::REPORT_BROADCAST) {
        esp_zb_get_extended_pan_id(req.zcl_basic_cmd.dst_addr_u.addr_long);
        req.zcl_basic_cmd.dst_endpoint = 1;
    } else {
        req.zcl_basic_cmd.dst_endpoint = endpointId;
    }
    req.zcl_basic_cmd.src_endpoint = _endpoint->getEndpoint();
    req.address_mode = ESP_ZB_APS_ADDR_MODE_64_ENDP_PRESENT;
    req.cluster_id = getClusterId();
    req.profile_id = ESP_ZB_AF_HA_PROFILE_ID;
    req.direction = ESP_ZB_ZCL_CMD_DIRECTION_TO_SRV;
    req.custom_cmd_id = commandId;
    // Type is just any type that will copy the buffer and buffer length verbatim.
    // See e.g. https://github.com/espressif/esp-zigbee-sdk/issues/467#issuecomment-2481330169.
    req.data.type = ESP_ZB_ZCL_ATTR_TYPE_SET;
    req.data.size = buf_len;
    req.data.value = (void*)buf;

    esp_zb_lock_acquire(portMAX_DELAY);
    esp_zb_zcl_custom_cluster_cmd_req(&req);
    esp_zb_lock_release();

    return ESP_OK;
}
