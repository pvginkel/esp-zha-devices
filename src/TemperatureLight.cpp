#include "support.h"

#include "TemperatureLight.h"

LOG_TAG(TemperatureLight);

template <class InterpolateAlgorithm>
void TemperatureLight<InterpolateAlgorithm>::setTemperature(uint16_t temperature, uint32_t time) {
    DEBUG(F("Setting temperature to "), temperature, F(" minimum "), _minimumTemperature, F(" maximum "),
          _maximumTemperature);

    if (temperature < _minimumTemperature) {
        temperature = _minimumTemperature;
    }
    if (temperature > _maximumTemperature) {
        temperature = _maximumTemperature;
    }

    _temperature = temperature;

    updateLevelAndTemperature(time);

    _temperatureChanged.call(temperature);
}

template <class InterpolateAlgorithm>
void TemperatureLight<InterpolateAlgorithm>::updateLevelAndTemperature(uint32_t time) {
    const auto range = float(_maximumTemperature - _minimumTemperature);
    const auto offset = float(_temperature - _minimumTemperature) / range;

    // We only use the first third of the HUE space. That's enough for
    // us to properly mix warm and cold white.

    auto mix = offset * 120.0f;

    auto interpolatedLevel = InterpolateAlgorithm::interpolate(_level);
    auto scaledLevel = scaleLightLevel(interpolatedLevel, _minimumLevel, _maximumLevel);
    auto color = hsi2rgb(mix, 100, scaledLevel);

    auto warmLevel = color.g;
    auto coldLevel = color.r;

    ESP_LOGD(TAG, "Level %f, interpolatedLevel %f, scaled %f, warm level %f, cold level %f", _level, interpolatedLevel,
             scaledLevel, warmLevel, coldLevel);

    _cold.setLevel(coldLevel, time);
    _warm.setLevel(warmLevel, time);
}
