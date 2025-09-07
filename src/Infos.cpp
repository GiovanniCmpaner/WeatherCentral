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
#include <numeric>

#include "Configuration.hpp"
#include "Peripherals.hpp"
#include "Infos.hpp"
#include "Utils.hpp"

namespace Infos
{
    static std::chrono::system_clock::time_point updateTimer = {};

    static BME280I2C bme = {};

    static hw_timer_t* windSpeedTimer = nullptr;
    static std::atomic<uint32_t> windSpeedCounter = 0;

    static float pressure = NAN;
    static float temperature = NAN;
    static float humidity = NAN;
    static std::atomic<float> windSpeed = NAN;

    static std::pair<WindDirection, ResponsiveAnalogRead> windDirection = {WindDirection::NORTH, {}};
    static std::pair<RainIntensity, ResponsiveAnalogRead> rainIntensity = {RainIntensity::DRY, {}};

    static IRAM_ATTR auto windSpeedCalculate() -> void 
    {
        Infos::windSpeed = Infos::windSpeedCounter.exchange(0) * (2.0 * M_PI * cfg.windSpeed.radius) * 3.6;
    }

    static IRAM_ATTR auto windSpeedCount() -> void 
    {
        Infos::windSpeedCounter += 1;
    }

    auto init() -> void
    {
        if( not bme.begin() )
        {
            log_d( "bme error" );
        }

        windSpeedTimer = timerBegin(0, 80, true);
        timerAttachInterrupt(windSpeedTimer, Infos::windSpeedCalculate, false);
        timerAlarmWrite(windSpeedTimer, 1000000, true);
        timerAlarmEnable(windSpeedTimer);

        attachInterrupt(Peripherals::WIND_SPEED, Infos::windSpeedCount, FALLING);

        windDirection.second.begin(Peripherals::WIND_DIRECTION, false);
        windDirection.second.setAnalogResolution(4096);

        rainIntensity.second.begin(Peripherals::RAIN_INTENSITY, false);
        rainIntensity.second.setAnalogResolution(4096);
    }

    auto process() -> void
    {
        const auto now = std::chrono::system_clock::now();

        if (now - updateTimer >= std::chrono::milliseconds{1000})
        {
            updateTimer = now;

            bme.read( pressure, temperature, humidity, BME280::TempUnit_Celsius, BME280::PresUnit_hPa );

            windDirection.second.update();
            rainIntensity.second.update();

            for(const auto& [direction, threshould] : cfg.windDirection.threshoulds)
            {   
                if(windDirection.second.getValue() >= threshould.first && windDirection.second.getValue() <= threshould.second)
                {
                    windDirection.first = direction;
                    break;
                }
            }

            for(const auto& [intensity, threshould] : cfg.rainIntensity.threshoulds)
            {   
                if(rainIntensity.second.getValue() >= threshould.first && rainIntensity.second.getValue() <= threshould.second)
                {
                    rainIntensity.first = intensity;
                    break;
                }
            }
        }
    }

    auto SensorData::serialize( ArduinoJson::JsonVariant& json ) const -> void
    {
        json["datetime"] = Utils::DateTime::toString( std::chrono::system_clock::from_time_t( this->dateTime ) );
        json["temperature"] = this->temperature;
        json["humidity"] = this->humidity;
        json["pressure"] = this->pressure;
        json["wind_speed"] = this->windSpeed;
        json["wind_direction"] = Utils::WindDirection::getName(this->windDirection);
        json["rain_intensity"] = Utils::RainIntensity::getName(this->rainIntensity);
    }

    auto SensorData::get() -> SensorData
    {
        return
        {
            .dateTime = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() ),
            .temperature = Infos::temperature,
            .humidity = Infos::humidity,
            .pressure = Infos::pressure,
            .windSpeed = Infos::windSpeed,
            .windDirection = Infos::windDirection.first,
            .rainIntensity = Infos::rainIntensity.first,
        };
    }
} // namespace Sensors