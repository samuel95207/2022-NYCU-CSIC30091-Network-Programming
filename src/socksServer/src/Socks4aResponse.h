#include <map>
#include <string>
#include <vector>

using namespace std;

class Socks4aResponse {
   public:
    byte VN;
    byte CD;
    uint16_t dstPort;
    uint32_t dstIp;


   public:
    Socks4aResponse(bool success, uint16_t dstPort, uint32_t dstIp);
    Socks4aResponse(bool success);
    Socks4aResponse();
    void print();

    bool isAccept();
    string getDstIpString();
    string getAcceptString();

    void setAccept(bool success);
    void setDstPort(uint16_t port);
    void setDstIp(uint32_t ip);
    void setDstIpByString(string ip);

    void setHton();

};