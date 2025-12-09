#pragma once

#include <Arduino.h>

namespace Indicator
{
    auto init() -> void;
    auto process() -> void;
    auto slow() -> void;
    auto fast() -> void;
}