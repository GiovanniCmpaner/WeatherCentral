#pragma once

#include <Arduino.h>
#include <ArduinoJson.hpp>

#include "Configuration.hpp"

namespace Infos
{
    auto init() -> void;
    auto process() -> void;
    auto getPressureHpa() -> float;
    auto getTemperatureCelsius() -> float;
    auto getHumidityPercentage() -> float;
    auto getWindSpeedKmh() -> float;
    auto getWindDirection() -> WindDirection;
    auto getRainIntensity() -> RainIntensity;

    auto serialize( ArduinoJson::JsonVariant& json ) -> void;
}