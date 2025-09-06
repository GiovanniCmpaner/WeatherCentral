#pragma once

#include <Arduino.h>
#include <ArduinoJson.hpp>
#include <functional>
#include <chrono>
#include <sqlite3.h>

namespace Database
{
    struct SensorData
    {
        std::int64_t id;
        std::time_t dateTime;
        double temperature;
        double humidity;
        double pressure;
        double windSpeed;
        std::string windDirection;
        std::string rainIntensity;

        static auto get() -> SensorData;
        auto serialize ( ArduinoJson::JsonVariant& json ) const -> void;
    };

    class Filter
    {
        private:
            sqlite3_stmt* res;
        public:
            Filter( int64_t id = 0, std::chrono::system_clock::time_point start = std::chrono::system_clock::time_point::min(), std::chrono::system_clock::time_point end = std::chrono::system_clock::time_point::max() );
            Filter( Filter& ) = delete;
            Filter( Filter&& );
            ~Filter();

            auto next( SensorData* sensorData ) const -> bool;
    };

    auto init() -> void;
    auto process() -> void;
} // namespace Database