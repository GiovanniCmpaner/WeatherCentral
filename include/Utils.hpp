#pragma once

#include <chrono>

namespace Utils
{
    auto periodic( std::chrono::milliseconds interval, void( *func )() ) -> void;
    auto bound( std::chrono::milliseconds interval, void( *func )() ) -> void;

    namespace WindDirection 
    {
        auto getName(::WindDirection dir) -> std::string;
        auto getValue(const std::string& name) -> ::WindDirection;
    }

    namespace RainIntensity 
    {
        auto getName(::RainIntensity dir) -> std::string;
        auto getValue(const std::string& name) -> ::RainIntensity;
    }

    namespace DateTime
    {
        auto fromString( const std::string& str ) -> std::chrono::system_clock::time_point;
        auto toString( const std::chrono::system_clock::time_point& timePoint ) -> std::string;
        auto fromStringHttp( const std::string& str ) -> std::chrono::system_clock::time_point;
        auto toStringHttp( const std::chrono::system_clock::time_point& timePoint ) -> std::string;
        auto compiled(const char* compiledDate = __DATE__, const char* compiledTime = __TIME__) -> std::chrono::system_clock::time_point;
        auto ceil( const std::chrono::system_clock::time_point& timePoint, std::chrono::milliseconds duration ) -> std::chrono::system_clock::time_point;
        auto floor( const std::chrono::system_clock::time_point& timePoint, std::chrono::milliseconds duration ) -> std::chrono::system_clock::time_point;
        auto round( const std::chrono::system_clock::time_point& timePoint, std::chrono::milliseconds duration ) -> std::chrono::system_clock::time_point;
    }
}