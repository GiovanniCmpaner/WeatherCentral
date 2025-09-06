#include <cstdlib>
#include <esp_log.h>
#include <driver/gpio.h>
#include <LittleFS.h>

#include "Peripherals.hpp"

namespace Peripherals
{
    auto init() -> void
    {
        log_d( "begin" );

        pinMode( Peripherals::BME280::SDA, INPUT_PULLUP );
        pinMode( Peripherals::BME280::SCL, INPUT_PULLUP );
        pinMode( Peripherals::DS3231::SDA, INPUT_PULLUP );
        pinMode( Peripherals::DS3231::SCL, INPUT_PULLUP );
        pinMode( Peripherals::LED_HTB, OUTPUT );
        pinMode( Peripherals::WIND_SPEED, INPUT );
        pinMode( Peripherals::WIND_DIRECTION, INPUT );
        pinMode( Peripherals::RAIN_INTENSITY, INPUT );

        digitalWrite( Peripherals::LED_HTB, LOW );

        LittleFS.begin(true);

        log_d( "end" );
    }
}; // namespace Peripherals