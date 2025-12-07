#pragma once

#include <Arduino.h>
#include <ArduinoJson.hpp>
#include <functional>
#include <chrono>
#include <sqlite3.h>
#include <optional>

#include "Configuration.hpp"
#include "Infos.hpp"

namespace Database
{
    class Filter
    {
        private:
            sqlite3_stmt* res = nullptr;
            uint32_t count = 0;
        public:
            Filter( std::chrono::system_clock::time_point start, std::chrono::system_clock::time_point end, uint32_t limit );
            Filter( Filter& ) = delete;
            Filter( Filter&& );
            ~Filter();

            auto next() -> std::optional<Infos::SensorData>;
    };

    auto init() -> void;
    auto process() -> void;
} // namespace Database