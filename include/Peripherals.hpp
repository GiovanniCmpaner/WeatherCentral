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
    };

    namespace DS3231
    {
        enum Pins
        {
            SCL = 22,
            SDA = 21
        };
        static constexpr uint8_t I2C_ADDRESS = 0x68;
    };

     namespace SD_CARD
    {
        enum Pins
        {
            SS = 5,
            MOSI = 23,
            MISO = 19,
            SCK = 18
        };
        static SPIClass SPI = {VSPI};
    };

    enum Pins
    {
        //LED_HTB = 22,

        WIND_SPEED = 35,
        WIND_DIRECTION = 32,
        RAIN_INTENSITY = 33,
    };

    auto init() -> void;
}; // namespace Peripherals