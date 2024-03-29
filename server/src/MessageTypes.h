#pragma once

#include <cstdint>

namespace MessageTypes
{
    struct Header
    {
        uint16_t server_id;
        uint8_t client_id;
        uint8_t packet_type;
    };

    struct DrawCommand
    {
        uint16_t old_x;
        uint16_t old_y;
        uint16_t new_x;
        uint16_t new_y;
        uint16_t color;
    };

    struct CanvasPacket
    {
        uint16_t commands_size;
        DrawCommand *commands;
    };

    struct GuessPacket
    {
        uint16_t guess_length;
        char *guess;
    };
};
