#include <fstream>
#include <iostream>
#include <sstream>
#include <string>


#ifndef _FIREWALL_H_
#define _FIREWALL_H_
#include "Firewall.h"
#endif

using namespace std;

Firewall::Firewall() {}
Firewall::Firewall(string configFilename) : configFilename(configFilename) {}

bool Firewall::isPermit(const Socks4aRequest& request) {
    if (!loadConfigFile()) {
        return false;
    }

    if (request.getCommandString() == "CONNECT") {
        for (auto connectIpFilter : connectIpFilters) {
            regex reg(connectIpFilter);
            if (regex_match(request.getDstIpString(), reg)) {
                return true;
            }
        }
    } else if (request.getCommandString() == "BIND") {
        for (auto bindIpFilter : bindIpFilters) {
            regex reg(bindIpFilter);
            if (regex_match(request.getDstIpString(), reg)) {
                return true;
            }
        }
    }

    return false;
}

bool Firewall::loadConfigFile() {
    ifstream configFile(configFilename.c_str());
    if (!configFile.is_open()) {
        cerr << "Error! socks.conf not found." << endl;
        return false;
    }

    string line;
    while (getline(configFile, line)) {
        istringstream issLine;
        issLine.str(line);

        string permit, type, filterString;
        issLine >> permit >> type >> filterString;

        if (permit != "permit") {
            continue;
        }

        filterString = regex_replace(filterString, regex("\\."), "\\.");
        filterString = regex_replace(filterString, regex("\\*"), ".*");

        // cout << filterString << endl;

        if (type == "c") {
            connectIpFilters.push_back(filterString);
        } else if (type == "b") {
            bindIpFilters.push_back(filterString);
        }
    }

    configFile.close();

    return true;
}