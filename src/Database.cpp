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

    auto SensorData::serialize( ArduinoJson::JsonVariant& json ) const -> void
    {
        json["id"] = this->id;
        json["datetime"] = Utils::DateTime::toString( std::chrono::system_clock::from_time_t( this->dateTime ) );
        json["temperature"] = this->temperature;
        json["humidity"] = this->humidity;
        json["pressure"] = this->pressure;
        json["wind_speed"] = this->windSpeed;
        json["wind_direction"] = this->windDirection;
        json["rain_intensity"] = this->rainIntensity;
    }

    auto SensorData::get() -> SensorData
    {
        return
        {
            .id = 0,
            .dateTime = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() ),
            .temperature = Infos::getTemperatureCelsius(),
            .humidity = Infos::getHumidityPercentage(),
            .pressure = Infos::getPressureHpa(),
            .windSpeed = Infos::getWindSpeedKmh(),
            .windDirection = Infos::getWindDirection(),
            .rainIntensity = Infos::getRainIntensity(),
        };
    }

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
            const auto query = "CREATE TABLE IF NOT EXISTS                   "
                               "    SENSORS_DATA (                           "
                               "        ID              INTEGER PRIMARY KEY, "
                               "        DATE_TIME       DATETIME,            "
                               "        TEMPERATURE     NUMERIC,             "
                               "        HUMIDITY        NUMERIC,             "
                               "        PRESSURE        NUMERIC,             "
                               "        WIND_SPEED      NUMERIC,             "
                               "        WIND_DIRECTION  TEXT,                "
                               "        RAIN_INTENSITY  TEXT                 "
                               "    )                                        ";

            const auto rc = sqlite3_exec( db, query, nullptr, nullptr, nullptr );
            if ( rc != SQLITE_OK )
            {
                log_e( "table create error: %s\n", sqlite3_errmsg( db ) );
                std::abort();
            }
        }
        {
            const auto query{"CREATE UNIQUE INDEX IF NOT EXISTS DATE_TIME_INDEX "
                             "ON SENSORS_DATA( DATE_TIME )                      "};

            const auto rc{sqlite3_exec( db, query, nullptr, nullptr, nullptr )};
            if ( rc != SQLITE_OK )
            {
                log_e( "table index error: %s\n", sqlite3_errmsg( db ) );
                std::abort();
            }
        }
        log_d( "end" );
    }

    static auto insert( const SensorData& sensorData ) -> int64_t
    {
        const auto query
        {
            "INSERT INTO SENSORS_DATA ( "
            "    DATE_TIME,             "
            "    TEMPERATURE,           "
            "    HUMIDITY,              "
            "    PRESSURE,              "
            "    WIND_SPEED,            "
            "    WIND_DIRECTION,        "
            "    RAIN_INTENSITY         "
            ")                          "
            "VALUES                     "
            "    (?,?,?,?,?,?,?)        "};

        sqlite3_stmt* res;
        const auto rc{sqlite3_prepare_v2( db, query, strlen( query ), &res, nullptr )};
        if ( rc != SQLITE_OK )
        {
            log_d( "insert prepare error: %s", sqlite3_errmsg( db ) );
            return 0;
        }

        sqlite3_bind_int64( res, 1, sensorData.dateTime );
        sqlite3_bind_double( res, 2, sensorData.temperature );
        sqlite3_bind_double( res, 3, sensorData.humidity );
        sqlite3_bind_double( res, 4, sensorData.pressure );
        sqlite3_bind_double( res, 5, sensorData.windSpeed );
        sqlite3_bind_text( res, 6, sensorData.windDirection.c_str(), sensorData.windDirection.size(), SQLITE_TRANSIENT);
        sqlite3_bind_text( res, 7, sensorData.rainIntensity.c_str(), sensorData.rainIntensity.size(), SQLITE_TRANSIENT);
        if ( sqlite3_step( res ) != SQLITE_DONE )
        {
            log_d( "insert error: %s", sqlite3_errmsg( db ) );
        }
        sqlite3_finalize( res );
        return sqlite3_last_insert_rowid( db );
    }

    static auto generate() -> void
    {
        insert( SensorData::get() );
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
        Utils::bound( std::chrono::minutes( 5 ), Database::generate );
    }

    Filter::Filter( int64_t id, std::chrono::system_clock::time_point start, std::chrono::system_clock::time_point end )
    {
        const auto query
        {
            "SELECT                                       "
            "    ID,                                      "
            "    DATE_TIME,                               "
            "    TEMPERATURE,                             "
            "    HUMIDITY,                                "
            "    PRESSURE,                                "
            "    WIND_SPEED,                              "
            "    WIND_DIRECTION,                          "
            "    RAIN_INTENSITY                           "
            "FROM                                         "
            "    SENSORS_DATA                             "
            "WHERE                                        "
            "        ( ID >= IFNULL(?,ID) )               "
            "    AND ( DATE_TIME >= IFNULL(?,DATE_TIME) ) "
            "    AND ( DATE_TIME <= IFNULL(?,DATE_TIME) ) "
            "ORDER BY                                     "
            "    DATE_TIME ASC                            "};

        const auto rc{sqlite3_prepare_v2( db, query, strlen( query ), &this->res, nullptr )};
        if ( rc != SQLITE_OK )
        {
            log_d( "select prepare error: %s", sqlite3_errmsg( db ) );
            this->res = nullptr;
        }
        else
        {
            if ( id != int64_t{} )
            {
                sqlite3_bind_int64( this->res, 1, id );
            }
            if ( start != std::chrono::system_clock::time_point::min() )
            {
                sqlite3_bind_int64( this->res, 2, std::chrono::system_clock::to_time_t( start ) );
            }
            if ( end != std::chrono::system_clock::time_point::max() )
            {
                sqlite3_bind_int64( this->res, 3, std::chrono::system_clock::to_time_t( end ) );
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

    auto Filter::next( SensorData* sensorData ) const -> bool
    {
        if ( this->res == nullptr )
        {
            return false;
        }

        if( sqlite3_step( this->res ) != SQLITE_ROW )
        {
            return false;
        }

        sensorData->id = sqlite3_column_int64( this->res, 0 );
        sensorData->dateTime = sqlite3_column_int64( this->res, 1 );
        sensorData->temperature = sqlite3_column_double( this->res, 2 );
        sensorData->humidity = sqlite3_column_double( this->res, 3 );
        sensorData->pressure = sqlite3_column_double( this->res, 4 );
        sensorData->windSpeed = sqlite3_column_double( this->res, 5 );
        sensorData->windDirection = reinterpret_cast<const char*>(sqlite3_column_text( this->res, 6 ));
        sensorData->rainIntensity = reinterpret_cast<const char*>(sqlite3_column_text( this->res, 7 ));
        return true;
    }
} // namespace Database