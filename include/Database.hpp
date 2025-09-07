#pragma once

#include <Arduino.h>
#include <ArduinoJson.hpp>
#include <functional>
#include <chrono>
#include <sqlite3.h>

#include "Configuration.hpp"

namespace Database
{
    struct SensorData
    {
        std::time_t dateTime;
        double temperature;
        double humidity;
        double pressure;
        double windSpeed;
        WindDirection windDirection;
        RainIntensity rainIntensity;

        static auto get() -> SensorData;
        auto serialize ( ArduinoJson::JsonVariant& json ) const -> void;
    };

    class Filter
    {
        private:
            sqlite3_stmt* res;
        public:
            Filter( std::chrono::system_clock::time_point start = std::chrono::system_clock::time_point::min(), std::chrono::system_clock::time_point end = std::chrono::system_clock::time_point::max() );
            Filter( Filter& ) = delete;
            Filter( Filter&& );
            ~Filter();

            auto next( SensorData* sensorData ) const -> bool;
    };

    auto init() -> void;
    auto process() -> void;
} // namespace Database