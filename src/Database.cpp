#include <Arduino.h>

#include <FS.h>
#include <cstdlib>
#include <esp_log.h>
#include <sqlite3.h>
#include <future>
#include <thread>

#include "Configuration.hpp"
#include "Database.hpp"
#include "Peripherals.hpp"
#include "Utils.hpp"
#include "Infos.hpp"
#include "RealTime.hpp"

namespace Database
{
    static sqlite3* db = nullptr;

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
        const auto query = "DELETE FROM SENSORS_DATA "
                           "WHERE DATE_TIME < strftime('%s','now','-2 months')";

        char* errMsg = nullptr;
        const auto rc = sqlite3_exec(db, query, nullptr, nullptr, nullptr);
        if (rc != SQLITE_OK)
        {
            log_d("delete error: %s", sqlite3_errmsg( db ));
        }
    }

    static auto insert( const Infos::SensorData& sensorData ) -> void
    {
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
            log_d( "insert prepare error: %s", sqlite3_errmsg( db ) );
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
            log_d( "insert error: %s", sqlite3_errmsg( db ) );
        }
        sqlite3_finalize( res );
    }

    static auto generate() -> void
    {
        insert( Infos::SensorData::get() );
        cleanup();
    }

    auto init() -> void
    {
        log_d( "begin" );

        initializeDatabase();
        createTable();

        log_d( "end" );
    }

    auto process() -> void
    {
        Utils::bound( std::chrono::minutes( 15 ), Database::generate );
    }

    Filter::Filter( std::chrono::system_clock::time_point start, std::chrono::system_clock::time_point end )
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
                           "     DATE_TIME ASC                            ";

        const auto rc{sqlite3_prepare_v2( db, query, strlen( query ), &this->res, nullptr )};
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

    auto Filter::next() const -> std::optional<Infos::SensorData>
    {
        if ( this->res == nullptr )
        {
            return {};
        }

        if( sqlite3_step( this->res ) != SQLITE_ROW )
        {
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