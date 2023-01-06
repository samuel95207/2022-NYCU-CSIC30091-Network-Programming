#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <utility>


#ifndef _SOCKS_SERVER_H_
#define _SOCKS_SERVER_H_
#include "SocksServer.h"
#endif

using namespace std;

int main(int argc, char* argv[]) {
    try {
        int port = stoi(string(argv[1]));

        boost::asio::io_context io_context;
        SocksServer server(io_context, port);
        io_context.run();

    } catch (std::exception& e) {
        cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}