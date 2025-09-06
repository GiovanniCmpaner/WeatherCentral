#include <Arduino.h>

#include <ArduinoJson.hpp>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <chrono>
#include <cstdlib>
#include <cstdint>
#include <sstream>
#include <esp_log.h>
#include <functional>
#include <memory>
#include <Update.h>
#include <esp_task_wdt.h>
#include <soc/rtc_wdt.h>
#include <rom/rtc.h>

#include "Configuration.hpp"
#include "Database.hpp"
#include "Peripherals.hpp"
#include "RealTime.hpp"
#include "WebInterface.hpp"
#include "Utils.hpp"
#include "Infos.hpp"

extern const uint8_t configuration_html_start[] asm( "_binary_html_configuration_html_start" );
extern const uint8_t configuration_js_start[] asm( "_binary_html_configuration_js_start" );
extern const uint8_t data_html_start[] asm( "_binary_html_data_html_start" );
extern const uint8_t data_js_start[] asm( "_binary_html_data_js_start" );
extern const uint8_t jquery_min_js_start[] asm( "_binary_html_jquery_min_js_start" );
extern const uint8_t infos_html_start[] asm( "_binary_html_infos_html_start" );
extern const uint8_t infos_js_start[] asm( "_binary_html_infos_js_start" );
extern const uint8_t style_css_start[] asm( "_binary_html_style_css_start" );

extern const uint8_t configuration_html_end[] asm( "_binary_html_configuration_html_end" );
extern const uint8_t configuration_js_end[] asm( "_binary_html_configuration_js_end" );
extern const uint8_t data_html_end[] asm( "_binary_html_data_html_end" );
extern const uint8_t data_js_end[] asm( "_binary_html_data_js_end" );
extern const uint8_t jquery_min_js_end[] asm( "_binary_html_jquery_min_js_end" );
extern const uint8_t infos_html_end[] asm( "_binary_html_infos_html_end" );
extern const uint8_t infos_js_end[] asm( "_binary_html_infos_js_end" );
extern const uint8_t style_css_end[] asm( "_binary_html_style_css_end" );

namespace WebInterface
{
    static std::unique_ptr<AsyncWebServer> server{};
    static std::chrono::system_clock::time_point modeTimer{};

    static auto buildFilter( AsyncWebServerRequest* request ) -> Database::Filter
    {
        auto id{int64_t{}};
        auto start{std::chrono::system_clock::time_point::min()};
        auto end{std::chrono::system_clock::time_point::max()};

        if ( request->hasParam( "id" ) )
        {
            id = request->getParam( "id" )->value().toInt();
        }
        if ( request->hasParam( "start" ) )
        {
            start = Utils::DateTime::fromString( request->getParam( "start" )->value().c_str() );
        }
        if ( request->hasParam( "end" ) )
        {
            end = Utils::DateTime::fromString( request->getParam( "end" )->value().c_str() );
        }

        return Database::Filter{id, start, end};
    }

    namespace Get
    {
        static auto handleProgmem( AsyncWebServerRequest* request, const std::string& contentType, const uint8_t* content, size_t len ) -> void
        {
            const auto lastModified{Utils::DateTime::compiled()};
            auto ifModifiedSince{std::chrono::system_clock::time_point::min()};

            if ( request->hasHeader( "If-Modified-Since" ) )
            {
                ifModifiedSince = Utils::DateTime::fromStringHttp( request->getHeader( "If-Modified-Since" )->value().c_str() );
            }

            auto response{lastModified <= ifModifiedSince ? request->beginResponse( 304 ) : request->beginResponse_P( 200, contentType.data(), content, len )};
            response->addHeader( "Last-Modified", Utils::DateTime::toStringHttp( lastModified ).data() );
            response->addHeader( "Date", Utils::DateTime::toStringHttp( std::chrono::system_clock::now() ).data() );
            response->addHeader( "Cache-Control", "public, max-age=0" );
            request->send( response );
        }

        static auto handleConfigurationJson( AsyncWebServerRequest* request ) -> void
        {
            auto response{new AsyncJsonResponse{false, 2048}};
            auto& responseJson{response->getRoot()};

            cfg.serialize( responseJson );

            response->setLength();
            request->send( response );
        }

        static auto handleDataJson( AsyncWebServerRequest* request ) -> void
        {
            auto response{new AsyncJsonResponse{true, 4096}};
            auto& responseJson{response->getRoot()};

            {
                auto filter{ WebInterface::buildFilter( request )};
                Database::SensorData sensorData;
                for ( auto n{0}; n < 20 and filter.next( &sensorData ); ++n )
                {
                    auto element{responseJson.add()};
                    sensorData.serialize( element );
                }
            }

            response->setLength();
            request->send( response );
        }

        static auto handleDateTimeJson( AsyncWebServerRequest* request ) -> void
        {
            auto response{new AsyncJsonResponse{}};
            auto& responseJson{response->getRoot()};

            responseJson.set( Utils::DateTime::toString( std::chrono::system_clock::now() ) );

            response->setLength();
            request->send( response );
        }

        static auto handleInfosJson( AsyncWebServerRequest* request ) -> void
        {
            auto response{new AsyncJsonResponse{}};
            auto& responseJson{response->getRoot()};

            Infos::serialize( responseJson );

            response->setLength();
            request->send( response );
        }

        static auto handleConfigurationHtml( AsyncWebServerRequest* request ) -> void
        {
            //handleProgmem( request, "text/html", configuration_html_start, static_cast<size_t>( configuration_html_end - configuration_html_start ) );
            request->send_P( 200, "text/html", configuration_html_start, static_cast<size_t>( configuration_html_end - configuration_html_start ) );
        }

        static auto handleConfigurationJs( AsyncWebServerRequest* request ) -> void
        {
            //handleProgmem( request, "application/javascript", configuration_js_start, static_cast<size_t>( configuration_js_end - configuration_js_start ) );
            request->send_P( 200, "application/javascript", configuration_js_start, static_cast<size_t>( configuration_js_end - configuration_js_start ) );
        }

        static auto handleDataHtml( AsyncWebServerRequest* request ) -> void
        {
            //handleProgmem( request, "text/html", data_html_start, static_cast<size_t>( data_html_end - data_html_start ) );
            request->send_P( 200, "text/html", data_html_start, static_cast<size_t>( data_html_end - data_html_start ) );
        }

        static auto handleDataJs( AsyncWebServerRequest* request ) -> void
        {
            //handleProgmem( request, "application/javascript", data_js_start, static_cast<size_t>( data_js_end - data_js_start ) );
            request->send_P( 200, "application/javascript", data_js_start, static_cast<size_t>( data_js_end - data_js_start ) );
        }

        class comma_punct : public std::numpunct<char>
        {
            protected:
                char do_decimal_point() const override
                {
                    return ',';   //comma
                }
        };
        std::locale loc{ std::locale{}, new comma_punct };

        static auto handleDataCsv( AsyncWebServerRequest* request ) -> void
        {
            auto stream{std::make_shared<std::stringstream>()};
            stream->imbue( loc );
            auto filter{std::make_shared<Database::Filter>( WebInterface::buildFilter( request ) )};
            auto response{request->beginChunkedResponse( "text/csv", [ = ]( uint8_t* buffer, size_t maxLen, size_t index ) -> size_t {
                    auto len{stream->readsome( reinterpret_cast<char*>( buffer ), maxLen )};
                    if ( len == 0 )
                    {
                        if( index == 0 )
                        {
                            ( *stream ) << "id" << ';'
                                        << "datetime" << ';'
                                        << "temperature" << ';'
                                        << "humidity" << ';'
                                        << "pressure" << ';'
                                        << "wind_speed" << ';'
                                        << "wind_direction" << ';'
                                        << "rain_intensity" << "\r\n";
                            len = stream->readsome( reinterpret_cast<char*>( buffer ), maxLen );
                        }
                        else
                        {
                            Database::SensorData sensorData;
                            if( filter->next( &sensorData ) )
                            {
                                ( *stream ) << sensorData.id << ';'
                                            << Utils::DateTime::toString( std::chrono::system_clock::from_time_t( sensorData.dateTime ) ) << ';'
                                            << sensorData.temperature << ';'
                                            << sensorData.humidity << ';'
                                            << sensorData.pressure << ';'
                                            << sensorData.windSpeed << ';'
                                            << sensorData.windDirection << ';'
                                            << sensorData.rainIntensity << "\r\n";
                                len = stream->readsome( reinterpret_cast<char*>( buffer ), maxLen );
                            }
                        }
                    }
                    return len;
                } )};
            response->addHeader( "Content-Disposition", "attachment;filename=data.csv" );
            request->send( response );
            log_d( "end" );
        }

        static auto handleJqueryJs( AsyncWebServerRequest* request ) -> void
        {
            //handleProgmem( request, "application/javascript", jquery_min_js_start, static_cast<size_t>( jquery_min_js_end - jquery_min_js_start ) );
            request->send_P( 200, "application/javascript", jquery_min_js_start, static_cast<size_t>( jquery_min_js_end - jquery_min_js_start ) );
        }

        static auto handleInfosHtml( AsyncWebServerRequest* request ) -> void
        {
            //handleProgmem( request, "text/html", infos_html_start, static_cast<size_t>( infos_html_end - infos_html_start ) );
            request->send_P( 200, "text/html", infos_html_start, static_cast<size_t>( infos_html_end - infos_html_start ) );
        }

        static auto handleInfosJs( AsyncWebServerRequest* request ) -> void
        {
            //handleProgmem( request, "application/javascript", infos_js_start, static_cast<size_t>( infos_js_end - infos_js_start ) );
            request->send_P( 200, "application/javascript", infos_js_start, static_cast<size_t>( infos_js_end - infos_js_start ) );
        }

        static auto handleStyleCss( AsyncWebServerRequest* request ) -> void
        {
            //handleProgmem( request, "text/css", style_css_start, static_cast<size_t>( style_css_end - style_css_start ) );
            request->send_P( 200, "text/css", style_css_start, static_cast<size_t>( style_css_end - style_css_start ) );
        }

    } // namespace Get

    namespace Post
    {
        static auto handleConfigurationJson( AsyncWebServerRequest* request, JsonVariant& requestJson ) -> void
        {
            auto response{new AsyncJsonResponse{}};
            auto& responseJson{response->getRoot()};

            auto newCfg{cfg};
            newCfg.deserialize( requestJson );
            Configuration::save( newCfg );

            responseJson.set( "Configuration saved, restarting in 3 seconds" );
            response->setLength();
            request->send( response );

            delay( 3000 );
            esp_restart();
        }

        static auto handleDateTimeJson( AsyncWebServerRequest* request, JsonVariant& requestJson ) -> void
        {
            auto response{new AsyncJsonResponse{}};
            auto& responseJson{response->getRoot()};

            const auto dateTime{Utils::DateTime::fromString( requestJson.as<std::string>() )};
            RealTime::adjustDateTime( dateTime );

            responseJson.set( "DateTime saved, restarting in 3 seconds" );
            response->setLength();
            request->send( response );

            delay( 3000 );
            ESP.restart();
        }

        static auto handleUpdate( AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final ) -> void
        {
            if( request->url() != "/update" )
            {
                request->send( 404 );
                return;
            }
            if( filename != "firmware.bin" )
            {
                request->send( 400, "text/plain", "Invalid filename" );
                return;
            }
            if( index == 0 )
            {
                Serial.printf( "UploadStart: %s\n", filename.c_str() );
                if( not Update.begin( request->contentLength() ) )
                {
                    request->send( 500, "text/plain", Update.errorString() );
                    return;
                }

            }
            if ( Update.write( data, len ) != len )
            {
                request->send( 500, "text/plain", Update.errorString() );
                return;
            }
            if( final )
            {
                if( not Update.end( true ) )
                {
                    request->send( 500, "text/plain", Update.errorString() );
                    return;
                }
                request->send( 200, "text/plain", "Success, rebooting in 3 seconds" );
                delay( 3000 );
                ESP.restart();
            }
        }
    } // namespace Post

    static auto configureServer() -> void
    {
        server.release();

        if ( WiFi.getMode() == WIFI_MODE_STA )
        {
            server.reset( new AsyncWebServer{cfg.station.port} );
        }
        else if ( WiFi.getMode() == WIFI_MODE_AP )
        {
            server.reset( new AsyncWebServer{cfg.accessPoint.port} );
        }

        if ( server )
        {
            server->on( "/configuration.json", HTTP_GET, Get::handleConfigurationJson );
            server->on( "/datetime.json", HTTP_GET, Get::handleDateTimeJson );
            server->on( "/data.json", HTTP_GET, Get::handleDataJson );
            server->on( "/infos.json", HTTP_GET, Get::handleInfosJson );
            server->on( "/configuration.html", HTTP_GET, Get::handleConfigurationHtml );
            server->on( "/configuration.js", HTTP_GET, Get::handleConfigurationJs );
            server->on( "/data.html", HTTP_GET, Get::handleDataHtml );
            server->on( "/data.js", HTTP_GET, Get::handleDataJs );
            server->on( "/data.csv", HTTP_GET, Get::handleDataCsv );
            server->on( "/jquery.min.js", HTTP_GET, Get::handleJqueryJs );
            server->on( "/infos.html", HTTP_GET, Get::handleInfosHtml );
            server->on( "/infos.js", HTTP_GET, Get::handleInfosJs );
            server->on( "/style.css", HTTP_GET, Get::handleStyleCss );

            server->addHandler( new AsyncCallbackJsonWebHandler( "/configuration.json", Post::handleConfigurationJson, 2048 ) );
            server->addHandler( new AsyncCallbackJsonWebHandler( "/datetime.json", Post::handleDateTimeJson, 1024 ) );
            server->onFileUpload( Post::handleUpdate );

            DefaultHeaders::Instance().addHeader( "Access-Control-Allow-Origin", "*" );
            DefaultHeaders::Instance().addHeader( "Access-Control-Allow-Methods", "POST, GET, OPTIONS" );
            DefaultHeaders::Instance().addHeader( "Access-Control-Allow-Headers", "Content-Type" );
            DefaultHeaders::Instance().addHeader( "Access-Control-Max-Age", "86400" );
            server->onNotFound( []( AsyncWebServerRequest * request )
            {
                if ( request->method() == HTTP_OPTIONS )
                {
                    request->send( 200 );
                }
                else
                {
                    request->send( 404 );
                }
            } );
            server->begin();
        }
    }

    static auto configureStation() -> bool
    {
        log_d( "begin" );

        log_d( "enabled = %u", cfg.station.enabled );
        log_d( "mac = %02X-%02X-%02X-%02X-%02X-%02X", cfg.station.mac[0], cfg.station.mac[1], cfg.station.mac[2], cfg.station.mac[3], cfg.station.mac[4], cfg.station.mac[5] );
        log_d( "ip = %u.%u.%u.%u", cfg.station.ip[0], cfg.station.ip[1], cfg.station.ip[2], cfg.station.ip[3] );
        log_d( "netmask = %u.%u.%u.%u", cfg.station.netmask[0], cfg.station.netmask[1], cfg.station.netmask[2], cfg.station.netmask[3] );
        log_d( "gateway = %u.%u.%u.%u", cfg.station.gateway[0], cfg.station.gateway[1], cfg.station.gateway[2], cfg.station.gateway[3] );
        log_d( "port = %u", cfg.station.port );
        log_d( "user = %s", cfg.station.user.data() );
        log_d( "password = %s", cfg.station.password.data() );

        if ( not cfg.station.enabled )
        {
            WiFi.mode( WIFI_MODE_NULL );
            return false;
        }

        if ( not WiFi.mode( WIFI_MODE_STA ) )
        {
            log_d( "mode error" );
            return false;
        }

        WiFi.persistent( false );
        WiFi.setAutoConnect( false );
        WiFi.setAutoReconnect( true );

        if ( not WiFi.config( cfg.station.ip.data(), cfg.station.gateway.data(), cfg.station.netmask.data() ) )
        {
            log_d( "config error" );
            return false;
        }

        WiFi.setHostname( "WeatherCentral" );

        if ( not WiFi.begin( cfg.station.user.data(), cfg.station.password.data() ) )
        {
            log_d( "init error" );
            return false;
        }

        configureServer();
        return true;
    }

    static auto configureAccessPoint() -> bool
    {
        log_d( "begin" );

        log_d( "enabled = %u", cfg.accessPoint.enabled );
        log_d( "mac = %02X-%02X-%02X-%02X-%02X-%02X", cfg.accessPoint.mac[0], cfg.accessPoint.mac[1], cfg.accessPoint.mac[2], cfg.accessPoint.mac[3], cfg.accessPoint.mac[4], cfg.accessPoint.mac[5] );
        log_d( "ip = %u.%u.%u.%u", cfg.accessPoint.ip[0], cfg.accessPoint.ip[1], cfg.accessPoint.ip[2], cfg.accessPoint.ip[3] );
        log_d( "netmask = %u.%u.%u.%u", cfg.accessPoint.netmask[0], cfg.accessPoint.netmask[1], cfg.accessPoint.netmask[2], cfg.accessPoint.netmask[3] );
        log_d( "gateway = %u.%u.%u.%u", cfg.accessPoint.gateway[0], cfg.accessPoint.gateway[1], cfg.accessPoint.gateway[2], cfg.accessPoint.gateway[3] );
        log_d( "port = %u", cfg.accessPoint.port );
        log_d( "user = %s", cfg.accessPoint.user.data() );
        log_d( "password = %s", cfg.accessPoint.password.data() );
        log_d( "duration = %u", cfg.accessPoint.duration );

        if ( not cfg.accessPoint.enabled or rtc_get_reset_reason( 0 ) == DEEPSLEEP_RESET )
        {
            WiFi.mode( WIFI_MODE_NULL );
            return false;
        }

        if ( not WiFi.mode( WIFI_MODE_AP ) )
        {
            log_d( "mode error" );
            return false;
        }

        WiFi.persistent( false );

        if ( not WiFi.softAPConfig( cfg.accessPoint.ip.data(), cfg.accessPoint.gateway.data(), cfg.accessPoint.netmask.data() ) )
        {
            log_d( "config error" );
            return false;
        }

        WiFi.setHostname( "WeatherCentral" );

        if ( not WiFi.softAP( cfg.accessPoint.user.data(), cfg.accessPoint.password.data() ) )
        {
            log_d( "init error" );
            return false;
        }

        configureServer();
        return true;
    }

    auto init() -> void
    {
        log_d( "begin" );

        if( not configureAccessPoint() )
        {
            configureStation();
        }

        modeTimer = std::chrono::system_clock::now();

        log_d( "end" );
    }

    auto process() -> void
    {
        if( cfg.accessPoint.enabled and WiFi.getMode() == WIFI_MODE_AP )
        {
            const auto now{std::chrono::system_clock::now()};
            if( WiFi.softAPgetStationNum() > 0 )
            {
                modeTimer = now;
            }
            else
            {
                if( now - modeTimer > std::chrono::seconds( cfg.accessPoint.duration ) )
                {
                    configureStation();
                }
            }
        }
    }
} // namespace WebInterface