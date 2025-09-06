#include <ctime>
#include <unordered_map>
#include <sstream>
#include <iomanip>

#include "Utils.hpp"


namespace Utils
{
    static std::unordered_map<void( * )(), std::chrono::system_clock::time_point> timers{};
    auto periodic( std::chrono::milliseconds interval, void( *func )() ) -> void
    {
        if( interval > std::chrono::milliseconds( 0 ) )
        {
            const auto now = std::chrono::system_clock::now();
            auto timer = Utils::timers.find( func );
            if( timer != Utils::timers.end() )
            {
                if( now >= timer->second )
                {
                    func();
                    timer->second = now + interval;
                }
            }
            else
            {
                func();
                Utils::timers.insert( { func, now + interval } );
            }
        }
        else
        {
            func();
        }
    }

    auto bound( std::chrono::milliseconds interval, void( *func )() ) -> void
    {
        if( interval > std::chrono::milliseconds( 0 ) )
        {
            const auto now = std::chrono::system_clock::now();
            auto timer = Utils::timers.find( func );
            if( timer != Utils::timers.end() )
            {
                if( now >= timer->second )
                {
                    func();
                    timer->second = now + interval;
                }
            }
            else
            {
                Utils::timers.insert( { func, Utils::DateTime::ceil( now, interval ) } );
            }
        }
        else
        {
            func();
        }
    }

    namespace DateTime
    {
        auto fromString( const std::string& str ) -> std::chrono::system_clock::time_point
        {
            auto stream{std::istringstream{str}};
            auto time{std::tm{}};
            stream >> std::get_time( &time, "%Y-%m-%d %H:%M:%S" );
            return std::chrono::system_clock::from_time_t( std::mktime( &time ) );
        }

        auto toString( const std::chrono::system_clock::time_point& timePoint ) -> std::string
        {
            auto stream{std::ostringstream{}};
            auto time{std::chrono::system_clock::to_time_t( timePoint )};
            stream << std::put_time( std::localtime( &time ), "%Y-%m-%d %H:%M:%S" );
            return stream.str();
        }

        auto fromStringHttp( const std::string& str ) -> std::chrono::system_clock::time_point
        {
            auto stream{std::istringstream{str}};
            auto time{std::tm{}};
            stream >> std::get_time( &time, "%a, %d %b %Y %H:%M:%S GMT" );
            return std::chrono::system_clock::from_time_t( std::mktime( &time ) );
        }

        auto toStringHttp( const std::chrono::system_clock::time_point& timePoint ) -> std::string
        {
            auto stream{std::ostringstream{}};
            auto time{std::chrono::system_clock::to_time_t( timePoint )};
            stream << std::put_time( std::localtime( &time ), "%a, %d %b %Y %H:%M:%S GMT" );
            return stream.str();
        }

        auto compiled( const char* compiledDate, const char* compiledTime ) -> std::chrono::system_clock::time_point
        {
            auto stream{std::stringstream{}};
            stream << compiledDate << ' ' << compiledTime;
            auto time{std::tm{}};
            stream >> std::get_time( &time, "%b %d %Y %H:%M:%S" );
            return std::chrono::system_clock::from_time_t( std::mktime( &time ) );
        }

        auto ceil( const std::chrono::system_clock::time_point& timePoint, std::chrono::milliseconds duration ) -> std::chrono::system_clock::time_point
        {
            const auto milliseconds{ ( ( std::chrono::time_point_cast<std::chrono::milliseconds>( timePoint ).time_since_epoch().count() + duration.count() - 1 ) / duration.count() )* duration.count() };
            return std::chrono::system_clock::time_point{ std::chrono::milliseconds{milliseconds} };
        }

        auto floor( const std::chrono::system_clock::time_point& timePoint, std::chrono::milliseconds duration ) -> std::chrono::system_clock::time_point
        {
            const auto milliseconds{ ( std::chrono::time_point_cast<std::chrono::milliseconds>( timePoint ).time_since_epoch().count() / duration.count() )* duration.count() };
            return std::chrono::system_clock::time_point{ std::chrono::milliseconds{milliseconds} };
        }

        auto round( const std::chrono::system_clock::time_point& timePoint, std::chrono::milliseconds duration ) -> std::chrono::system_clock::time_point
        {
            const auto milliseconds{ ( ( std::chrono::time_point_cast<std::chrono::milliseconds>( timePoint ).time_since_epoch().count() + duration.count() / 2 ) / duration.count() )* duration.count() };
            return std::chrono::system_clock::time_point{ std::chrono::milliseconds{milliseconds} };
        }
    }
}