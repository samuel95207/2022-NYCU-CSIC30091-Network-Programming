#include <map>
#include <string>
#include <vector>

using namespace std;

class HttpRequest {
   public:
    string version;
    string method;
    string host;
    string addr;
    int port;
    string uri;
    string uriOnly;
    string queryStr;

    vector<string> parsedUri;
    map<string, string> queryMap;
    map<string, string> headerMap;

   public:
    static HttpRequest parse(string rawRequest);
    void print();
};