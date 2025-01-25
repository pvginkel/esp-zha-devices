#include "support.h"

#include "ZigBeeEndpoint.h"

ZigBeeAttributeBool* ZigBeeEndpoint::create_attribute_bool(uint16_t cluster_id, uint16_t attribute_id) {
    const auto attribute = new ZigBeeAttributeBool(get_endpoint_id(), cluster_id, attribute_id);
    _attributes.push_back(attribute);
    return attribute;
}

ZigBeeAttributeU8* ZigBeeEndpoint::create_attribute_u8(uint16_t cluster_id, uint16_t attribute_id) {
    const auto attribute = new ZigBeeAttributeU8(get_endpoint_id(), cluster_id, attribute_id);
    _attributes.push_back(attribute);
    return attribute;
}

ZigBeeAttributeU16* ZigBeeEndpoint::create_attribute_u16(uint16_t cluster_id, uint16_t attribute_id) {
    const auto attribute = new ZigBeeAttributeU16(get_endpoint_id(), cluster_id, attribute_id);
    _attributes.push_back(attribute);
    return attribute;
}

esp_err_t ZigBeeEndpoint::attribute_handler(const esp_zb_zcl_set_attr_value_message_t* message) {
    for (const auto attribute : _attributes) {
        if (attribute->_cluster_id == message->info.cluster && attribute->_attribute_id == message->attribute.id) {
            attribute->raise_changed(message);
            break;
        }
    }

    return ESP_OK;
}
