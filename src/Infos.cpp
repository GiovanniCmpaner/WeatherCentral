#include <Arduino.h>

#include <ResponsiveAnalogRead.h>
#include <array>
#include <chrono>
#include <future>
#include <thread>
#include <Wire.h>
#include <BME280I2C.h>
#include <map>
#include <algorithm>

#include "Configuration.hpp"
#include "Peripherals.hpp"
#include "Infos.hpp"
#include "Utils.hpp"

namespace Infos
{
    static BME280I2C bme = {};

    static float pressure = NAN;
    static float temperature = NAN;
    static float humidity = NAN;
    static float windSpeed = NAN;

    static std::pair<std::string, ResponsiveAnalogRead> windDirection = {"N/A", {Peripherals::WIND_DIRECTION, false}};
    static std::pair<std::string, ResponsiveAnalogRead> rainIntensity = {"N/A", {Peripherals::RAIN_INTENSITY, false}};

    static auto update() -> void
    {
        bme.read( pressure, temperature, humidity, BME280::TempUnit_Celsius, BME280::PresUnit_hPa );

        windDirection.second.update();
        rainIntensity.second.update();
    }

    auto getPressureHpa() -> float
    {
        return ( round( pressure * 100 ) / 100 );
    }

    auto getTemperatureCelsius() -> float
    {
        return ( round( temperature * 100 ) / 100 );
    }

    auto getHumidityPercentage() -> float
    {
        return ( round( humidity * 100 ) / 100 );
    }

    auto getWindSpeedKmh() -> float
    {
        return ( round( windSpeed * 100 ) / 100 );
    }

    auto init() -> void
    {
        if( not bme.begin() )
        {
            log_d( "bme error" );
        }

        attachInterrupt(Peripherals::WIND_SPEED, nullptr, FALLING);
    }

    auto process() -> void
    {
        Utils::periodic( std::chrono::milliseconds( 500 ), Infos::update );
    }

    auto serialize( ArduinoJson::JsonVariant& json ) -> void
    {
        json["temperature"] = Infos::getTemperatureCelsius();
        json["humidity"] = Infos::getHumidityPercentage();
        json["pressure"] = Infos::getPressureHpa();
        json["wind_speed"] = Infos::getWindSpeedKmh();
        json["wind_direction"] = Infos::getWindDirection();
        json["rain_intensity"] = Infos::getRainIntensity();
    }
} // namespace Sensors