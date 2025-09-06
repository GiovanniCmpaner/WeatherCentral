#include <Arduino.h>

#include <RtcDS3231.h>
#include <Wire.h>
#include <driver/rtc_io.h>
#include <esp_log.h>
#include <esp_sleep.h>

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <sys/time.h>
#include <thread>

#include "Configuration.hpp"
#include "Database.hpp"
#include "Peripherals.hpp"
#include "RealTime.hpp"
#include "Utils.hpp"

namespace RealTime
{
    static RtcDS3231<TwoWire> rtc{Wire};

    static auto syncDateTime() -> void
    {
        const auto time{timeval{static_cast<std::time_t>( rtc.GetDateTime().Unix32Time() )}};
        settimeofday( &time, nullptr );
    }

    static auto startHardware() -> void
    {
        rtc.Begin();

        if ( not rtc.IsDateTimeValid() && rtc.LastError() != Rtc_Wire_Error_None )
        {
            log_d( "rtc error: %d", rtc.LastError() );
        }

        rtc.SetIsRunning( true );
    }

    auto init() -> void
    {
        log_d( "begin" );

        startHardware();
        syncDateTime();

        log_d( "now = %s", Utils::DateTime::toString( std::chrono::system_clock::now() ).data() );

        log_d( "end" );
    }

    auto adjustDateTime( const std::chrono::system_clock::time_point& timePoint ) -> void
    {
        RtcDateTime rtcDateTime{};
        rtcDateTime.InitWithUnix32Time( std::chrono::system_clock::to_time_t( timePoint ) );
        rtc.SetDateTime( rtcDateTime );
        rtc.SetIsRunning( true );
    }

    auto process() -> void
    {
        Utils::periodic( std::chrono::minutes( 5 ), RealTime::syncDateTime );
    }
} // namespace RealTime