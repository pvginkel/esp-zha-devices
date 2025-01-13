#pragma once

#include <string>

#include "DateTime.h"
#include "ZigBeeStream.h"

class Cluster;

class Attribute {
    esp_zb_zcl_attr_type_t _dataType;
    uint16_t _attributeId;
    bool _dirty;
    uint8_t _reportingEndpointId;

public:
    static constexpr uint8_t REPORT_BROADCAST = 0xff;

    Attribute(uint16_t attributeId, esp_zb_zcl_attr_type_t dataType)
        : _dataType(dataType), _attributeId(attributeId), _dirty(true), _reportingEndpointId(0) {}
    Attribute(const Attribute&) = delete;
    virtual ~Attribute() = default;

    uint16_t getAttributeId() { return _attributeId; }
    esp_zb_zcl_attr_type_t getDataType() { return _dataType; }

    uint8_t getReportingEndpointId() { return _reportingEndpointId; }
    void setReportingEndpointId(uint8_t reportingEndpointId) { _reportingEndpointId = reportingEndpointId; }
    void configureBroadcastReporting() { setReportingEndpointId(REPORT_BROADCAST); }
    bool isDirty() { return _dirty; }
    void markClean() { _dirty = false; }

    virtual std::string toString() = 0;

    virtual void readValue(ZigBeeStream& memory) = 0;
    virtual void writeValue(ZigBeeStream& memory) = 0;

    Attribute& operator=(const Attribute&) = delete;

protected:
    void markDirty() { _dirty = true; }
};

class AttributeString : public Attribute {
    std::string _value;

public:
    AttributeString(uint16_t attributeId) : Attribute(attributeId, ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING) {}

    void setValue(const std::string& value) {
        _value = value;
        markDirty();
    }

    void setValue(const char* value) {
        _value = value;
        markDirty();
    }

    std::string toString() override { return _value; }

    void readValue(ZigBeeStream& memory) override {
        // Unsupported.
    }

    void readStringValue(ZigBeeStream& memory, esp_zb_zcl_attr_type_t dataType) {
        uint16_t length;
        if (dataType == ESP_ZB_ZCL_ATTR_TYPE_LONG_CHAR_STRING) {
            length = memory.readUInt16Le();
        } else {
            length = memory.readUInt8();
        }

        _value.clear();
        _value.reserve(length);

        while (length--) {
            _value.push_back((char)memory.readUInt8());
        }
    }

    void writeValue(ZigBeeStream& memory) override {
        // We only support long strings to keep program size down.
        memory.writeUInt16Le(_value.length());

        auto value = _value.c_str();
        for (;;) {
            auto c = *value++;
            if (!c) {
                break;
            }
            memory.writeUInt8(c);
        }
    }
};

class AttributeOctstr : public Attribute {
    Buffer _value{};

public:
    AttributeOctstr(uint16_t attributeId) : Attribute(attributeId, ESP_ZB_ZCL_ATTR_TYPE_OCTET_STRING) {}

    const Buffer& getValue() { return _value; }

    void setValue(Buffer value) {
        _value = value;
        markDirty();
    }

    std::string toString() override { return "BUFFER"; }

    void readValue(ZigBeeStream& memory) override {
        // Unsupported.
    }

    void readOctstrValue(ZigBeeStream& memory, esp_zb_zcl_attr_type_t dataType) {
        if (dataType == ESP_ZB_ZCL_ATTR_TYPE_LONG_OCTET_STRING) {
            _value = memory.readOctstr16Le();
        } else {
            _value = memory.readOctstr();
        }
    }

    void writeValue(ZigBeeStream& memory) override {
        // We only support long octstrs to keep program size down.
        memory.writeOctstr16Le(_value);
    }
};

// GENERATION START
class AttributeUInt8 : public Attribute {
    uint8_t _value{};

public:
    AttributeUInt8(uint16_t attributeId, esp_zb_zcl_attr_type_t dataType) : Attribute(attributeId, dataType) {}

    uint8_t getValue() { return _value; }

    void setValue(uint8_t value) {
        if (_value != value) {
            _value = value;
            markDirty();
        }
    }

    std::string toString() override { return "TODO"; }

    void readValue(ZigBeeStream& memory) override { _value = memory.readUInt8(); }

    void writeValue(ZigBeeStream& memory) override { memory.writeUInt8(_value); }
};

class AttributeUInt16 : public Attribute {
    uint16_t _value{};

public:
    AttributeUInt16(uint16_t attributeId, esp_zb_zcl_attr_type_t dataType) : Attribute(attributeId, dataType) {}

    uint16_t getValue() { return _value; }

    void setValue(uint16_t value) {
        if (_value != value) {
            _value = value;
            markDirty();
        }
    }

    std::string toString() override { return "TODO"; }

    void readValue(ZigBeeStream& memory) override { _value = memory.readUInt16Le(); }

    void writeValue(ZigBeeStream& memory) override { memory.writeUInt16Le(_value); }
};

class AttributeUInt24 : public Attribute {
    uint32_t _value{};

public:
    AttributeUInt24(uint16_t attributeId, esp_zb_zcl_attr_type_t dataType) : Attribute(attributeId, dataType) {}

    uint32_t getValue() { return _value; }

    void setValue(uint32_t value) {
        if (_value != value) {
            _value = value;
            markDirty();
        }
    }

    std::string toString() override { return "TODO"; }

    void readValue(ZigBeeStream& memory) override { _value = memory.readUInt24Le(); }

    void writeValue(ZigBeeStream& memory) override { memory.writeUInt24Le(_value); }
};

class AttributeUInt32 : public Attribute {
    uint32_t _value{};

public:
    AttributeUInt32(uint16_t attributeId, esp_zb_zcl_attr_type_t dataType) : Attribute(attributeId, dataType) {}

    uint32_t getValue() { return _value; }

    void setValue(uint32_t value) {
        if (_value != value) {
            _value = value;
            markDirty();
        }
    }

    std::string toString() override { return "TODO"; }

    void readValue(ZigBeeStream& memory) override { _value = memory.readUInt32Le(); }

    void writeValue(ZigBeeStream& memory) override { memory.writeUInt32Le(_value); }
};

class AttributeUInt40 : public Attribute {
    uint64_t _value{};

public:
    AttributeUInt40(uint16_t attributeId, esp_zb_zcl_attr_type_t dataType) : Attribute(attributeId, dataType) {}

    uint64_t getValue() { return _value; }

    void setValue(uint64_t value) {
        if (_value != value) {
            _value = value;
            markDirty();
        }
    }

    std::string toString() override { return "TODO"; }

    void readValue(ZigBeeStream& memory) override { _value = memory.readUInt40Le(); }

    void writeValue(ZigBeeStream& memory) override { memory.writeUInt40Le(_value); }
};

class AttributeUInt48 : public Attribute {
    uint64_t _value{};

public:
    AttributeUInt48(uint16_t attributeId, esp_zb_zcl_attr_type_t dataType) : Attribute(attributeId, dataType) {}

    uint64_t getValue() { return _value; }

    void setValue(uint64_t value) {
        if (_value != value) {
            _value = value;
            markDirty();
        }
    }

    std::string toString() override { return "TODO"; }

    void readValue(ZigBeeStream& memory) override { _value = memory.readUInt48Le(); }

    void writeValue(ZigBeeStream& memory) override { memory.writeUInt48Le(_value); }
};

class AttributeUInt56 : public Attribute {
    uint64_t _value{};

public:
    AttributeUInt56(uint16_t attributeId, esp_zb_zcl_attr_type_t dataType) : Attribute(attributeId, dataType) {}

    uint64_t getValue() { return _value; }

    void setValue(uint64_t value) {
        if (_value != value) {
            _value = value;
            markDirty();
        }
    }

    std::string toString() override { return "TODO"; }

    void readValue(ZigBeeStream& memory) override { _value = memory.readUInt56Le(); }

    void writeValue(ZigBeeStream& memory) override { memory.writeUInt56Le(_value); }
};

class AttributeUInt64 : public Attribute {
    uint64_t _value{};

public:
    AttributeUInt64(uint16_t attributeId, esp_zb_zcl_attr_type_t dataType) : Attribute(attributeId, dataType) {}

    uint64_t getValue() { return _value; }

    void setValue(uint64_t value) {
        if (_value != value) {
            _value = value;
            markDirty();
        }
    }

    std::string toString() override { return "TODO"; }

    void readValue(ZigBeeStream& memory) override { _value = memory.readUInt64Le(); }

    void writeValue(ZigBeeStream& memory) override { memory.writeUInt64Le(_value); }
};

class AttributeInt8 : public Attribute {
    int8_t _value{};

public:
    AttributeInt8(uint16_t attributeId, esp_zb_zcl_attr_type_t dataType) : Attribute(attributeId, dataType) {}

    int8_t getValue() { return _value; }

    void setValue(int8_t value) {
        if (_value != value) {
            _value = value;
            markDirty();
        }
    }

    std::string toString() override { return "TODO"; }

    void readValue(ZigBeeStream& memory) override { _value = memory.readInt8(); }

    void writeValue(ZigBeeStream& memory) override { memory.writeInt8(_value); }
};

class AttributeInt16 : public Attribute {
    int16_t _value{};

public:
    AttributeInt16(uint16_t attributeId, esp_zb_zcl_attr_type_t dataType) : Attribute(attributeId, dataType) {}

    int16_t getValue() { return _value; }

    void setValue(int16_t value) {
        if (_value != value) {
            _value = value;
            markDirty();
        }
    }

    std::string toString() override { return "TODO"; }

    void readValue(ZigBeeStream& memory) override { _value = memory.readInt16Le(); }

    void writeValue(ZigBeeStream& memory) override { memory.writeInt16Le(_value); }
};

class AttributeInt24 : public Attribute {
    int32_t _value{};

public:
    AttributeInt24(uint16_t attributeId, esp_zb_zcl_attr_type_t dataType) : Attribute(attributeId, dataType) {}

    int32_t getValue() { return _value; }

    void setValue(int32_t value) {
        if (_value != value) {
            _value = value;
            markDirty();
        }
    }

    std::string toString() override { return "TODO"; }

    void readValue(ZigBeeStream& memory) override { _value = memory.readInt24Le(); }

    void writeValue(ZigBeeStream& memory) override { memory.writeInt24Le(_value); }
};

class AttributeInt32 : public Attribute {
    int32_t _value{};

public:
    AttributeInt32(uint16_t attributeId, esp_zb_zcl_attr_type_t dataType) : Attribute(attributeId, dataType) {}

    int32_t getValue() { return _value; }

    void setValue(int32_t value) {
        if (_value != value) {
            _value = value;
            markDirty();
        }
    }

    std::string toString() override { return "TODO"; }

    void readValue(ZigBeeStream& memory) override { _value = memory.readInt32Le(); }

    void writeValue(ZigBeeStream& memory) override { memory.writeInt32Le(_value); }
};

class AttributeInt40 : public Attribute {
    int64_t _value{};

public:
    AttributeInt40(uint16_t attributeId, esp_zb_zcl_attr_type_t dataType) : Attribute(attributeId, dataType) {}

    int64_t getValue() { return _value; }

    void setValue(int64_t value) {
        if (_value != value) {
            _value = value;
            markDirty();
        }
    }

    std::string toString() override { return "TODO"; }

    void readValue(ZigBeeStream& memory) override { _value = memory.readInt40Le(); }

    void writeValue(ZigBeeStream& memory) override { memory.writeInt40Le(_value); }
};

class AttributeInt48 : public Attribute {
    int64_t _value{};

public:
    AttributeInt48(uint16_t attributeId, esp_zb_zcl_attr_type_t dataType) : Attribute(attributeId, dataType) {}

    int64_t getValue() { return _value; }

    void setValue(int64_t value) {
        if (_value != value) {
            _value = value;
            markDirty();
        }
    }

    std::string toString() override { return "TODO"; }

    void readValue(ZigBeeStream& memory) override { _value = memory.readInt48Le(); }

    void writeValue(ZigBeeStream& memory) override { memory.writeInt48Le(_value); }
};

class AttributeInt56 : public Attribute {
    int64_t _value{};

public:
    AttributeInt56(uint16_t attributeId, esp_zb_zcl_attr_type_t dataType) : Attribute(attributeId, dataType) {}

    int64_t getValue() { return _value; }

    void setValue(int64_t value) {
        if (_value != value) {
            _value = value;
            markDirty();
        }
    }

    std::string toString() override { return "TODO"; }

    void readValue(ZigBeeStream& memory) override { _value = memory.readInt56Le(); }

    void writeValue(ZigBeeStream& memory) override { memory.writeInt56Le(_value); }
};

class AttributeInt64 : public Attribute {
    int64_t _value{};

public:
    AttributeInt64(uint16_t attributeId, esp_zb_zcl_attr_type_t dataType) : Attribute(attributeId, dataType) {}

    int64_t getValue() { return _value; }

    void setValue(int64_t value) {
        if (_value != value) {
            _value = value;
            markDirty();
        }
    }

    std::string toString() override { return "TODO"; }

    void readValue(ZigBeeStream& memory) override { _value = memory.readInt64Le(); }

    void writeValue(ZigBeeStream& memory) override { memory.writeInt64Le(_value); }
};

class AttributeSingle : public Attribute {
    float _value{};

public:
    AttributeSingle(uint16_t attributeId, esp_zb_zcl_attr_type_t dataType) : Attribute(attributeId, dataType) {}

    float getValue() { return _value; }

    void setValue(float value) {
        if (_value != value) {
            _value = value;
            markDirty();
        }
    }

    std::string toString() override { return "TODO"; }

    void readValue(ZigBeeStream& memory) override { _value = memory.readSingle(); }

    void writeValue(ZigBeeStream& memory) override { memory.writeSingle(_value); }
};

class AttributeDouble : public Attribute {
    double _value{};

public:
    AttributeDouble(uint16_t attributeId, esp_zb_zcl_attr_type_t dataType) : Attribute(attributeId, dataType) {}

    double getValue() { return _value; }

    void setValue(double value) {
        if (_value != value) {
            _value = value;
            markDirty();
        }
    }

    std::string toString() override { return "TODO"; }

    void readValue(ZigBeeStream& memory) override { _value = memory.readDouble(); }

    void writeValue(ZigBeeStream& memory) override { memory.writeDouble(_value); }
};

class AttributeDateTime : public Attribute {
    DateTime _value{};

public:
    AttributeDateTime(uint16_t attributeId, esp_zb_zcl_attr_type_t dataType) : Attribute(attributeId, dataType) {}

    DateTime getValue() { return _value; }

    void setValue(DateTime value) {
        if (_value != value) {
            _value = value;
            markDirty();
        }
    }

    std::string toString() override { return "DATETIME"; }

    void readValue(ZigBeeStream& memory) override {
        // Has the date data type.
        memory.readUInt8();
        auto date = memory.readUInt32Le();
        // Has the time of day data type.
        memory.readUInt8();
        auto time = memory.readUInt32Le();
        _value = DateTime(date, time);
    }

    void writeValue(ZigBeeStream& memory) override {
        memory.writeUInt8((uint8_t)ESP_ZB_ZCL_ATTR_TYPE_DATE);
        memory.writeUInt32Le(_value.getDate());
        memory.writeUInt8((uint8_t)ESP_ZB_ZCL_ATTR_TYPE_TIME_OF_DAY);
        memory.writeUInt32Le(_value.getTime());
    }
};

// GENERATION END
