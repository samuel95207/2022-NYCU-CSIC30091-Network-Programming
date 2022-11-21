#include <boost/asio.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <utility>


#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_
#include "HttpRequest.h"
#endif


using namespace std;
using boost::asio::ip::tcp;

class Console;

enum class OutputType { COMMAND, RESPONSE, ERRORMSG };

struct Output {
    string value;
    OutputType type;
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

    vector<Output> commandResponseArr;
    Output currentOutput;

    char data[BUF_SIZE];


    bool exit = false;



   public:
    ConsoleSession(boost::asio::io_service& io_service, Console* console, int id, string host, int port,
                   string filename, HttpRequest request);

    void start();
    bool isExit();


    string getHost();
    int getPort();
    vector<Output> getOutputArr();




   private:
    void onResolve(const boost::system::error_code& errorCode, tcp::resolver::iterator iterator);
    void onConnect(const boost::system::error_code& errorCode, tcp::resolver::iterator iterator);
    void doRead();
    void doWrite();

    void addOutput(string output, OutputType type);
    void exitSession();
};