/*
    helpers.c 
    by Nathan Palmer
    for CSPB 3753 PA5
    Code adapted from Beej's Guide to Network Programming
        https://beej.us/guide/bgnet/

    Helper functions for client and server programs
*/

#include "client-server.h"

// Pack the version and data of the data_msg_t structure into a single 32-bit integer.
uint32_t pack(data_msg_t* data){
    uint32_t packed = data->data;
    packed = packed | (data->version)<<31;
    return htonl(packed);
}

void unpack(data_msg_t* data, uint32_t rec){
    uint32_t packed = ntohl(rec);
    data->version = packed>>31;
    data->data = packed & ~(1 << 31);
}

bool IPcheck(const char *ipAddress) {
    regex_t regex;
    int ret = regcomp(&regex, "^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$", REG_EXTENDED);
    if (ret != 0) {
        fprintf(stderr, "Could not compile regex\n");
        return false;
    }

    ret = regexec(&regex, ipAddress, 0, NULL, 0);
    regfree(&regex);

    return ret == 0;
}

int TCP_UDPcheck(char *serv) {
    if (!strcmp(serv, "tcp") || !strcmp(serv, "TCP")){
        return TCP;
    }
    else if (!strcmp(serv, "udp") || !strcmp(serv, "UDP")){
        return UDP;
    }
    else {
        fprintf(stderr, "Invalid server type, please use -t tcp or -t udp\n");
        exit(3);
    }
}

int port_check(char *port) {
    int iport = atoi(port);
    if (iport > 65535 || iport < 1024){
        fprintf(stderr, "Invalid port number\n");
        exit(3);
    }
    return 0;
}
      