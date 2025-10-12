#include <Arduino.h>

#include <FS.h>
#include <cstdlib>
#include <esp_log.h>
#include <sqlite3.h>
#include <future>
#include <thread>
#include <array>
#include <numeric>
#include <LittleFS.h>

#include "Configuration.hpp"
#include "Database.hpp"
#include "Peripherals.hpp"
#include "Utils.hpp"
#include "Infos.hpp"
#include "RealTime.hpp"

namespace Database
{
    static sqlite3* db = nullptr;
    static std::vector<Infos::SensorData> rolling = {};
    static std::size_t index = 0u;

    static auto initializeDatabase() -> void
    {
        log_d( "begin" );

        sqlite3_initialize();

        const auto rc = sqlite3_open( "/littlefs/sensors_data.db", &db );
        if ( rc != SQLITE_OK )
        {
            log_e( "database open error: %s\n", sqlite3_errmsg( db ) );
            std::abort();
        }

        log_d( "end" );
    }

    static auto createTable() -> void
    {
        log_d( "begin" );
        {
            const auto query = " CREATE TABLE IF NOT EXISTS                    "
                               "     SENSORS_DATA (                            "
                               "         DATE_TIME       DATETIME PRIMARY KEY, "
                               "         TEMPERATURE     NUMERIC,              "
                               "         HUMIDITY        NUMERIC,              "
                               "         PRESSURE        NUMERIC,              "
                               "         WIND_SPEED      NUMERIC,              "
                               "         WIND_DIRECTION  INTEGER,              "
                               "         RAIN_INTENSITY  INTEGER               "
                               "     )                                         ";

            const auto rc = sqlite3_exec( db, query, nullptr, nullptr, nullptr );
            if ( rc != SQLITE_OK )
            {
                log_e( "table create error: %s\n", sqlite3_errmsg( db ) );
                std::abort();
            }
        }
        log_d( "end" );
    }

    static auto cleanup() -> void 
    {
        log_d("cleanup");

        const auto query = "DELETE FROM SENSORS_DATA "
                           "WHERE DATE_TIME <= ?";

        sqlite3_stmt* res;
        const auto rc = sqlite3_prepare_v2( db, query, strlen( query ), &res, nullptr );
        if ( rc != SQLITE_OK )
        {
            log_e( "cleanup prepare error: %s", sqlite3_errmsg( db ) );
            return;
        }

        sqlite3_bind_int64( res, 1, std::time(nullptr) - (45 * 24 * 60 * 60) );

        if ( sqlite3_step( res ) != SQLITE_DONE )
        {
            log_e( "cleanup error: %s", sqlite3_errmsg( db ) );
        }
        sqlite3_finalize( res );
    }

    static auto insert( const Infos::SensorData& sensorData ) -> void
    {
        log_d("insert");

        const auto query = " INSERT INTO SENSORS_DATA ( "
                           "     DATE_TIME,             "
                           "     TEMPERATURE,           "
                           "     HUMIDITY,              "
                           "     PRESSURE,              "
                           "     WIND_SPEED,            "
                           "     WIND_DIRECTION,        "
                           "     RAIN_INTENSITY         "
                           " )                          "
                           " VALUES                     "
                           "     (?,?,?,?,?,?,?)        ";

        sqlite3_stmt* res;
        const auto rc = sqlite3_prepare_v2( db, query, strlen( query ), &res, nullptr );
        if ( rc != SQLITE_OK )
        {
            log_e( "insert prepare error: %s", sqlite3_errmsg( db ) );
            return;
        }

        sqlite3_bind_int64( res, 1, sensorData.dateTime );
        sqlite3_bind_double( res, 2, sensorData.temperature );
        sqlite3_bind_double( res, 3, sensorData.humidity );
        sqlite3_bind_double( res, 4, sensorData.pressure );
        sqlite3_bind_double( res, 5, sensorData.windSpeed );
        sqlite3_bind_int( res, 6, static_cast<int>(sensorData.windDirection));
        sqlite3_bind_int( res, 7, static_cast<int>(sensorData.rainIntensity));
        if ( sqlite3_step( res ) != SQLITE_DONE )
        {
            log_e( "insert error: %s", sqlite3_errmsg( db ) );
        }
        sqlite3_finalize( res );
    }

    static auto generate() -> void
    {
        const auto current = Infos::SensorData::get();
        const auto average = Infos::SensorData{
            .dateTime = current.dateTime,
            .temperature = std::accumulate(rolling.begin(), rolling.end(), 0.0f, [](auto v, const auto& s){ return v + s.temperature; }) / rolling.size(),
            .humidity = std::accumulate(rolling.begin(), rolling.end(), 0.0f, [](auto v, const auto& s){ return v + s.humidity; }) / rolling.size(),
            .pressure = std::accumulate(rolling.begin(), rolling.end(), 0.0f, [](auto v, const auto& s){ return v + s.pressure; }) / rolling.size(),
            .windSpeed = std::accumulate(rolling.begin(), rolling.end(), 0.0f, [](auto v, const auto& s){ return v + s.windSpeed; }) / rolling.size(),
            .windDirection = current.windDirection,
            .rainIntensity = current.rainIntensity,
        };

        cleanup();
        insert( average );
    }

    static auto sample() -> void
    {
        if(rolling.size() < 90){
            rolling.emplace_back(Infos::SensorData::get());
        }
        else {
            rolling[index] = Infos::SensorData::get();
            index = (index + 1) % 90;
        }
    }


    auto init() -> void
    {
        log_d( "begin" );

        rolling.reserve(90);
        index = 0;

        initializeDatabase();
        createTable();

        log_d( "end" );
    }

    auto process() -> void
    {
        Utils::bound( std::chrono::seconds( 10 ), Database::sample );
        Utils::bound( std::chrono::minutes( 15 ), Database::generate );
    }

    Filter::Filter( std::chrono::system_clock::time_point start, std::chrono::system_clock::time_point end, uint32_t limit )
    {
        const auto query = " SELECT                                       "
                           "     DATE_TIME,                               "
                           "     TEMPERATURE,                             "
                           "     HUMIDITY,                                "
                           "     PRESSURE,                                "
                           "     WIND_SPEED,                              "
                           "     WIND_DIRECTION,                          "
                           "     RAIN_INTENSITY                           "
                           " FROM                                         "
                           "     SENSORS_DATA                             "
                           " WHERE                                        "
                           "         ( DATE_TIME >= IFNULL(?,DATE_TIME) ) "
                           "     AND ( DATE_TIME <= IFNULL(?,DATE_TIME) ) "
                           " ORDER BY                                     "
                           "     DATE_TIME ASC                            "
                           " LIMIT ?                                      ";

        const auto rc = sqlite3_prepare_v2( db, query, strlen( query ), &this->res, nullptr );
        if ( rc != SQLITE_OK )
        {
            log_d( "select prepare error: %s", sqlite3_errmsg( db ) );
            this->res = nullptr;
        }
        else
        {
            if ( start != std::chrono::system_clock::time_point::min() )
            {
                sqlite3_bind_int64( this->res, 1, std::chrono::system_clock::to_time_t( start ) );
            }
            if ( end != std::chrono::system_clock::time_point::max() )
            {
                sqlite3_bind_int64( this->res, 2, std::chrono::system_clock::to_time_t( end ) );
            }
            
            sqlite3_bind_int( this->res, 3, limit );
        }
    }

    Filter::Filter( Filter&& other )
    {
        this->res = other.res;
        other.res = nullptr;
    }

    Filter::~Filter()
    {
        if( this->res != nullptr )
        {
            sqlite3_finalize( this->res );
            this->res = nullptr;
        }
    }

    auto Filter::next() -> std::optional<Infos::SensorData>
    {
        if (this->res == nullptr)
        {
            return {};
        }

        if( sqlite3_step( this->res ) != SQLITE_ROW )
        {
            sqlite3_finalize( this->res );
            this->res = nullptr;
            return {};
        }

        return Infos::SensorData{
            .dateTime = static_cast<std::time_t>(sqlite3_column_int64( this->res, 0 )),
            .temperature = static_cast<float>(sqlite3_column_double( this->res, 1 )),
            .humidity = static_cast<float>(sqlite3_column_double( this->res, 2 )),
            .pressure = static_cast<float>(sqlite3_column_double( this->res, 3 )),
            .windSpeed = static_cast<float>(sqlite3_column_double( this->res, 4 )),
            .windDirection = static_cast<WindDirection>(sqlite3_column_int( this->res, 5 )),
            .rainIntensity = static_cast<RainIntensity>(sqlite3_column_int( this->res, 6 )),
        };
    }
} // namespace Database