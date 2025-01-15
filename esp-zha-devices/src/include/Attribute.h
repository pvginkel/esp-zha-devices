#pragma once

#include <string>

#include "Callback.h"
#include "Cluster.h"
#include "DateTime.h"
#include "ZigBeeStream.h"

class Attribute {
    friend Cluster;

    Cluster* _cluster{};
    esp_zb_zcl_attr_type_t _dataType;
    uint16_t _attributeId;
    Callback<void> _valueChanged;

public:
    static constexpr uint8_t REPORT_BROADCAST = 0xff;

    Attribute(uint16_t attributeId, esp_zb_zcl_attr_type_t dataType) : _dataType(dataType), _attributeId(attributeId) {}
    Attribute(const Attribute&) = delete;
    virtual ~Attribute() = default;

    uint16_t getAttributeId() { return _attributeId; }
    esp_zb_zcl_attr_type_t getDataType() { return _dataType; }

    void onValueChanged(std::function<void(void)> func) { _valueChanged.add(func); }

    Attribute& operator=(const Attribute&) = delete;

protected:
    esp_zb_attribute_list_t* getZbCluster() { return _cluster->_cluster; }

    esp_zb_attribute_list_t* getZbAttribute() {
        for (auto attr = _cluster->_cluster; attr; attr = attr->next) {
            if (attr->cluster_id == _cluster->getClusterId() && attr->attribute.id == _attributeId) {
                return attr;
            }
        }

        return nullptr;
    }
};

class AttributeString : public Attribute {
public:
    AttributeString(uint16_t attributeId) : Attribute(attributeId, ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING) {}

    std::string getValue() {
        const auto attr = getZbAttribute();
        ZigBeeStream stream((uint8_t*)attr->attribute.data_p);

        return stream.readString();
    }

    void setValue(const std::string& value) { setValue(value.c_str()); }

    void setValue(const char* value) {
        const auto len = strlen(value);
        const auto buffer = (uint8_t*)malloc(len + 1);
        buffer[0] = (uint8_t)len;
        memcpy(buffer + 1, value, len);

        auto err = esp_zb_cluster_update_attr(getZbCluster(), getAttributeId(), buffer);
        if (err == ESP_ERR_NOT_FOUND) {
            err = esp_zb_custom_cluster_add_custom_attr(
                getZbCluster(), getAttributeId(), getDataType(),
                ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE | ESP_ZB_ZCL_ATTR_ACCESS_REPORTING, buffer);
        }
        ESP_ERROR_CHECK(err);

        free(buffer);
    }
};

// // GENERATION START
class AttributeUInt8 : public Attribute {
public:
    AttributeUInt8(uint16_t attributeId, esp_zb_zcl_attr_type_t dataType) : Attribute(attributeId, dataType) {}

    uint8_t getValue() {
        const auto attr = getZbAttribute();
        ZigBeeStream stream((uint8_t*)attr->attribute.data_p);

        return stream.readUInt8();
    }

    void setValue(uint8_t value) {
        auto err = esp_zb_cluster_update_attr(getZbCluster(), getAttributeId(), (void*)&value);
        if (err == ESP_ERR_NOT_FOUND) {
            err = esp_zb_custom_cluster_add_custom_attr(
                getZbCluster(), getAttributeId(), getDataType(),
                ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE | ESP_ZB_ZCL_ATTR_ACCESS_REPORTING, (void*)&value);
        }
        ESP_ERROR_CHECK(err);
    }
};

class AttributeUInt16 : public Attribute {
public:
    AttributeUInt16(uint16_t attributeId, esp_zb_zcl_attr_type_t dataType) : Attribute(attributeId, dataType) {}

    uint8_t getValue() {
        const auto attr = getZbAttribute();
        ZigBeeStream stream((uint8_t*)attr->attribute.data_p);

        return stream.readUInt16Le();
    }

    void setValue(uint16_t value) {
        auto err = esp_zb_cluster_update_attr(getZbCluster(), getAttributeId(), (void*)&value);
        if (err == ESP_ERR_NOT_FOUND) {
            err = esp_zb_custom_cluster_add_custom_attr(
                getZbCluster(), getAttributeId(), getDataType(),
                ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE | ESP_ZB_ZCL_ATTR_ACCESS_REPORTING, (void*)&value);
        }
        ESP_ERROR_CHECK(err);
    }
};

// GENERATION END
