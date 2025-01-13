#include "support.h"

#include "Clusters.h"

#include "ZigBeeCore.h"

AttributeUInt8* GenOnOffCluster::getOnOff() {
    auto result = (AttributeUInt8*)getAttributeById((uint16_t)GenOnOffAttribute::OnOff);
    if (result == nullptr) {
        result = new AttributeUInt8(0, ESP_ZB_ZCL_ATTR_TYPE_BOOL);
        addAttribute(result);
    }
    return result;
}

AttributeUInt8* GenOnOffCluster::getGlobalSceneCtrl() {
    auto result = (AttributeUInt8*)getAttributeById((uint16_t)GenOnOffAttribute::GlobalSceneCtrl);
    if (result == nullptr) {
        result = new AttributeUInt8(16384, ESP_ZB_ZCL_ATTR_TYPE_BOOL);
        addAttribute(result);
    }
    return result;
}

AttributeUInt16* GenOnOffCluster::getOnTime() {
    auto result = (AttributeUInt16*)getAttributeById((uint16_t)GenOnOffAttribute::OnTime);
    if (result == nullptr) {
        result = new AttributeUInt16(16385, ESP_ZB_ZCL_ATTR_TYPE_U16);
        addAttribute(result);
    }
    return result;
}

AttributeUInt16* GenOnOffCluster::getOffWaitTime() {
    auto result = (AttributeUInt16*)getAttributeById((uint16_t)GenOnOffAttribute::OffWaitTime);
    if (result == nullptr) {
        result = new AttributeUInt16(16386, ESP_ZB_ZCL_ATTR_TYPE_U16);
        addAttribute(result);
    }
    return result;
}

AttributeUInt8* GenOnOffCluster::getStartUpOnOff() {
    auto result = (AttributeUInt8*)getAttributeById((uint16_t)GenOnOffAttribute::StartUpOnOff);
    if (result == nullptr) {
        result = new AttributeUInt8(16387, ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM);
        addAttribute(result);
    }
    return result;
}

AttributeUInt8* GenOnOffCluster::getMoesStartUpOnOff() {
    auto result = (AttributeUInt8*)getAttributeById((uint16_t)GenOnOffAttribute::MoesStartUpOnOff);
    if (result == nullptr) {
        result = new AttributeUInt8(32770, ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM);
        addAttribute(result);
    }
    return result;
}

void GenOnOffCluster::sendOffCommand(uint8_t endpointId) { sendMessage(endpointId, 0); }

void GenOnOffCluster::sendOnCommand(uint8_t endpointId) { sendMessage(endpointId, 1); }

void GenOnOffCluster::sendToggleCommand(uint8_t endpointId) { sendMessage(endpointId, 2); }

void GenOnOffCluster::sendOffWithEffectCommand(uint8_t endpointId, uint8_t effectid, uint8_t effectvariant) {
    uint8_t buffer[MAX_FRAME_DATA_SIZE];
    ZigBeeStream stream(buffer);

    stream.writeUInt8(effectid);
    stream.writeUInt8(effectvariant);

    sendMessage(endpointId, 64, stream);
}

void GenOnOffCluster::sendOnWithRecallGlobalSceneCommand(uint8_t endpointId) { sendMessage(endpointId, 65); }

void GenOnOffCluster::sendOnWithTimedOffCommand(uint8_t endpointId, uint8_t ctrlbits, uint16_t ontime,
                                                uint16_t offwaittime) {
    uint8_t buffer[MAX_FRAME_DATA_SIZE];
    ZigBeeStream stream(buffer);

    stream.writeUInt8(ctrlbits);
    stream.writeUInt16Le(ontime);
    stream.writeUInt16Le(offwaittime);

    sendMessage(endpointId, 66, stream);
}

esp_err_t GenOnOffCluster::zbCommand(const zb_zcl_parsed_hdr_t* cmd_info, ZigBeeStream& request,
                                     ZigBeeStream& response) {
    switch (cmd_info->cmd_id) {
        case 0: {
            return onOffCommand();
        }
        case 1: {
            return onOnCommand();
        }
        case 2: {
            return onToggleCommand();
        }
        case 64: {
            auto effectid = request.readUInt8();
            auto effectvariant = request.readUInt8();
            return onOffWithEffectCommand(effectid, effectvariant);
        }
        case 65: {
            return onOnWithRecallGlobalSceneCommand();
        }
        case 66: {
            auto ctrlbits = request.readUInt8();
            auto ontime = request.readUInt16Le();
            auto offwaittime = request.readUInt16Le();
            return onOnWithTimedOffCommand(ctrlbits, ontime, offwaittime);
        }
        default:
            return Cluster::zbCommand(cmd_info, request, response);
    }
}
