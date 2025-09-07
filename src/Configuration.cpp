#include <Arduino.h>

#include <ArduinoJson.hpp>
#include <LittleFS.h>
#include <FastCRC.h>
#include <WiFi.h>
#include <cstdio>
#include <cstdlib>
#include <esp_log.h>
#include <string>

#include "Configuration.hpp"
#include "Peripherals.hpp"
#include "Utils.hpp"

static const Configuration defaultCfg
{
    .station = {
        .enabled = true,
        .mac = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED},
        .ip = {192, 168, 1, 200},
        .netmask = {255, 255, 255, 0},
        .gateway = {192, 168, 1, 1},
        .port = 80,
        .user = "WORKGROUP",
        .password = "49WNN7F3CD@22"
    },
    .accessPoint = {
        .enabled = false,
        .mac = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED},
        .ip = {192, 168, 1, 200},
        .netmask = {255, 255, 255, 0},
        .gateway = {192, 168, 1, 1},
        .port = 80,
        .user = "WeatherCentral",
        .password = "W3@th3rC3ntr4l",
        .duration = 30
    },
    .windSpeed = {
        .radius = 0.15,
    },
    .windDirection = {
        .threshoulds = {
            {
                {WindDirection::SOUTHWEST, {47, 52}},
                {WindDirection::SOUTH,     {518, 572}},
                {WindDirection::WEST,      {1078, 1192}},
                {WindDirection::SOUTHEAST, {1222, 1351}},
                {WindDirection::NORTHWEST, {1966, 2173}},
                {WindDirection::EAST,      {2508, 2772}},
                {WindDirection::NORTH,     {3094, 3420}},
                {WindDirection::NORTHEAST, {3809, 4095}},
            }
        }
    },
    .rainIntensity = {
        .threshoulds = {
            {RainIntensity::DRY,   {   0, 1000}},
            {RainIntensity::HUMID, {1001, 3000}},
            {RainIntensity::RAINY, {3001, 4095}},
        }
    }
};

static std::array<uint8_t, 6> stationMAC{};
static std::array<uint8_t, 6> accessPointMAC{};

auto Configuration::init() -> void
{
    log_d( "begin" );

    WiFi.mode( WIFI_MODE_APSTA );
    WiFi.macAddress( stationMAC.data() );
    WiFi.softAPmacAddress( accessPointMAC.data() );
    WiFi.mode( WIFI_MODE_NULL );

    log_d( "end" );
}

auto Configuration::serialize( ArduinoJson::JsonVariant& json ) const -> void
{
    {
        json["access_point"]["enabled"] = this->accessPoint.enabled;
        for ( auto n : accessPointMAC )
        {
            json["access_point"]["mac"].add( n );
        }
        for ( auto n : this->accessPoint.ip )
        {
            json["access_point"]["ip"].add( n );
        }
        for ( auto n : this->accessPoint.netmask )
        {
            json["access_point"]["netmask"].add( n );
        }
        for ( auto n : this->accessPoint.gateway )
        {
            json["access_point"]["gateway"].add( n );
        }
        json["access_point"]["port"] = this->accessPoint.port;
        json["access_point"]["user"] = this->accessPoint.user;
        json["access_point"]["password"] = this->accessPoint.password;
        json["access_point"]["duration"] = this->accessPoint.duration;
    }
    {
        json["station"]["enabled"] = this->station.enabled;
        for ( auto n : stationMAC )
        {
            json["station"]["mac"].add( n );
        }
        for ( auto n : this->station.ip )
        {
            json["station"]["ip"].add( n );
        }
        for ( auto n : this->station.netmask )
        {
            json["station"]["netmask"].add( n );
        }
        for ( auto n : this->station.gateway )
        {
            json["station"]["gateway"].add( n );
        }
        json["station"]["port"] = this->station.port;
        json["station"]["user"] = this->station.user;
        json["station"]["password"] = this->station.password;
    }
    {
        json["wind_speed"]["radius"] = this->windSpeed.radius;
    }
    {
        for(auto& [direction, threshould] : this->windDirection.threshoulds) 
        {
            json["wind_direction"]["threshoulds"][Utils::WindDirection::getName(direction)]["min"] = threshould.first;
            json["wind_direction"]["threshoulds"][Utils::WindDirection::getName(direction)]["max"] = threshould.second;
        }
    }
    {
        for(auto& [intensity, threshould] : this->rainIntensity.threshoulds) 
        {
            json["rain_intensity"]["threshoulds"][Utils::RainIntensity::getName(intensity)]["min"] = threshould.first;
            json["rain_intensity"]["threshoulds"][Utils::RainIntensity::getName(intensity)]["max"] = threshould.second;
        }
    }
}

auto Configuration::deserialize( const ArduinoJson::JsonVariant& json ) -> void
{
    if(json.containsKey("station"))
    {
        this->station.enabled = json["station"]["enabled"] | true;
        this->station.mac = stationMAC;

        this->station.ip[0] = json["station"]["ip"][0] | 192;
        this->station.ip[1] = json["station"]["ip"][1] | 168;
        this->station.ip[2] = json["station"]["ip"][2] | 1;
        this->station.ip[3] = json["station"]["ip"][3] | 200;

        this->station.netmask[0] = json["netmask"]["ip"][0] | 255;
        this->station.netmask[1] = json["netmask"]["ip"][1] | 255;
        this->station.netmask[2] = json["netmask"]["ip"][2] | 255;
        this->station.netmask[3] = json["netmask"]["ip"][3] | 0;

        this->station.gateway[0] = json["gateway"]["ip"][0] | 192;
        this->station.gateway[1] = json["gateway"]["ip"][1] | 168;
        this->station.gateway[2] = json["gateway"]["ip"][2] | 1;
        this->station.gateway[3] = json["gateway"]["ip"][3] | 1;

        this->station.port = json["station"]["port"] | 80;
        this->station.user = json["station"]["user"] | "WORKGROUP";
        this->station.password = json["station"]["password"] | "49WNN7F3CD@22";
    }

    if(json.containsKey("access_point"))
    {
        this->accessPoint.enabled = json["access_point"]["enabled"] | false;
        this->accessPoint.mac = accessPointMAC;

        this->accessPoint.ip[0] = json["access_point"]["ip"][0] | 192;
        this->accessPoint.ip[1] = json["access_point"]["ip"][1] | 168;
        this->accessPoint.ip[2] = json["access_point"]["ip"][2] | 1;
        this->accessPoint.ip[3] = json["access_point"]["ip"][3] | 200;

        this->accessPoint.netmask[0] = json["netmask"]["ip"][0] | 255;
        this->accessPoint.netmask[1] = json["netmask"]["ip"][1] | 255;
        this->accessPoint.netmask[2] = json["netmask"]["ip"][2] | 255;
        this->accessPoint.netmask[3] = json["netmask"]["ip"][3] | 0;

        this->accessPoint.gateway[0] = json["gateway"]["ip"][0] | 192;
        this->accessPoint.gateway[1] = json["gateway"]["ip"][1] | 168;
        this->accessPoint.gateway[2] = json["gateway"]["ip"][2] | 1;
        this->accessPoint.gateway[3] = json["gateway"]["ip"][3] | 1;

        this->accessPoint.port = json["access_point"]["port"] | 80;
        this->accessPoint.user = json["access_point"]["user"] | "WeatherCentral";
        this->accessPoint.password = json["access_point"]["password"] | "W3@th3rC3ntr4l";
        this->accessPoint.duration = json["access_point"]["duration"] | 30;
    }

    if(json.containsKey("wind_speed"))
    {
        this->windSpeed.radius = json["wind_speed"]["radius"] | 1.0;
    }

    if(json.containsKey("wind_direction"))
    {
        for(auto& [direction, threshould] : this->windDirection.threshoulds) 
        {
            threshould.first = json["wind_direction"]["threshoulds"][Utils::WindDirection::getName(direction)]["min"] | 0;
            threshould.second = json["wind_direction"]["threshoulds"][Utils::WindDirection::getName(direction)]["max"] | 0;
        }
    }

    if(json.containsKey("rain_intensity"))
    {
        for(auto& [intensity, threshould] : this->rainIntensity.threshoulds) 
        {
            threshould.first = json["rain_intensity"]["threshoulds"][Utils::RainIntensity::getName(intensity)]["min"] | 0;
            threshould.second = json["rain_intensity"]["threshoulds"][Utils::RainIntensity::getName(intensity)]["max"] | 0;
        }
    }
}

auto Configuration::load( Configuration* cfg ) -> void
{
    log_d( "begin" );

    *cfg = defaultCfg;

    if ( not LittleFS.exists( "/configuration.json" ) )
    {
        log_d( "file not found" );
    }
    else
    {
        auto file{LittleFS.open( "/configuration.json", FILE_READ )};
        if ( not file )
        {
            log_e( "file error" );
        }
        else
        {
            auto doc{ArduinoJson::DynamicJsonDocument{3072}};
            auto err{ArduinoJson::deserializeJson( doc, file )};
            file.close();

            if ( err != ArduinoJson::DeserializationError::Ok )
            {
                log_d( "json error = %s", err.c_str() );
            }
            else
            {
                auto json{doc.as<ArduinoJson::JsonVariant>()};
                cfg->deserialize( json );
            }
        }
    }

    Configuration::save( *cfg );

    log_d( "end" );
}

auto Configuration::save( const Configuration& cfg ) -> void
{
    log_d( "begin" );

    auto file{LittleFS.open( "/configuration.json", FILE_WRITE )};
    if ( not file )
    {
        log_e( "file error" );
        std::abort();
    }

    auto doc{ArduinoJson::DynamicJsonDocument{3072}};
    auto json{doc.as<ArduinoJson::JsonVariant>()};

    cfg.serialize( json );

    ArduinoJson::serializeJsonPretty( doc, file );
    file.close();

    log_d( "end" );
}

Configuration cfg{};