#include <boost/asio.hpp>
#include <map>
#include <string>
#include <vector>

using namespace std;

class Socks4aRequest {
   public:
    byte VN;
    byte CD;
    uint16_t dstPort;
    uint32_t dstIp;
    string userId;
    string domainName;


   public:
    static Socks4aRequest parse(char* rawRequest);
    void print() const;

    bool needDomainResolve() const;
    string getDstIpString() const;
    string getCommandString() const;

    void resolveDomainName(boost::asio::ip::tcp::resolver& resolver);

    int size();
    char* byteArr();
};