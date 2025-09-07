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
        LED_HTB = 23,

        WIND_SPEED = 35,
        WIND_DIRECTION = 32,
        RAIN_INTENSITY = 33,
    };

    auto init() -> void;
}; // namespace Peripherals