#include <Arduino.h>

#include <chrono>
#include <optional>

#include "Peripherals.hpp"
#include "Indicator.hpp"

namespace Indicator
{
    static auto _temporizador = std::chrono::system_clock::now();
    static auto _ligado = false;
    static auto _rapido = true;

    auto init() -> void
    {
        digitalWrite(Peripherals::LED_HTB, LOW);
    }

    auto process() -> void
    {
        const auto agora = std::chrono::system_clock::now();
        if(agora - _temporizador >= std::chrono::milliseconds{_ligado ? 100 : _rapido ? 300 : 3000})
        {
            _temporizador = agora;
            _ligado = not _ligado;
            digitalWrite(Peripherals::LED_HTB, _ligado);
        }
    }

    auto slow() -> void
    {
        _rapido = false;
    }

    auto fast() -> void
    {
        _rapido = true;
    }
}