#pragma once

#include <Arduino.h>

#include <ArduinoJson.hpp>
#include <array>
#include <map>

enum class WindDirection 
{
    NORTH     = 1,
    SOUTH     = 2,
    EAST      = 3,
    WEST      = 4,
    NORTHEAST = 5,
    SOUTHEAST = 6,
    SOUTHWEST = 7,
    NORTHWEST = 8,
};

enum class RainIntensity {
    DRY   = 0,
    HUMID = 1,
    RAINY = 2,
};

struct Configuration
{
    struct Station
    {
        bool enabled;
        std::array<uint8_t, 6> mac;
        std::array<uint8_t, 4> ip;
        std::array<uint8_t, 4> netmask;
        std::array<uint8_t, 4> gateway;
        uint16_t port;
        std::string user;
        std::string password;
    };

    struct AccessPoint
    {
        bool enabled;
        std::array<uint8_t, 6> mac;
        std::array<uint8_t, 4> ip;
        std::array<uint8_t, 4> netmask;
        std::array<uint8_t, 4> gateway;
        uint16_t port;
        std::string user;
        std::string password;
        uint16_t duration;
    };

    struct WindSpeed
    {
        float radius;
    };

    struct WindDirection
    {
        std::map<::WindDirection, std::pair<uint16_t, uint16_t>> threshoulds;
    };

    struct RainIntensity
    {
        std::map<::RainIntensity, std::pair<uint16_t, uint16_t>> threshoulds;
    };

    Station station;
    AccessPoint accessPoint;
    WindSpeed windSpeed;
    WindDirection windDirection;
    RainIntensity rainIntensity;

    static auto init() -> void;
    static auto load( Configuration* cfg ) -> void;
    static auto save( const Configuration& cfg ) -> void;

    auto serialize( ArduinoJson::JsonVariant& json ) const -> void;
    auto deserialize( const ArduinoJson::JsonVariant& json ) -> void;
};

extern Configuration cfg;