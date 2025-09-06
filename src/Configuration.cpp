#include <Arduino.h>

#include <ArduinoJson.hpp>
#include <SPIFFS.h>
#include <FastCRC.h>
#include <WiFi.h>
#include <cstdio>
#include <cstdlib>
#include <esp_log.h>
#include <string>

#include "Configuration.hpp"
#include "Peripherals.hpp"

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
        .radius = 1.0,
    },
    .windDirection = {
        .threshoulds = {
            {"Norte", {0, 0}},
            {"Sul", {0, 0}},
            {"Leste", {0, 0}},
            {"Oeste", {0, 0}},
            {"Nordeste", {0, 0}},
            {"Sudeste", {0, 0}},
            {"Sudoeste", {0, 0}},
            {"Noroeste", {0, 0}},
        }
    },
    .rainIntensity = {
        .threshoulds = {
            {"Seco", {0, 0}},
            {"Umido", {0, 0}},
            {"Chuvoso", {0, 0}},
            {"Tempestade", {0, 0}},
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
        json["wind_direction"]["threshoulds"]["leste"]["min"] = this->windDirection.threshoulds.at("Leste").first;
        json["wind_direction"]["threshoulds"]["leste"]["max"] = this->windDirection.threshoulds.at("Leste").second;
        json["wind_direction"]["threshoulds"]["nordeste"]["min"] = this->windDirection.threshoulds.at("Nordeste").first;
        json["wind_direction"]["threshoulds"]["nordeste"]["max"] = this->windDirection.threshoulds.at("Nordeste").second;
        json["wind_direction"]["threshoulds"]["noroeste"]["min"] = this->windDirection.threshoulds.at("Noroeste").first;
        json["wind_direction"]["threshoulds"]["noroeste"]["max"] = this->windDirection.threshoulds.at("Noroeste").second;
        json["wind_direction"]["threshoulds"]["norte"]["min"] = this->windDirection.threshoulds.at("Norte").first;
        json["wind_direction"]["threshoulds"]["norte"]["max"] = this->windDirection.threshoulds.at("Norte").second;
        json["wind_direction"]["threshoulds"]["oeste"]["min"] = this->windDirection.threshoulds.at("Oeste").first;
        json["wind_direction"]["threshoulds"]["oeste"]["max"] = this->windDirection.threshoulds.at("Oeste").second;
        json["wind_direction"]["threshoulds"]["sudeste"]["min"] = this->windDirection.threshoulds.at("Sudeste").first;
        json["wind_direction"]["threshoulds"]["sudeste"]["max"] = this->windDirection.threshoulds.at("Sudeste").second;        
        json["wind_direction"]["threshoulds"]["sudoeste"]["min"] = this->windDirection.threshoulds.at("Sudoeste").first;
        json["wind_direction"]["threshoulds"]["sudoeste"]["max"] = this->windDirection.threshoulds.at("Sudoeste").second;
        json["wind_direction"]["threshoulds"]["sul"]["min"] = this->windDirection.threshoulds.at("Sul").first;
        json["wind_direction"]["threshoulds"]["sul"]["max"] = this->windDirection.threshoulds.at("Sul").second;
    }
    {
        json["rain_intensity"]["threshoulds"]["seco"]["min"] = this->windDirection.threshoulds.at("Seco").first;
        json["rain_intensity"]["threshoulds"]["seco"]["max"] = this->windDirection.threshoulds.at("Seco").second;
        json["rain_intensity"]["threshoulds"]["umido"]["min"] = this->windDirection.threshoulds.at("Umido").first;
        json["rain_intensity"]["threshoulds"]["umido"]["max"] = this->windDirection.threshoulds.at("Umido").second;
        json["rain_intensity"]["threshoulds"]["chuvoso"]["min"] = this->windDirection.threshoulds.at("Chuvoso").first;
        json["rain_intensity"]["threshoulds"]["chuvoso"]["max"] = this->windDirection.threshoulds.at("Chuvoso").second;
        json["rain_intensity"]["threshoulds"]["tempestade"]["min"] = this->windDirection.threshoulds.at("Tempestade").first;
        json["rain_intensity"]["threshoulds"]["tempestade"]["max"] = this->windDirection.threshoulds.at("Tempestade").second;
    }
}

auto Configuration::deserialize( const ArduinoJson::JsonVariant& json ) -> void
{
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
    {
        this->windSpeed.radius = json["wind_speed"]["radius"] | 1.0;
    }
    {
        this->windDirection.threshoulds["Leste"].first = json["wind_direction"]["threshoulds"]["leste"]["min"];
        this->windDirection.threshoulds["Leste"].second = json["wind_direction"]["threshoulds"]["leste"]["max"];
        this->windDirection.threshoulds["Nordeste"].first = json["wind_direction"]["threshoulds"]["nordeste"]["min"];
        this->windDirection.threshoulds["Nordeste"].second = json["wind_direction"]["threshoulds"]["nordeste"]["max"];
        this->windDirection.threshoulds["Noroeste"].first = json["wind_direction"]["threshoulds"]["noroeste"]["min"];
        this->windDirection.threshoulds["Noroeste"].second = json["wind_direction"]["threshoulds"]["noroeste"]["max"];
        this->windDirection.threshoulds["Norte"].first = json["wind_direction"]["threshoulds"]["norte"]["min"];
        this->windDirection.threshoulds["Norte"].second = json["wind_direction"]["threshoulds"]["norte"]["max"];
        this->windDirection.threshoulds["Oeste"].first = json["wind_direction"]["threshoulds"]["oeste"]["min"];
        this->windDirection.threshoulds["Oeste"].second = json["wind_direction"]["threshoulds"]["oeste"]["max"];
        this->windDirection.threshoulds["Sudeste"].first = json["wind_direction"]["threshoulds"]["sudeste"]["min"];
        this->windDirection.threshoulds["Sudeste"].second = json["wind_direction"]["threshoulds"]["sudeste"]["max"];        
        this->windDirection.threshoulds["Sudoeste"].first = json["wind_direction"]["threshoulds"]["sudoeste"]["min"];
        this->windDirection.threshoulds["Sudoeste"].second = json["wind_direction"]["threshoulds"]["sudoeste"]["max"];
        this->windDirection.threshoulds["Sul"].first = json["wind_direction"]["threshoulds"]["sul"]["min"];
        this->windDirection.threshoulds["Sul"].second = json["wind_direction"]["threshoulds"]["sul"]["max"];
    }
    {
        this->windDirection.threshoulds["Seco"].first = json["rain_intensity"]["threshoulds"]["seco"]["min"];
        this->windDirection.threshoulds["Seco"].second = json["rain_intensity"]["threshoulds"]["seco"]["max"];
        this->windDirection.threshoulds["Umido"].first = json["rain_intensity"]["threshoulds"]["umido"]["min"];
        this->windDirection.threshoulds["Umido"].second = json["rain_intensity"]["threshoulds"]["umido"]["max"];
        this->windDirection.threshoulds["Chuvoso"].first = json["rain_intensity"]["threshoulds"]["chuvoso"]["min"];
        this->windDirection.threshoulds["Chuvoso"].second = json["rain_intensity"]["threshoulds"]["chuvoso"]["max"];
        this->windDirection.threshoulds["Tempestade"].first = json["rain_intensity"]["threshoulds"]["tempestade"]["min"];
        this->windDirection.threshoulds["Tempestade"].second = json["rain_intensity"]["threshoulds"]["tempestade"]["max"];
    }
}

auto Configuration::load( Configuration* cfg ) -> void
{
    log_d( "begin" );

    *cfg = defaultCfg;

    if ( not SPIFFS.exists( "/configuration.json" ) )
    {
        log_d( "file not found" );
    }
    else
    {
        auto file{SPIFFS.open( "/configuration.json", FILE_READ )};
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

    auto file{SPIFFS.open( "/configuration.json", FILE_WRITE )};
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