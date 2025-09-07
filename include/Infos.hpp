#pragma once

#include <Arduino.h>
#include <ArduinoJson.hpp>

#include "Configuration.hpp"

namespace Infos
{
    struct SensorData
    {
        std::time_t dateTime;
        float temperature;
        float humidity;
        float pressure;
        float windSpeed;
        WindDirection windDirection;
        RainIntensity rainIntensity;

        static auto get() -> SensorData;
        auto serialize ( ArduinoJson::JsonVariant& json ) const -> void;
    };

    auto init() -> void;
    auto process() -> void;
}