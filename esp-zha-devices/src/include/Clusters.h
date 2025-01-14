#pragma once

#include "Cluster.h"

enum class GenBasicAttribute : uint16_t {
    ZclVersion = 0,
    AppVersion = 1,
    StackVersion = 2,
    HwVersion = 3,
    ManufacturerName = 4,
    ModelId = 5,
    DateCode = 6,
    PowerSource = 7,
    AppProfileVersion = 8,
    SwBuildId = 16384,
    LocationDesc = 16,
    PhysicalEnv = 17,
    DeviceEnabled = 18,
    AlarmMask = 19,
    DisableLocalConfig = 20,
    DevelcoPrimarySwVersion = 32768,
    DevelcoPrimaryHwVersion = 32800,
    DevelcoLedControl = 33024,
    SchneiderMeterRadioPower = 57856,
};

class GenBasicCluster : public Cluster {
public:
    GenBasicCluster(ClusterType type = ClusterType::Input) : Cluster(0, type) {}
    AttributeUInt8* getZclVersion();
    AttributeUInt8* getAppVersion();
    AttributeUInt8* getStackVersion();
    AttributeUInt8* getHwVersion();
    AttributeString* getManufacturerName();
    AttributeString* getModelId();
    AttributeString* getDateCode();
    AttributeUInt8* getPowerSource();
    AttributeUInt8* getAppProfileVersion();
    AttributeString* getSwBuildId();
    AttributeString* getLocationDesc();
    AttributeUInt8* getPhysicalEnv();
    AttributeUInt8* getDeviceEnabled();
    AttributeUInt8* getAlarmMask();
    AttributeUInt8* getDisableLocalConfig();

    virtual esp_err_t onResetFactDefaultCommand() { return ESP_ERR_NOT_SUPPORTED; }

    void sendResetFactDefaultCommand(uint8_t endpointId);

    esp_err_t zbCommand(const zb_zcl_parsed_hdr_t* cmd_info, ZigBeeStream& request, ZigBeeStream& response) override;
};

enum class GenIdentifyAttribute : uint16_t {
    IdentifyTime = 0,
    IdentifyCommissionState = 1,
};

class GenIdentifyCluster : public Cluster {
public:
    GenIdentifyCluster(ClusterType type = ClusterType::Input) : Cluster(3, type) {}
    AttributeUInt16* getIdentifyTime();

    virtual esp_err_t onIdentifyCommand(uint16_t identifytime) { return ESP_ERR_NOT_SUPPORTED; }

    void sendIdentifyCommand(uint8_t endpointId, uint16_t identifytime);

    virtual esp_err_t onIdentifyQueryCommand() { return ESP_ERR_NOT_SUPPORTED; }

    void sendIdentifyQueryCommand(uint8_t endpointId);

    virtual esp_err_t onEzmodeInvokeCommand(uint8_t action) { return ESP_ERR_NOT_SUPPORTED; }

    void sendEzmodeInvokeCommand(uint8_t endpointId, uint8_t action);

    virtual esp_err_t onUpdateCommissionStateCommand(uint8_t action, uint8_t commstatemask) {
        return ESP_ERR_NOT_SUPPORTED;
    }

    void sendUpdateCommissionStateCommand(uint8_t endpointId, uint8_t action, uint8_t commstatemask);

    virtual esp_err_t onTriggerEffectCommand(uint8_t effectid, uint8_t effectvariant) { return ESP_ERR_NOT_SUPPORTED; }

    void sendTriggerEffectCommand(uint8_t endpointId, uint8_t effectid, uint8_t effectvariant);

    esp_err_t zbCommand(const zb_zcl_parsed_hdr_t* cmd_info, ZigBeeStream& request, ZigBeeStream& response) override;
};

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

enum class GenLevelCtrlAttribute : uint16_t {
    CurrentLevel = 0,
    RemainingTime = 1,
    MinLevel = 2,
    MaxLevel = 3,
    Options = 15,
    OnOffTransitionTime = 16,
    OnLevel = 17,
    OnTransitionTime = 18,
    OffTransitionTime = 19,
    DefaultMoveRate = 20,
    StartUpCurrentLevel = 16384,
    ElkoStartUpCurrentLevel = 16384,
    UbisysMinimumOnLevel = 0,
    UbisysValveType = 1,
    UbisysCyclePeriod = 2,
    UbisysSeason = 3,
    UbisysBackupLevel = 4,
    UbisysAlternateBackupLevel = 5,
    UbisysLowerRange = 6,
    UbisysUpperRange = 7,
    UbisysPumpThresholdOn = 8,
    UbisysPumpThresholdOff = 9,
    UbisysHeatingDemandEnableThreshold = 10,
    UbisysHeatingDemandDisableThreshold = 11,
    UbisysCoolingDemandEnableThreshold = 12,
    UbisysCoolingDemandDisableThreshold = 13,
};

class GenLevelCtrlCluster : public Cluster {
public:
    GenLevelCtrlCluster(ClusterType type = ClusterType::Input) : Cluster(8, type) {}
    AttributeUInt8* getCurrentLevel();
    AttributeUInt16* getRemainingTime();
    AttributeUInt8* getMinLevel();
    AttributeUInt8* getMaxLevel();
    AttributeUInt8* getOptions();
    AttributeUInt16* getOnOffTransitionTime();
    AttributeUInt8* getOnLevel();
    AttributeUInt16* getOnTransitionTime();
    AttributeUInt16* getOffTransitionTime();
    AttributeUInt16* getDefaultMoveRate();
    AttributeUInt8* getStartUpCurrentLevel();

    virtual esp_err_t onMoveToLevelCommand(uint8_t level, uint16_t transtime) { return ESP_ERR_NOT_SUPPORTED; }

    void sendMoveToLevelCommand(uint8_t endpointId, uint8_t level, uint16_t transtime);

    virtual esp_err_t onMoveCommand(uint8_t movemode, uint8_t rate) { return ESP_ERR_NOT_SUPPORTED; }

    void sendMoveCommand(uint8_t endpointId, uint8_t movemode, uint8_t rate);

    virtual esp_err_t onStepCommand(uint8_t stepmode, uint8_t stepsize, uint16_t transtime) {
        return ESP_ERR_NOT_SUPPORTED;
    }

    void sendStepCommand(uint8_t endpointId, uint8_t stepmode, uint8_t stepsize, uint16_t transtime);

    virtual esp_err_t onStopCommand() { return ESP_ERR_NOT_SUPPORTED; }

    void sendStopCommand(uint8_t endpointId);

    virtual esp_err_t onMoveToLevelWithOnOffCommand(uint8_t level, uint16_t transtime) { return ESP_ERR_NOT_SUPPORTED; }

    void sendMoveToLevelWithOnOffCommand(uint8_t endpointId, uint8_t level, uint16_t transtime);

    virtual esp_err_t onMoveWithOnOffCommand(uint8_t movemode, uint8_t rate) { return ESP_ERR_NOT_SUPPORTED; }

    void sendMoveWithOnOffCommand(uint8_t endpointId, uint8_t movemode, uint8_t rate);

    virtual esp_err_t onStepWithOnOffCommand(uint8_t stepmode, uint8_t stepsize, uint16_t transtime) {
        return ESP_ERR_NOT_SUPPORTED;
    }

    void sendStepWithOnOffCommand(uint8_t endpointId, uint8_t stepmode, uint8_t stepsize, uint16_t transtime);

    virtual esp_err_t onStopWithOnOffCommand() { return ESP_ERR_NOT_SUPPORTED; }

    void sendStopWithOnOffCommand(uint8_t endpointId);

    virtual esp_err_t onMoveToLevelTuyaCommand(uint16_t level, uint16_t transtime) { return ESP_ERR_NOT_SUPPORTED; }

    void sendMoveToLevelTuyaCommand(uint8_t endpointId, uint16_t level, uint16_t transtime);

    esp_err_t zbCommand(const zb_zcl_parsed_hdr_t* cmd_info, ZigBeeStream& request, ZigBeeStream& response) override;
};

enum class LightingColorCtrlAttribute : uint16_t {
    CurrentHue = 0,
    CurrentSaturation = 1,
    RemainingTime = 2,
    CurrentX = 3,
    CurrentY = 4,
    DriftCompensation = 5,
    CompensationText = 6,
    ColorTemperature = 7,
    ColorMode = 8,
    Options = 15,
    NumPrimaries = 16,
    Primary1X = 17,
    Primary1Y = 18,
    Primary1Intensity = 19,
    Primary2X = 21,
    Primary2Y = 22,
    Primary2Intensity = 23,
    Primary3X = 25,
    Primary3Y = 26,
    Primary3Intensity = 27,
    Primary4X = 32,
    Primary4Y = 33,
    Primary4Intensity = 34,
    Primary5X = 36,
    Primary5Y = 37,
    Primary5Intensity = 38,
    Primary6X = 40,
    Primary6Y = 41,
    Primary6Intensity = 42,
    WhitePointX = 48,
    WhitePointY = 49,
    ColorPointRX = 50,
    ColorPointRY = 51,
    ColorPointRIntensity = 52,
    ColorPointGX = 54,
    ColorPointGY = 55,
    ColorPointGIntensity = 56,
    ColorPointBX = 58,
    ColorPointBY = 59,
    ColorPointBIntensity = 60,
    EnhancedCurrentHue = 16384,
    EnhancedColorMode = 16385,
    ColorLoopActive = 16386,
    ColorLoopDirection = 16387,
    ColorLoopTime = 16388,
    ColorLoopStartEnhancedHue = 16389,
    ColorLoopStoredEnhancedHue = 16390,
    ColorCapabilities = 16394,
    ColorTempPhysicalMin = 16395,
    ColorTempPhysicalMax = 16396,
    CoupleColorTempToLevelMin = 16397,
    StartUpColorTemperature = 16400,
    TuyaBrightness = 61441,
    TuyaRgbMode = 61440,
};
