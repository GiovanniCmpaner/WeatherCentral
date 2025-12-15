#include <Arduino.h>

#include <ArduinoJson.hpp>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <chrono>
#include <cstdlib>
#include <cstdint>
#include <sstream>
#include <esp_log.h>
#include <functional>
#include <memory>
#include <Update.h>
#include <esp_task_wdt.h>
#include <rom/rtc.h>
#include <future>

#include "Configuration.hpp"
#include "Database.hpp"
#include "Peripherals.hpp"
#include "RealTime.hpp"
#include "WebInterface.hpp"
#include "Utils.hpp"
#include "Infos.hpp"
#include "Files.hpp"
#include "Indicator.hpp"

namespace WebInterface
{
    static std::unique_ptr<AsyncWebServer> _server = {};
    static std::chrono::system_clock::time_point _modeTimer = {};
    static std::chrono::system_clock::time_point _reconnectTimer = {};
    static std::chrono::system_clock::time_point _sensorsSendTimer = {};
    static std::chrono::system_clock::time_point _wsCleanupTimer = {};
    static std::future<void> _futuroReinicio = {};

    static AsyncWebSocket _sensorsWs("/sensors.ws");

    static auto reinicia() -> void {
        _futuroReinicio = std::async(std::launch::async, []
        {
            std::this_thread::sleep_for(std::chrono::seconds(3));
            esp_restart();
        });
    }

    namespace Get
    {
        static auto handleConfigurationJson( AsyncWebServerRequest* request ) -> void
        {
            auto response{new AsyncJsonResponse{false, 2048}};
            auto& responseJson{response->getRoot()};

            cfg.serialize( responseJson );

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

        static auto handleConfigurationHtml( AsyncWebServerRequest* request ) -> void
        {
            request->send_P( 200, "text/html", configuration_html_start, static_cast<size_t>( configuration_html_end - configuration_html_start ) );
        }

        static auto handleConfigurationJs( AsyncWebServerRequest* request ) -> void
        {
            request->send_P( 200, "application/javascript", configuration_js_start, static_cast<size_t>( configuration_js_end - configuration_js_start ) );
        }

        static auto handleDataHtml( AsyncWebServerRequest* request ) -> void
        {
            request->send_P( 200, "text/html", data_html_start, static_cast<size_t>( data_html_end - data_html_start ) );
        }

        static auto handleDataJs( AsyncWebServerRequest* request ) -> void
        {
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
            const auto start = Utils::DateTime::fromString( request->getParam( "start" )->value().c_str() );
            const auto end = Utils::DateTime::fromString( request->getParam( "end" )->value().c_str() );

            auto filter = std::make_shared<Database::Filter>(start, end, 10000);

            auto response = request->beginChunkedResponse( "text/csv", [=]( uint8_t* buffer, size_t maxLen, size_t index ) -> size_t 
            {
                auto len = 0u;
                auto rowBuf = std::array<char, 100u>{};

                if(index == 0 and len == 0)
                {
                    memcpy(buffer, "datahora;temp;umid;pressao;vento;direcao;chuva\r\n", 48);
                    len += 48;
                }

                while(len + rowBuf.size() <= maxLen)
                {
                    const auto sensorData = filter->next();
                    if(not sensorData.has_value()){
                        return len;
                    }

                    const auto written = sensorData->serialize(rowBuf);

                    memcpy(buffer + len, rowBuf.data(), written);
                    len += written;
                }

                return (len == 0 ? RESPONSE_TRY_AGAIN : len);
            });

            response->addHeader( "Content-Disposition", "attachment;filename=data.csv" );
            request->send( response );
        }

        static auto handleJqueryMinJs( AsyncWebServerRequest* request ) -> void
        {
            auto response = request->beginResponse_P(200, "application/javascript", jquery_min_js_gz_start, static_cast<size_t>(jquery_min_js_gz_end - jquery_min_js_gz_start));
            response->addHeader("Content-Encoding", "gzip");
            request->send(response);
        }

        static auto handleChartMinJs(AsyncWebServerRequest *request) -> void
        {
            auto response{request->beginResponse_P(200, "application/javascript", chart_min_js_gz_start, static_cast<size_t>(chart_min_js_gz_end - chart_min_js_gz_start))};
            response->addHeader("Content-Encoding", "gzip");
            request->send(response);
        }

        static auto handleInfosHtml( AsyncWebServerRequest* request ) -> void
        {
            request->send_P( 200, "text/html", infos_html_start, static_cast<size_t>( infos_html_end - infos_html_start ) );
        }

        static auto handleInfosJs( AsyncWebServerRequest* request ) -> void
        {
            request->send_P( 200, "application/javascript", infos_js_start, static_cast<size_t>( infos_js_end - infos_js_start ) );
        }

        static auto handleStyleCss( AsyncWebServerRequest* request ) -> void
        {
            request->send_P( 200, "text/css", style_css_start, static_cast<size_t>( style_css_end - style_css_start ) );
        }

    } // namespace Get

    namespace File 
    {
        auto handleFirmwareBin(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) -> void
        {
            if (index == 0)
            {
                log_d("POST /firmware.bin");

                if (not filename.endsWith(".bin"))
                {
                    request->send(400, "text/plain", "File extension must be .bin");
                    return;
                }

                if (request->contentLength() > 1945600)
                {
                    request->send(400, "text/plain", "File size must be 1945600 bytes or less");
                    return;
                }

                if (Update.size() > 0)
                {
                    request->send(500, "text/plain", "Already uploading");
                    return;
                }

                if (not Update.begin(request->contentLength()))
                {
                    request->send(500, "text/plain", Update.errorString());
                    return;
                }
            }

            if (Update.write(data, len) != len)
            {
                request->send(500, "text/plain", Update.errorString());
                return;
            }

            if (final)
            {
                if (not Update.end(true))
                {
                    request->send(500, "text/plain", Update.errorString());
                    return;
                }
                request->send(200, "text/plain", "Success, rebooting");
            }
        }

        auto handleConfigurationJson(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) -> void
        {
            static auto file = fs::File{};

            if (index == 0)
            {
                log_d("POST /configuration.json");

                if (not filename.endsWith(".json"))
                {
                    request->send(400, "text/plain", "File extension must be .tflite");
                    return;
                }

                if (request->contentLength() > 4096)
                {
                    request->send(400, "text/plain", "File size must be 4096 bytes or less");
                    return;
                }

                if (file.size() > 0)
                {
                    request->send(500, "text/plain", "Already uploading");
                    return;
                }

                file = LittleFS.open("/configuration.json", FILE_WRITE);
                if (not file)
                {
                    request->send(500, "text/plain", "Error opening file");
                    return;
                }
            }

            if (file.write(data, len) != len)
            {
                request->send(500, "text/plain", "Error writing to file, probably there's no space left");

                file.close();
                LittleFS.remove("/configuration.json");

                return;
            }

            if (final)
            {
                file.close();

                request->send(200, "text/plain", "Success, rebooting");
                
                WebInterface::reinicia();
            }
        }
    }

    namespace Post
    {
        static auto handleFirmwareBin(AsyncWebServerRequest *request) -> void 
        {
            request->send(200, "text/plain", "Success, rebooting");

            WebInterface::reinicia();
        }

        static auto handleConfigurationJson( AsyncWebServerRequest* request, JsonVariant& requestJson ) -> void
        {
            log_d("POST /configuration.json");

            auto response{new AsyncJsonResponse{}};
            auto& responseJson{response->getRoot()};

            auto newCfg{cfg};
            newCfg.deserialize( requestJson );
            Configuration::save( newCfg );

            responseJson.set( "Configuration saved, rebooting" );
            response->setLength();
            request->send( response );

            WebInterface::reinicia();
        }

        static auto handleDateTimeJson( AsyncWebServerRequest* request, JsonVariant& requestJson ) -> void
        {
            log_d("POST /datetime.json");

            auto response{new AsyncJsonResponse{}};
            auto& responseJson{response->getRoot()};

            const auto dateTime{Utils::DateTime::fromString( requestJson.as<std::string>() )};
            RealTime::adjustDateTime( dateTime );

            responseJson.set( "DateTime saved, rebooting" );
            response->setLength();
            request->send( response );

            WebInterface::reinicia();
        }

        auto handleFile(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) -> void
        {
            if (request->url() == "/firmware.bin")
            {
                File::handleFirmwareBin(request, filename, index, data, len, final);
            }
            else if (request->url() == "/configuration.json")
            {
                File::handleConfigurationJson(request, filename, index, data, len, final);
            }
            else
            {
                request->send(404, "text/plain", "file not found");
            }
        }
    } // namespace Post

    namespace WebSocket 
    {
        auto handleDefaultWs(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) -> void
        {
            if (type == WS_EVT_CONNECT)
            {
                log_d("WS %s (%u) connect", server->url(), client->id());
                client->ping();
            }
            else if (type == WS_EVT_DISCONNECT)
            {
                log_d("WS %s (%u) disconnect", server->url(), client->id());
            }
            else if (type == WS_EVT_ERROR)
            {
                log_e("WS %s (%u) error (%u): %s", server->url(), client->id(), *reinterpret_cast<const uint16_t *>(arg), reinterpret_cast<const char *>(data));
            }
            else if (type == WS_EVT_PONG)
            {
                log_d("WS %s (%u) pong", server->url(), client->id());
            }
            else if (type == WS_EVT_DATA)
            {
                // NOTHING
            }
        }
    }

    static auto configureServer() -> void
    {
        _server.release();

        if ( WiFi.getMode() == WIFI_MODE_STA )
        {
            _server.reset( new AsyncWebServer{cfg.station.port} );
        }
        else if ( WiFi.getMode() == WIFI_MODE_AP )
        {
            _server.reset( new AsyncWebServer{cfg.accessPoint.port} );
        }

        if ( _server )
        {
            _server->on( "/", HTTP_GET, Get::handleInfosHtml );
            _server->on( "/configuration.json", HTTP_GET, Get::handleConfigurationJson );
            _server->on( "/datetime.json", HTTP_GET, Get::handleDateTimeJson );
            _server->on( "/configuration.html", HTTP_GET, Get::handleConfigurationHtml );
            _server->on( "/configuration.js", HTTP_GET, Get::handleConfigurationJs );
            _server->on( "/data.html", HTTP_GET, Get::handleDataHtml );
            _server->on( "/data.js", HTTP_GET, Get::handleDataJs );
            _server->on( "/data.csv", HTTP_GET, Get::handleDataCsv );
            _server->on( "/jquery.min.js", HTTP_GET, Get::handleJqueryMinJs);
            _server->on( "/chart.min.js", HTTP_GET, Get::handleChartMinJs);
            _server->on( "/infos.html", HTTP_GET, Get::handleInfosHtml );
            _server->on( "/infos.js", HTTP_GET, Get::handleInfosJs );
            _server->on( "/style.css", HTTP_GET, Get::handleStyleCss );

            _server->on( "/firmware.bin", HTTP_POST, Post::handleFirmwareBin, File::handleFirmwareBin );
            _server->on( "/configuration.json", HTTP_POST, Post::handleConfigurationJson );
            _server->on( "/datetime.json", HTTP_POST, Post::handleDateTimeJson );

            _sensorsWs.onEvent(WebSocket::handleDefaultWs);
            _server->addHandler(&_sensorsWs);

            DefaultHeaders::Instance().addHeader( "Access-Control-Allow-Origin", "*" );
            DefaultHeaders::Instance().addHeader( "Access-Control-Allow-Methods", "POST, GET, OPTIONS" );
            DefaultHeaders::Instance().addHeader( "Access-Control-Allow-Headers", "Content-Type" );
            DefaultHeaders::Instance().addHeader( "Access-Control-Max-Age", "86400" );
            _server->onNotFound( []( AsyncWebServerRequest * request )
            {
                if ( request->method() == HTTP_OPTIONS )
                {
                    request->send( 200 );
                }
                else
                {
                    request->send( 404, "text/plain", "page not found" );
                }
            } );
            _server->begin();
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
        WiFi.setAutoReconnect( false );

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
        Indicator::fast();
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
        Indicator::fast();
        return true;
    }

    static auto cleanupWebSockets() -> void
    {
        const auto now = std::chrono::system_clock::now();

        if (now - _wsCleanupTimer >= std::chrono::milliseconds{1000})
        {
            _wsCleanupTimer = now;

            _sensorsWs.cleanupClients(1);
        }
    }

    static auto sendSensors() -> void
    {
        const auto now = std::chrono::system_clock::now();

        if (now - _sensorsSendTimer >= std::chrono::milliseconds{1000})
        {
            _sensorsSendTimer = now;

            if (_sensorsWs.count() > 0)
            {
                auto doc{ArduinoJson::DynamicJsonDocument{1024}};
                auto json{doc.as<ArduinoJson::JsonVariant>()};

                const auto sensorData = Infos::SensorData::get();
                sensorData.serialize(json);

                auto str = String{};
                ArduinoJson::serializeJson(doc, str);
                _sensorsWs.textAll(str);
            }
        }
    }

    static auto checkModeChange() -> void 
    {
        if(not cfg.accessPoint.enabled or WiFi.getMode() != WIFI_MODE_AP)
        {
            return;
        }

        const auto now = std::chrono::system_clock::now();

        if( WiFi.softAPgetStationNum() > 0 )
        {
            Indicator::slow();
            _modeTimer = now;
        }
        else 
        {
            Indicator::fast();

            if( now - _modeTimer > std::chrono::seconds( cfg.accessPoint.duration ) )
            {
                _modeTimer = now;
                configureStation();
            }
        }
    }

    static auto checkReconnect() -> void 
    {
        if(not cfg.station.enabled or WiFi.getMode() != WIFI_MODE_STA)
        {
            return;
        }

        const auto now = std::chrono::system_clock::now();

        if(WiFi.isConnected())
        {
            Indicator::slow();
            _reconnectTimer = now;
        }
        else 
        {
            Indicator::fast();

            if( now - _reconnectTimer > std::chrono::seconds{15} )
            {
                _reconnectTimer = now;
                WiFi.begin();
            }
        }
    }

    auto init() -> void
    {
        log_d( "begin" );

        if( not configureAccessPoint() )
        {
            configureStation();
        }

        _modeTimer = std::chrono::system_clock::now();

        log_d( "end" );
    }

    auto process() -> void
    {
        WebInterface::checkModeChange();
        WebInterface::checkReconnect();
        WebInterface::cleanupWebSockets();
        WebInterface::sendSensors();
    }
} // namespace WebInterface