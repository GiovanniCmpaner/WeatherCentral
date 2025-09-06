#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <mutex>

namespace Peripherals
{
    namespace BME280
    {
        enum Pins
        {
            SDA = 21,
            SCL = 22
        };
        static constexpr uint8_t I2C_ADDRESS = 0x76;
    }; // namespace BME280

    namespace DS3231
    {
        enum Pins
        {
            SCL = 22,
            SDA = 21
        };
        static constexpr uint8_t I2C_ADDRESS = 0x68;
    }; // namespace DS3231

    enum Pins
    {
        LED_HTB = 16,

        WIND_SPEED = 23,
        WIND_DIRECTION = 24,
        RAIN_INTENSITY = 25,
    };

    auto init() -> void;
}; // namespace Peripherals