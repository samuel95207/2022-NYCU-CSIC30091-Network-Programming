#include <boost/asio.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <utility>


#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_
#include "../HttpRequest.h"
#endif


using namespace std;
using boost::asio::ip::tcp;

class Console;

enum class CommandResponseType { COMMAND, RESPONSE };

struct CommandResponse {
    string value;
    CommandResponseType type;
};

class ConsoleSession : public enable_shared_from_this<ConsoleSession> {
    static const int BUF_SIZE = 1048576;
    static const string scriptPath;

    tcp::socket socket;
    tcp::resolver resolver;

    Console* console;

    int id;
    string host;
    int port;
    string filename;
    HttpRequest request;

    fstream scriptFile;

    vector<CommandResponse> commandResponseArr;
    CommandResponse currentCommandResponse;

    char data[BUF_SIZE];


    bool exit = false;



   public:
    ConsoleSession(boost::asio::io_service& io_service, Console* console);

    void start(int idIn, string hostIn, int portIn, string filenameIn, HttpRequest requestIn);
    bool isExit();


    string getHost();
    int getPort();
    vector<CommandResponse> getCommandResponseArr();




   private:
    void onResolve(const boost::system::error_code& errorCode, tcp::resolver::iterator iterator);
    void onConnect(const boost::system::error_code& errorCode, tcp::resolver::iterator iterator);
    void doRead();
    void doWrite();

    void recvRequest(string rawRequest);
};