#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <utility>

using boost::asio::ip::tcp;


class SocksServer {
    int port;
    boost::asio::io_context io_context;
    tcp::acceptor acceptor;

   public:
    SocksServer(boost::asio::io_context& io_context, int port);

   private:
    void doAccept(boost::asio::io_context& io_context);
    static void childSignalHandler(int signum);
};