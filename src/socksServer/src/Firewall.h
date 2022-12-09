#include <regex>
#include <string>
#include <vector>

#ifndef _SOCKS_REQUEST_H_
#define _SOCKS_REQUEST_H_
#include "Socks4aRequest.h"
#endif



using namespace std;


class Firewall {
    string configFilename;

    vector<string> connectIpFilters;
    vector<string> bindIpFilters;


   public:
    Firewall();
    Firewall(string configFilename);

    bool isPermit(const Socks4aRequest& request);

   private:
    bool loadConfigFile();
};