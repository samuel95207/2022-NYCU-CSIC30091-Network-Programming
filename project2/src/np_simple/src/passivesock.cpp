// system types
#include <sys/types.h>
// system socket define
#include <sys/socket.h>
// struct sockaddr_in
#include <netinet/in.h>
// Network related functions, e.g. gethostbyname()
#include <netdb.h>
// va_list
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _PASSIVESOCK_H_
#define _PASSIVESOCK_H_
#include "passivesock.h"
#endif

// extern int errno;

// Allow test multi-service on the machine using different port
unsigned int portbase = 1024;
int passivesock(const char* service, const char* transport, int qlen) {
    // Store service entry return from getservbyname()
    struct servent* pse;
    // Store protocol entry return from getprotobyname()
    struct protoent* ppe;
    // Service-end socket
    struct sockaddr_in sin;
    // Service-end socket descriptor and service type
    int s, type;
    memset(&sin, 0, sizeof(sin));
    // TCP/IP suite
    sin.sin_family = AF_INET;
    // Use any local IP, need translate to internet byte order
    sin.sin_addr.s_addr = INADDR_ANY;
    // Get port number
    // service is service name
    pse = getservbyname(service, transport);
    if (pse) sin.sin_port = htons(ntohs((unsigned short)pse->s_port) + portbase);
    // service is port number
    else if ((sin.sin_port = htons((unsigned short)atoi(service))) == 0) {
        fprintf(stderr, "can't get \"%s\" service entry\n", service);
        return -1;
    }
    // Get protocol number
    if ((ppe = getprotobyname(transport)) == 0) {
        fprintf(stderr, "can't get \"%s\" protocol entry\n", transport);
        return -1;
    }
    // Tranport type
    if (strcmp(transport, "udp") == 0 || strcmp(transport, "UDP") == 0)
        type = SOCK_DGRAM;
    else
        type = SOCK_STREAM;
    // fprintf(stderr, "[SERVICE] transport: %s, protocol: %d, port: %u, type: %d\n", transport, ppe->p_proto,
    //         sin.sin_port, type);
    // Create socket
    s = socket(PF_INET, type, ppe->p_proto);
    if (s < 0) {
        fprintf(stderr, "can't create socket: %s \n", strerror(errno));
        return -1;
    }
    // Bind socket to service-end address

    int enable = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        fprintf(stderr, "setsockopt(SO_REUSEADDR) failed\n");
        return -1;
    }

    if (bind(s, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
        fprintf(stderr, "can't bind to %s port: %s \n", service, strerror(errno));
        return -1;
    }
    // For TCP socket, convert it to passive mode
    if (type == SOCK_STREAM && listen(s, qlen) < 0) {
        fprintf(stderr, "can't listen on %s port: %s \n", service, strerror(errno));
        return -1;
    }
    return s;
}