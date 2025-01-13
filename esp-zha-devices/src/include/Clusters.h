#pragma once

#include "Cluster.h"

enum class GenOnOffAttribute : uint16_t {
    OnOff = 0,
    GlobalSceneCtrl = 16384,
    OnTime = 16385,
    OffWaitTime = 16386,
    StartUpOnOff = 16387,
    TuyaBacklightSwitch = 20480,
    TuyaBacklightMode = 32769,
    MoesStartUpOnOff = 32770,
    TuyaOperationMode = 32772,
    ElkoPreWarningTime = 57344,
    ElkoOnTimeReload = 57345,
    ElkoOnTimeReloadOptions = 57346,
    NodonTransitionTime = 1,
};

class GenOnOffCluster : public Cluster {
public:
    GenOnOffCluster(ClusterType type = ClusterType::Input) : Cluster(6, type) {}
    AttributeUInt8* getOnOff();
    AttributeUInt8* getGlobalSceneCtrl();
    AttributeUInt16* getOnTime();
    AttributeUInt16* getOffWaitTime();
    AttributeUInt8* getStartUpOnOff();
    AttributeUInt8* getMoesStartUpOnOff();

    virtual esp_err_t onOffCommand() { return ESP_ERR_NOT_SUPPORTED; }

    void sendOffCommand(uint8_t endpointId);

    virtual esp_err_t onOnCommand() { return ESP_ERR_NOT_SUPPORTED; }

    void sendOnCommand(uint8_t endpointId);

    virtual esp_err_t onToggleCommand() { return ESP_ERR_NOT_SUPPORTED; }

    void sendToggleCommand(uint8_t endpointId);

    virtual esp_err_t onOffWithEffectCommand(uint8_t effectid, uint8_t effectvariant) { return ESP_ERR_NOT_SUPPORTED; }

    void sendOffWithEffectCommand(uint8_t endpointId, uint8_t effectid, uint8_t effectvariant);

    virtual esp_err_t onOnWithRecallGlobalSceneCommand() { return ESP_ERR_NOT_SUPPORTED; }

    void sendOnWithRecallGlobalSceneCommand(uint8_t endpointId);

    virtual esp_err_t onOnWithTimedOffCommand(uint8_t ctrlbits, uint16_t ontime, uint16_t offwaittime) {
        return ESP_ERR_NOT_SUPPORTED;
    }

    void sendOnWithTimedOffCommand(uint8_t endpointId, uint8_t ctrlbits, uint16_t ontime, uint16_t offwaittime);

    esp_err_t zbCommand(const zb_zcl_parsed_hdr_t* cmd_info, ZigBeeStream& request, ZigBeeStream& response) override;
};
