#include <iostream>
#include <vector>

#ifdef _WIN32
#define _WIN32_WINRT 0x0a00
#endif

#define ASIO_STANDALONE // No boost plz
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

#define USE_LOGGER_MACROS
#include "Logger.h"

#include "Server.h"

int main()
{
    std::cout << "hello world\n";

    auto& logger = Logger::Get();
    logger.AddOutput(LogLevel::Debug | LogLevel::Info, Logger::StandardOutput);
    logger.AddOutput(LogLevel::Warning | LogLevel::Error, Logger::StandardErrorOutput);

    asio::error_code error;
    asio::io_context context;

    asio::ip::tcp::endpoint endpoint(asio::ip::make_address("172.217.1.3", error), 80);

    asio::ip::tcp::socket socket(context);

    if (!error) INFO("connected"); else ERROR("failed to connect to address");

    Message count(123);
    constexpr int SZ = 64;
    uint8_t arr[SZ];

    for (int i = 0; i < SZ; ++i)
    {
        arr[i] = i;
    }

    count.Push(arr, SZ);

    std::cout << count << '\n';

    Message msg(10);

    int x = 0xabcdef01;
    int y = 0x12345678;
    const char* message = "hello";

    msg.Push(&x, sizeof(int));
    msg.Push(&y, sizeof(int));
    msg.Push((void*)message, (strlen(message) + 1) * sizeof(char));

    std::cout << msg << '\n';

    char text[6] = "no";
    if (!msg.Pop(&text, 120, 6)) ERROR_FL("fail");

    std::cerr << std::endl;

    std::cout << msg << '\n';

    unsigned int poppedY = 0;
    if (!msg.Pop(&poppedY, sizeof(int), sizeof(int))) ERROR_FL("fail");

    std::cout << msg << '\n';

    unsigned int poppedX = 0;
    if (!msg.Pop(&poppedX, sizeof(int), sizeof(int))) ERROR_FL("fail");
    if (!msg.Pop(&poppedX, sizeof(int), sizeof(int))) ERROR_FL("fail"); // Intentional fail


    std::cout << msg << '\n';

    std::cout << "final: " << poppedX << ' ' << poppedY << ' ' << text << '\n';

    //std::cout << "final: " << poppedX << ' ' << poppedY << ' ' << msg << std::endl;
    return 0;
}