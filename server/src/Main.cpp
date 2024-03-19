#include <iostream>

//#ifdef _WIN32
//#define _WIN32_WINRT 0x0a00
//#endif
#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

int main()
{
    std::cout << "hello world\n";

    asio::error_code error;
    asio::io_context context;

    asio::ip::tcp::endpoint endpoint(asio::ip::make_address("172.217.1.3", error), 80);

    asio::ip::tcp::socket socket(context);

    if (!error)
    {
        std::cout << "connected\n";
        return 0;
    }

    std::cout << "failed to connect to address\n";
    return -1;
}