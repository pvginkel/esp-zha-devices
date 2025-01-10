#pragma once

#include "Callback.h"

// Taken from https://blog.saikoled.com/post/43693602826/why-every-led-light-should-be-using-hsi.
//
// Function example takes H, S, I, and a pointer to the
// returned RGB colorspace converted vector. It should
// be initialized with:
//
// int rgb[3];
//
// in the calling function. After calling hsi2rgb
// the vector rgb will contain red, green, and blue
// calculated values.

struct RGB {
    float r;
    float g;
    float b;
};

RGB hsi2rgb(float H, float S, float I);

uint16_t mired2kelvin(uint16_t mired);
uint16_t kelvin2mired(uint16_t kelvin);

float scaleLightLevel(float level, float minimumLevel, float maximumLevel);

template <class InterpolateAlgorithm>
class Light {
    Callback<float> _levelChanged;

    ledc_channel_t _pin{(ledc_channel_t)-1};
    float _level{};
    float _actualLevel{};
    float _startLevel{};
    float _minimumLevel{};
    float _maximumLevel{1};
    uint32_t _transitionStart{};
    uint32_t _lastUpdate{};
    uint32_t _transitionTime{};

public:
    Light() {}

    void reconfigure(float minimumLevel, float maximumLevel, uint32_t time = 0);
    void begin(ledc_channel_t pin);
    bool isOn() { return _level > 0; }
    float getLevel() { return _level; }
    void onLevelChanged(std::function<void(float)> func) { _levelChanged.add(func); }
    void update();
    void setLevel(float level, uint32_t time = 0);
    void resetTransition();

private:
    void updatePinValue();
    float getScaledLevel() { return scaleLightLevel(_level, _minimumLevel, _maximumLevel); }
    static uint8_t interpolate(float level);
};

class LinearInterpolateAlgorithm {
public:
    static float interpolate(float level) { return level; }
};

class CIE1931InterpolateAlgorithm {
public:
    static float interpolate(float level);
};

typedef Light<LinearInterpolateAlgorithm> LinearLight;
typedef Light<CIE1931InterpolateAlgorithm> NaturalLight;