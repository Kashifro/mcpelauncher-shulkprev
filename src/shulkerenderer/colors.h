#pragma once
#include <cstdint>
#include "ui/minecraftuirendercontext.h"

struct ShulkerIdMap {
    uint16_t id;
    char code;
};

static inline char getShulkerColorCodeFromItemId(uint16_t id)
{
    static constexpr ShulkerIdMap table[] = {
        {205,   '0'}, // undyed
        {218,   '1'}, // white

        {64923, '7'}, // orange
        {64922, 'f'}, // magenta
        {64921, 'c'}, // light blue
        {64920, '8'}, // yellow
        {64919, '9'}, // lime
        {64918, 'g'}, // pink
        {64917, '3'}, // gray
        {64916, '2'}, // light gray
        {64915, 'b'}, // cyan
        {64914, 'e'}, // purple
        {64913, 'd'}, // blue
        {64912, '5'}, // brown
        {64911, 'a'}, // green
        {64910, '6'}, // red
        {64909, '4'}  // black
    };

    for (const auto& e : table)
        if (e.id == id)
            return e.code;

    return '0';
}

static inline mce::Color getShulkerTint(char code)
{
    switch (code)
    {
        case '0': return {0.45f,0.42f,0.40f,1.0f}; // undyed
        case '1': return {0.78f,0.76f,0.74f,1.0f}; // white
        case '2': return {0.55f,0.53f,0.52f,1.0f}; // light gray
        case '3': return {0.32f,0.31f,0.30f,1.0f}; // gray
        case '4': return {0.06f,0.05f,0.05f,1.0f}; // black
        case '5': return {0.33f,0.25f,0.14f,1.0f}; // brown
        case '6': return {0.55f,0.20f,0.18f,1.0f}; // red
        case '7': return {0.70f,0.42f,0.18f,1.0f}; // orange
        case '8': return {0.78f,0.72f,0.22f,1.0f}; // yellow
        case '9': return {0.42f,0.65f,0.22f,1.0f}; // lime
        case 'a': return {0.18f,0.40f,0.18f,1.0f}; // green
        case 'b': return {0.18f,0.55f,0.55f,1.0f}; // cyan
        case 'c': return {0.28f,0.46f,0.62f,1.0f}; // light blue
        case 'd': return {0.18f,0.24f,0.58f,1.0f}; // blue
        case 'e': return {0.45f,0.26f,0.60f,1.0f}; // purple
        case 'f': return {0.65f,0.34f,0.58f,1.0f}; // magenta
        case 'g': return {0.78f,0.52f,0.62f,1.0f}; // pink
    }

    return {0.55f,0.55f,0.55f,1.0f};
}