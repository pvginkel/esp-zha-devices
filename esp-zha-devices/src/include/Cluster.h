#pragma once

#include "Attribute.h"
#include "ZigBeeStream.h"

class ZigBeeEndpoint;

enum class ClusterType : uint8_t { Input, Output };

class Device;

class Cluster {
    esp_zb_attribute_list_t* _attributes;
    ZigBeeEndpoint* _endpoint;
    uint16_t _clusterId;
    ClusterType _type;

public:
    Cluster(uint16_t clusterId, ClusterType type);
    Cluster(const Cluster&) = delete;
    virtual ~Cluster();

    ClusterType getType() { return _type; }
    uint16_t getClusterId() { return _clusterId; }

    void addAttribute(Attribute* attribute);
    Attribute* getAttributeById(uint16_t attributeId);

    virtual esp_err_t zbCommand(const zb_zcl_parsed_hdr_t* cmd_info, ZigBeeStream& request, ZigBeeStream& response) {
        return ESP_ERR_NOT_SUPPORTED;
    }

    virtual void zbAttributeSet(const esp_zb_zcl_set_attr_value_message_t* message) {};

    Cluster& operator=(const Cluster&) = delete;

protected:
    esp_err_t sendMessage(uint8_t endpointId, uint16_t commandId) {
        return sendMessage(endpointId, commandId, nullptr, 0);
    }

    esp_err_t sendMessage(uint8_t endpointId, uint16_t commandId, ZigBeeStream& stream) {
        return sendMessage(endpointId, commandId, stream.getData(), stream.getPosition());
    }

private:
    esp_err_t sendMessage(uint8_t endpointId, uint16_t commandId, const uint8_t* buf, uint16_t buf_len);
};