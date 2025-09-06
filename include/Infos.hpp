#pragma once

#include <Arduino.h>

#include <ArduinoJson.hpp>

namespace Infos
{
    auto init() -> void;
    auto process() -> void;
    auto getPressureHpa() -> float;
    auto getTemperatureCelsius() -> float;
    auto getHumidityPercentage() -> float;
    auto getWindSpeedKmh() -> float;
    auto getWindDirection() -> std::string;
    auto getRainIntensity() -> std::string;

    auto serialize( ArduinoJson::JsonVariant& json ) -> void;
}