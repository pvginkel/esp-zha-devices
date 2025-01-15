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
    virtual esp_err_t createValue() = 0;

    Cluster* getCluster() { return _cluster; }
    esp_zb_attribute_list_t* getZbCluster() { return _cluster->_cluster; }
    esp_zb_attribute_list_t* getZbAttribute();
};

class AttributeString : public Attribute {
public:
    AttributeString(uint16_t attributeId) : Attribute(attributeId, ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING) {}

    std::string getValue();

    void setValue(const std::string& value) { setValue(value.c_str()); }

    void setValue(const char* value);

protected:
    esp_err_t createValue() override;
};

// // GENERATION START
class AttributeUInt8 : public Attribute {
public:
    AttributeUInt8(uint16_t attributeId, esp_zb_zcl_attr_type_t dataType) : Attribute(attributeId, dataType) {}

    uint8_t getValue();

    void setValue(uint8_t value);

protected:
    esp_err_t createValue() override;
};

class AttributeUInt16 : public Attribute {
public:
    AttributeUInt16(uint16_t attributeId, esp_zb_zcl_attr_type_t dataType) : Attribute(attributeId, dataType) {}

    uint8_t getValue();

    void setValue(uint16_t value);

protected:
    esp_err_t createValue() override;
};

// GENERATION END
