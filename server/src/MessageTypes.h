#pragma once

#include <cstdint>
#include <string>

namespace MessageTypes
{
    enum class PacketType: uint8_t
    {
        Null = 0, // Only to be used server side
        SetClientId,
        SetRoomId,
        JoinRoom,
        CreateRoom,
        DrawCommand,
        CanvasPacket,
        GuessPacket,
        CorrectGuess,
        SetDrawer
    };

    struct Header
    {
        Header(PacketType type, uint32_t size)
            : room(0xffff), client_id(0xff), packet_type(type), payload_size(size)
        {
        }

        uint16_t room; // Only for incoming messages
        uint8_t client_id; // Only for incoming messages
        PacketType packet_type;
        uint32_t payload_size;
    };
    static_assert(sizeof(Header) == 8, "Header allignment bad");

    struct DrawCommand
    {
        uint16_t old_x;
        uint16_t old_y;
        uint16_t new_x;
        uint16_t new_y;
        uint16_t color;
    };
    static_assert(sizeof(DrawCommand) == 10, "Draw command allignment bad");

    struct CanvasPacket
    {
        std::vector<DrawCommand> commands;
    };

    struct GuessPacket
    {
        std::string guess;
    };

    struct DrawerPacket
    {
        uint8_t isDrawer;
        std::string word;
    };
};
