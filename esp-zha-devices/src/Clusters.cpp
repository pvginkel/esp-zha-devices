#include "support.h"

#include "Clusters.h"

#include "ZigBeeCore.h"

AttributeUInt8* GenBasicCluster::getZclVersion() {
    auto result = (AttributeUInt8*)getAttributeById((uint16_t)GenBasicAttribute::ZclVersion);
    if (result == nullptr) {
        result = new AttributeUInt8(0, ESP_ZB_ZCL_ATTR_TYPE_U8);
        addAttribute(result);
    }
    return result;
}

AttributeUInt8* GenBasicCluster::getAppVersion() {
    auto result = (AttributeUInt8*)getAttributeById((uint16_t)GenBasicAttribute::AppVersion);
    if (result == nullptr) {
        result = new AttributeUInt8(1, ESP_ZB_ZCL_ATTR_TYPE_U8);
        addAttribute(result);
    }
    return result;
}

AttributeUInt8* GenBasicCluster::getStackVersion() {
    auto result = (AttributeUInt8*)getAttributeById((uint16_t)GenBasicAttribute::StackVersion);
    if (result == nullptr) {
        result = new AttributeUInt8(2, ESP_ZB_ZCL_ATTR_TYPE_U8);
        addAttribute(result);
    }
    return result;
}

AttributeUInt8* GenBasicCluster::getHwVersion() {
    auto result = (AttributeUInt8*)getAttributeById((uint16_t)GenBasicAttribute::HwVersion);
    if (result == nullptr) {
        result = new AttributeUInt8(3, ESP_ZB_ZCL_ATTR_TYPE_U8);
        addAttribute(result);
    }
    return result;
}

AttributeString* GenBasicCluster::getManufacturerName() {
    auto result = (AttributeString*)getAttributeById((uint16_t)GenBasicAttribute::ManufacturerName);
    if (result == nullptr) {
        result = new AttributeString(4);
        addAttribute(result);
    }
    return result;
}

AttributeString* GenBasicCluster::getModelId() {
    auto result = (AttributeString*)getAttributeById((uint16_t)GenBasicAttribute::ModelId);
    if (result == nullptr) {
        result = new AttributeString(5);
        addAttribute(result);
    }
    return result;
}

AttributeString* GenBasicCluster::getDateCode() {
    auto result = (AttributeString*)getAttributeById((uint16_t)GenBasicAttribute::DateCode);
    if (result == nullptr) {
        result = new AttributeString(6);
        addAttribute(result);
    }
    return result;
}

AttributeUInt8* GenBasicCluster::getPowerSource() {
    auto result = (AttributeUInt8*)getAttributeById((uint16_t)GenBasicAttribute::PowerSource);
    if (result == nullptr) {
        result = new AttributeUInt8(7, ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM);
        addAttribute(result);
    }
    return result;
}

AttributeUInt8* GenBasicCluster::getAppProfileVersion() {
    auto result = (AttributeUInt8*)getAttributeById((uint16_t)GenBasicAttribute::AppProfileVersion);
    if (result == nullptr) {
        result = new AttributeUInt8(8, ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM);
        addAttribute(result);
    }
    return result;
}

AttributeString* GenBasicCluster::getSwBuildId() {
    auto result = (AttributeString*)getAttributeById((uint16_t)GenBasicAttribute::SwBuildId);
    if (result == nullptr) {
        result = new AttributeString(16384);
        addAttribute(result);
    }
    return result;
}

AttributeString* GenBasicCluster::getLocationDesc() {
    auto result = (AttributeString*)getAttributeById((uint16_t)GenBasicAttribute::LocationDesc);
    if (result == nullptr) {
        result = new AttributeString(16);
        addAttribute(result);
    }
    return result;
}

AttributeUInt8* GenBasicCluster::getPhysicalEnv() {
    auto result = (AttributeUInt8*)getAttributeById((uint16_t)GenBasicAttribute::PhysicalEnv);
    if (result == nullptr) {
        result = new AttributeUInt8(17, ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM);
        addAttribute(result);
    }
    return result;
}

AttributeUInt8* GenBasicCluster::getDeviceEnabled() {
    auto result = (AttributeUInt8*)getAttributeById((uint16_t)GenBasicAttribute::DeviceEnabled);
    if (result == nullptr) {
        result = new AttributeUInt8(18, ESP_ZB_ZCL_ATTR_TYPE_BOOL);
        addAttribute(result);
    }
    return result;
}

AttributeUInt8* GenBasicCluster::getAlarmMask() {
    auto result = (AttributeUInt8*)getAttributeById((uint16_t)GenBasicAttribute::AlarmMask);
    if (result == nullptr) {
        result = new AttributeUInt8(19, ESP_ZB_ZCL_ATTR_TYPE_8BITMAP);
        addAttribute(result);
    }
    return result;
}

AttributeUInt8* GenBasicCluster::getDisableLocalConfig() {
    auto result = (AttributeUInt8*)getAttributeById((uint16_t)GenBasicAttribute::DisableLocalConfig);
    if (result == nullptr) {
        result = new AttributeUInt8(20, ESP_ZB_ZCL_ATTR_TYPE_8BITMAP);
        addAttribute(result);
    }
    return result;
}

void GenBasicCluster::sendResetFactDefaultCommand(uint8_t endpointId) { sendMessage(endpointId, 0); }

esp_err_t GenBasicCluster::zbCommand(const zb_zcl_parsed_hdr_t* cmd_info, ZigBeeStream& request,
                                     ZigBeeStream& response) {
    switch (cmd_info->cmd_id) {
        case 0: {
            return onResetFactDefaultCommand();
        }
        default:
            return Cluster::zbCommand(cmd_info, request, response);
    }
}

AttributeUInt16* GenIdentifyCluster::getIdentifyTime() {
    auto result = (AttributeUInt16*)getAttributeById((uint16_t)GenIdentifyAttribute::IdentifyTime);
    if (result == nullptr) {
        result = new AttributeUInt16(0, ESP_ZB_ZCL_ATTR_TYPE_U16);
        addAttribute(result);
    }
    return result;
}

void GenIdentifyCluster::sendIdentifyCommand(uint8_t endpointId, uint16_t identifytime) {
    uint8_t buffer[MAX_FRAME_DATA_SIZE];
    ZigBeeStream stream(buffer);

    stream.writeUInt16Le(identifytime);

    sendMessage(endpointId, 0, stream);
}

void GenIdentifyCluster::sendIdentifyQueryCommand(uint8_t endpointId) { sendMessage(endpointId, 1); }

void GenIdentifyCluster::sendEzmodeInvokeCommand(uint8_t endpointId, uint8_t action) {
    uint8_t buffer[MAX_FRAME_DATA_SIZE];
    ZigBeeStream stream(buffer);

    stream.writeUInt8(action);

    sendMessage(endpointId, 2, stream);
}

void GenIdentifyCluster::sendUpdateCommissionStateCommand(uint8_t endpointId, uint8_t action, uint8_t commstatemask) {
    uint8_t buffer[MAX_FRAME_DATA_SIZE];
    ZigBeeStream stream(buffer);

    stream.writeUInt8(action);
    stream.writeUInt8(commstatemask);

    sendMessage(endpointId, 3, stream);
}

void GenIdentifyCluster::sendTriggerEffectCommand(uint8_t endpointId, uint8_t effectid, uint8_t effectvariant) {
    uint8_t buffer[MAX_FRAME_DATA_SIZE];
    ZigBeeStream stream(buffer);

    stream.writeUInt8(effectid);
    stream.writeUInt8(effectvariant);

    sendMessage(endpointId, 64, stream);
}

esp_err_t GenIdentifyCluster::zbCommand(const zb_zcl_parsed_hdr_t* cmd_info, ZigBeeStream& request,
                                        ZigBeeStream& response) {
    switch (cmd_info->cmd_id) {
        case 0: {
            auto identifytime = request.readUInt16Le();
            return onIdentifyCommand(identifytime);
        }
        case 1: {
            return onIdentifyQueryCommand();
        }
        case 2: {
            auto action = request.readUInt8();
            return onEzmodeInvokeCommand(action);
        }
        case 3: {
            auto action = request.readUInt8();
            auto commstatemask = request.readUInt8();
            return onUpdateCommissionStateCommand(action, commstatemask);
        }
        case 64: {
            auto effectid = request.readUInt8();
            auto effectvariant = request.readUInt8();
            return onTriggerEffectCommand(effectid, effectvariant);
        }
        default:
            return Cluster::zbCommand(cmd_info, request, response);
    }
}

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

AttributeUInt8* GenLevelCtrlCluster::getCurrentLevel() {
    auto result = (AttributeUInt8*)getAttributeById((uint16_t)GenLevelCtrlAttribute::CurrentLevel);
    if (result == nullptr) {
        result = new AttributeUInt8(0, ESP_ZB_ZCL_ATTR_TYPE_U8);
        addAttribute(result);
    }
    return result;
}

AttributeUInt16* GenLevelCtrlCluster::getRemainingTime() {
    auto result = (AttributeUInt16*)getAttributeById((uint16_t)GenLevelCtrlAttribute::RemainingTime);
    if (result == nullptr) {
        result = new AttributeUInt16(1, ESP_ZB_ZCL_ATTR_TYPE_U16);
        addAttribute(result);
    }
    return result;
}

AttributeUInt8* GenLevelCtrlCluster::getMinLevel() {
    auto result = (AttributeUInt8*)getAttributeById((uint16_t)GenLevelCtrlAttribute::MinLevel);
    if (result == nullptr) {
        result = new AttributeUInt8(2, ESP_ZB_ZCL_ATTR_TYPE_U8);
        addAttribute(result);
    }
    return result;
}

AttributeUInt8* GenLevelCtrlCluster::getMaxLevel() {
    auto result = (AttributeUInt8*)getAttributeById((uint16_t)GenLevelCtrlAttribute::MaxLevel);
    if (result == nullptr) {
        result = new AttributeUInt8(3, ESP_ZB_ZCL_ATTR_TYPE_U8);
        addAttribute(result);
    }
    return result;
}

AttributeUInt8* GenLevelCtrlCluster::getOptions() {
    auto result = (AttributeUInt8*)getAttributeById((uint16_t)GenLevelCtrlAttribute::Options);
    if (result == nullptr) {
        result = new AttributeUInt8(15, ESP_ZB_ZCL_ATTR_TYPE_8BITMAP);
        addAttribute(result);
    }
    return result;
}

AttributeUInt16* GenLevelCtrlCluster::getOnOffTransitionTime() {
    auto result = (AttributeUInt16*)getAttributeById((uint16_t)GenLevelCtrlAttribute::OnOffTransitionTime);
    if (result == nullptr) {
        result = new AttributeUInt16(16, ESP_ZB_ZCL_ATTR_TYPE_U16);
        addAttribute(result);
    }
    return result;
}

AttributeUInt8* GenLevelCtrlCluster::getOnLevel() {
    auto result = (AttributeUInt8*)getAttributeById((uint16_t)GenLevelCtrlAttribute::OnLevel);
    if (result == nullptr) {
        result = new AttributeUInt8(17, ESP_ZB_ZCL_ATTR_TYPE_U8);
        addAttribute(result);
    }
    return result;
}

AttributeUInt16* GenLevelCtrlCluster::getOnTransitionTime() {
    auto result = (AttributeUInt16*)getAttributeById((uint16_t)GenLevelCtrlAttribute::OnTransitionTime);
    if (result == nullptr) {
        result = new AttributeUInt16(18, ESP_ZB_ZCL_ATTR_TYPE_U16);
        addAttribute(result);
    }
    return result;
}

AttributeUInt16* GenLevelCtrlCluster::getOffTransitionTime() {
    auto result = (AttributeUInt16*)getAttributeById((uint16_t)GenLevelCtrlAttribute::OffTransitionTime);
    if (result == nullptr) {
        result = new AttributeUInt16(19, ESP_ZB_ZCL_ATTR_TYPE_U16);
        addAttribute(result);
    }
    return result;
}

AttributeUInt16* GenLevelCtrlCluster::getDefaultMoveRate() {
    auto result = (AttributeUInt16*)getAttributeById((uint16_t)GenLevelCtrlAttribute::DefaultMoveRate);
    if (result == nullptr) {
        result = new AttributeUInt16(20, ESP_ZB_ZCL_ATTR_TYPE_U16);
        addAttribute(result);
    }
    return result;
}

AttributeUInt8* GenLevelCtrlCluster::getStartUpCurrentLevel() {
    auto result = (AttributeUInt8*)getAttributeById((uint16_t)GenLevelCtrlAttribute::StartUpCurrentLevel);
    if (result == nullptr) {
        result = new AttributeUInt8(16384, ESP_ZB_ZCL_ATTR_TYPE_U8);
        addAttribute(result);
    }
    return result;
}

void GenLevelCtrlCluster::sendMoveToLevelCommand(uint8_t endpointId, uint8_t level, uint16_t transtime) {
    uint8_t buffer[MAX_FRAME_DATA_SIZE];
    ZigBeeStream stream(buffer);

    stream.writeUInt8(level);
    stream.writeUInt16Le(transtime);

    sendMessage(endpointId, 0, stream);
}

void GenLevelCtrlCluster::sendMoveCommand(uint8_t endpointId, uint8_t movemode, uint8_t rate) {
    uint8_t buffer[MAX_FRAME_DATA_SIZE];
    ZigBeeStream stream(buffer);

    stream.writeUInt8(movemode);
    stream.writeUInt8(rate);

    sendMessage(endpointId, 1, stream);
}

void GenLevelCtrlCluster::sendStepCommand(uint8_t endpointId, uint8_t stepmode, uint8_t stepsize, uint16_t transtime) {
    uint8_t buffer[MAX_FRAME_DATA_SIZE];
    ZigBeeStream stream(buffer);

    stream.writeUInt8(stepmode);
    stream.writeUInt8(stepsize);
    stream.writeUInt16Le(transtime);

    sendMessage(endpointId, 2, stream);
}

void GenLevelCtrlCluster::sendStopCommand(uint8_t endpointId) { sendMessage(endpointId, 3); }

void GenLevelCtrlCluster::sendMoveToLevelWithOnOffCommand(uint8_t endpointId, uint8_t level, uint16_t transtime) {
    uint8_t buffer[MAX_FRAME_DATA_SIZE];
    ZigBeeStream stream(buffer);

    stream.writeUInt8(level);
    stream.writeUInt16Le(transtime);

    sendMessage(endpointId, 4, stream);
}

void GenLevelCtrlCluster::sendMoveWithOnOffCommand(uint8_t endpointId, uint8_t movemode, uint8_t rate) {
    uint8_t buffer[MAX_FRAME_DATA_SIZE];
    ZigBeeStream stream(buffer);

    stream.writeUInt8(movemode);
    stream.writeUInt8(rate);

    sendMessage(endpointId, 5, stream);
}

void GenLevelCtrlCluster::sendStepWithOnOffCommand(uint8_t endpointId, uint8_t stepmode, uint8_t stepsize,
                                                   uint16_t transtime) {
    uint8_t buffer[MAX_FRAME_DATA_SIZE];
    ZigBeeStream stream(buffer);

    stream.writeUInt8(stepmode);
    stream.writeUInt8(stepsize);
    stream.writeUInt16Le(transtime);

    sendMessage(endpointId, 6, stream);
}

void GenLevelCtrlCluster::sendStopWithOnOffCommand(uint8_t endpointId) { sendMessage(endpointId, 7); }

void GenLevelCtrlCluster::sendMoveToLevelTuyaCommand(uint8_t endpointId, uint16_t level, uint16_t transtime) {
    uint8_t buffer[MAX_FRAME_DATA_SIZE];
    ZigBeeStream stream(buffer);

    stream.writeUInt16Le(level);
    stream.writeUInt16Le(transtime);

    sendMessage(endpointId, 240, stream);
}

esp_err_t GenLevelCtrlCluster::zbCommand(const zb_zcl_parsed_hdr_t* cmd_info, ZigBeeStream& request,
                                         ZigBeeStream& response) {
    switch (cmd_info->cmd_id) {
        case 0: {
            auto level = request.readUInt8();
            auto transtime = request.readUInt16Le();
            return onMoveToLevelCommand(level, transtime);
        }
        case 1: {
            auto movemode = request.readUInt8();
            auto rate = request.readUInt8();
            return onMoveCommand(movemode, rate);
        }
        case 2: {
            auto stepmode = request.readUInt8();
            auto stepsize = request.readUInt8();
            auto transtime = request.readUInt16Le();
            return onStepCommand(stepmode, stepsize, transtime);
        }
        case 3: {
            return onStopCommand();
        }
        case 4: {
            auto level = request.readUInt8();
            auto transtime = request.readUInt16Le();
            return onMoveToLevelWithOnOffCommand(level, transtime);
        }
        case 5: {
            auto movemode = request.readUInt8();
            auto rate = request.readUInt8();
            return onMoveWithOnOffCommand(movemode, rate);
        }
        case 6: {
            auto stepmode = request.readUInt8();
            auto stepsize = request.readUInt8();
            auto transtime = request.readUInt16Le();
            return onStepWithOnOffCommand(stepmode, stepsize, transtime);
        }
        case 7: {
            return onStopWithOnOffCommand();
        }
        case 240: {
            auto level = request.readUInt16Le();
            auto transtime = request.readUInt16Le();
            return onMoveToLevelTuyaCommand(level, transtime);
        }
        default:
            return Cluster::zbCommand(cmd_info, request, response);
    }
}