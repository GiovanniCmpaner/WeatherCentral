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
        static constexpr uint8_t I2C_ADDRESS{0x76};
    }; // namespace BME280

    namespace DS3231
    {
        enum Pins
        {
            SQW_INT = 34,
            SCL = 22,
            SDA = 21
        };
        static constexpr uint8_t I2C_ADDRESS{0x68};
    }; // namespace DS3231

    enum Pins
    {
        BTN = 4,
        LED_HTB = 16,
    };

    auto init() -> void;
}; // namespace Peripherals