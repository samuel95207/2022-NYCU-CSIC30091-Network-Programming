#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <utility>

using boost::asio::ip::tcp;


class HttpServer {
    int port;
    tcp::acceptor acceptor;

   public:
    HttpServer(boost::asio::io_context& io_context, int port);

   private:
    void doAccept();
};