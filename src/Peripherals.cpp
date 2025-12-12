#include <cstdlib>
#include <esp_log.h>
#include <driver/gpio.h>
#include <LittleFS.h>
#include <SD.h>

#include "Peripherals.hpp"

namespace Peripherals
{
    auto init() -> void
    {
        log_d( "begin" );

        pinMode( BME280::SDA, INPUT_PULLUP );
        pinMode( BME280::SCL, INPUT_PULLUP );
        pinMode( DS3231::SDA, INPUT_PULLUP );
        pinMode( DS3231::SCL, INPUT_PULLUP );
        pinMode( SD_CARD::SS, OUTPUT );
        //pinMode( LED_HTB, OUTPUT );
        pinMode( WIND_SPEED, INPUT );
        pinMode( WIND_DIRECTION, INPUT );
        pinMode( RAIN_INTENSITY, INPUT );

        //digitalWrite( LED_HTB, LOW );
        digitalWrite( SD_CARD::SS, LOW );

        LittleFS.begin(true);

        if(not SD.begin(SD_CARD::SS, SD_CARD::SPI) or SD.cardType() == CARD_NONE) {
            log_e("sd error");
        }

        log_d( "end" );
    }
}; // namespace Peripherals