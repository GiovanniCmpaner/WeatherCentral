#include <Arduino.h>

#include <esp_log.h>
#include <esp32-hal.h>
#include <esp_sleep.h>
#include <esp_pthread.h>
#include <LittleFS.h>
#include <HTTPClient.h>

#include "Configuration.hpp"
#include "Database.hpp"
#include "Peripherals.hpp"
#include "RealTime.hpp"
#include "WebInterface.hpp"
#include "Infos.hpp"
#include "Utils.hpp"
#include "Indicator.hpp"

void setup()
{
    delay( 1000 );
    Serial.begin( 115200 );
    Serial.setDebugOutput( true );

    log_d( "begin" );

    Peripherals::init();
    Configuration::init();

    Configuration::load( &cfg );

    RealTime::init();
    Database::init();
    WebInterface::init();
    Infos::init();
    Indicator::init();

    log_d( "end" );
}

void loop()
{
    Infos::process();
    Database::process();
    RealTime::process();
    WebInterface::process();
    Indicator::process();
}