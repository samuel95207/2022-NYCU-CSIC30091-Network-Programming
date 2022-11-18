#include <iostream>
#include <sstream>
#include <string>



#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_
#include "HttpRequest.h"
#endif

using namespace std;

HttpRequest HttpRequest::parse(string rawRequest) {
    HttpRequest request;

    istringstream iss;
    iss.str(rawRequest);

    string methodStr;
    iss >> request.method >> request.uri >> request.version;

    string line;
    getline(iss, line);
    while (getline(iss, line)) {
        if (line.length() == 1) {
            continue;
        }
        istringstream issLine;
        issLine.str(line);
        string key, value;

        getline(issLine, key, ':');
        issLine >> ws;
        getline(issLine, value, '\r');

        // cout << key << "|" << value << endl;
        request.headerMap[key] = value;
    }

    istringstream issUrl;
    string urlPart, queryPart;
    issUrl.str(request.uri);
    getline(issUrl, request.uriOnly, '?');
    getline(issUrl, request.queryStr);


    istringstream issOnlyUri;
    issOnlyUri.str(request.uriOnly);
    string urlToken;
    while (getline(issOnlyUri, urlToken, '/')) {
        if (urlToken.length() == 0) {
            continue;
        }
        request.parsedUri.push_back(urlToken);
    }


    istringstream issQuery;
    issQuery.str(request.queryStr);
    string queryToken;
    while (getline(issQuery, queryToken, '&')) {
        if (queryToken.length() == 0) {
            continue;
        }
        istringstream queryTokenIss;
        queryTokenIss.str(queryToken);

        string key, value;
        getline(queryTokenIss, key, '=');
        getline(queryTokenIss, value);
        request.queryMap[key] = value;
    }


    istringstream issHost;
    request.host = request.headerMap["Host"];
    issHost.str(request.host);
    string portStr;
    getline(issHost, request.addr, ':');
    getline(issHost, portStr);
    request.port = stoi(portStr);



    return request;
}


void HttpRequest::print() {
    cout << "Method = " << method << endl;
    cout << "Uri = " << uri << endl;
    cout << "UriOnly = " << uriOnly << endl;
    cout << "QueryString = " << queryStr << endl;
    cout << "Version = " << version << endl;
    cout << "Host = " << host << endl;
    cout << "Addr = " << addr << endl;
    cout << "Port = " << port << endl;

    cout << "Query = {" << endl;
    for (auto keyValuePair : queryMap) {
        cout << "\t\"" << keyValuePair.first << "\" = " << keyValuePair.second << "," << endl;
    }
    cout << "}" << endl;

    cout << "Header = {" << endl;
    for (auto keyValuePair : headerMap) {
        cout << "\t\"" << keyValuePair.first << "\" = " << keyValuePair.second << "," << endl;
    }
    cout << "}" << endl;
}