#include <cstdlib>
#include <esp_log.h>
#include <driver/gpio.h>
#include <SPIFFS.h>

#include "Peripherals.hpp"

namespace Peripherals
{
    static SPIClass hspi{HSPI};

    auto init() -> void
    {
        log_d( "begin" );

        pinMode( Peripherals::BME280::SDA, INPUT_PULLUP );
        pinMode( Peripherals::BME280::SCL, INPUT_PULLUP );
        pinMode( Peripherals::DS3231::SQW_INT, INPUT_PULLUP );
        pinMode( Peripherals::DS3231::SDA, INPUT_PULLUP );
        pinMode( Peripherals::DS3231::SCL, INPUT_PULLUP );
        pinMode( Peripherals::BTN, INPUT_PULLUP );
        pinMode( Peripherals::LED_HTB, OUTPUT );

        digitalWrite( Peripherals::LED_HTB, LOW );

        SPIFFS.begin(true);

        log_d( "end" );
    }
}; // namespace Peripherals