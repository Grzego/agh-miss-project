#pragma once

#include <unordered_map>
#include <unordered_set>
#include "utils.h"

/**
 * Struktura odpowiedzialna za przechowywanie wiedzy agenta/ow
 */
struct Knowledge
{
    std::unordered_map<Vec2, int>    time_stamp;
    std::unordered_map<Vec2, double> values;
    std::unordered_set<Vec2>         positive;
    std::unordered_set<Vec2>         negative;
    std::unordered_set<Vec2>         blocked;
};