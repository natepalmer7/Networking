*
    client.c 
    by Nathan Palmer
    for CSPB 3753 PA5
    Code adapted from Beej's Guide to Network Programming
        https://beej.us/guide/bgnet/

    Client program to send messages through network sockets using TCP or UDP
*/

#include "client-server.h"

int main (int argc, char *argv[]){
    struct addrinfo hints, *servinfo;
    int serv_type;

    // For capturing command-line arguments
    int opt;
    char *data_s = NULL;
    char *serv = NULL;
    char *ip = NULL;
    char *port = NULL;

    while ((opt = getopt(argc, argv, "x:t:s:p:")) != -1) {
        switch (opt) {
            case 'x':
                data_s = optarg;
                break;
            case 't':

                serv = optarg;
                break;
            case 's':
                ip = optarg;
                break;
            case 'p':
                port = optarg;
                break;
            case '?':
                fprintf(stderr, "Usage: %s -x <data> -t <udp or tcp> -s <ip> -p <port number>\n", argv[0]);
                return 3;
        }
    }

    if ( !data_s || !serv || !ip || !port){
        fprintf(stderr, "Usage: %s -x <data> -t <udp or tcp> -s <ip> -p <port number>\n", argv[0]);
        return 3;
    }
 // Checking IP address validity
    if (!IPcheck(ip)){
        fprintf(stderr, "Invalid IP address\n");
        return 3;
    }

    port_check(port);

    serv_type = TCP_UDPcheck(serv);

    // Convert input string to numeric type, ensuring it's within valid range
    uint32_t data_l = atoi(data_s);
    if (data_l > INT32_MAX){
        fprintf(stderr, "Input error\n");
        return 3;
    }

    // initialize struct to hold message data
    data_msg_t data = {1, data_l};

    // pack data 
    uint32_t packed = pack(&data);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // use IPv4

    switch(serv_type){
        case TCP:
            tcp_send(packed, ip, port, &hints, servinfo);
            break;
        case UDP:
            udp_send(packed, ip, port, &hints, servinfo);
            break;
        default:
            fprintf(stderr, "Invalid server type\n");
    }
    
}

int recvtimeout(int sd, void *buf, int len, int timeout){
    fd_set fds;
    int n;
    struct timeval tv;

    // Prepare file descriptor set for selection
    FD_ZERO(&fds);
    FD_SET(sd, &fds);

    // Define timeout parameters
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

     // Wait until data arrives or a timeout occurs
    n = select(sd+1, &fds, NULL, NULL, &tv);
    if (n == 0) return -2; // timeout!
    if (n == -1) return -1; // error

    // data must be here, so do a normal recv()
    return recv(sd, buf, len, 0);
}

int recvfromtimeout(int sd, void *buf, int len, int timeout, struct sockaddr * their_addr, socklen_t addr_len) {
    fd_set fds;
    int n;
    struct timeval tv;

    // set up the file descriptor set
    FD_ZERO(&fds);
    FD_SET(sd, &fds);

    // set up the struct timeval for the timeout
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    // wait until timeout or data received
    n = select(sd+1, &fds, NULL, NULL, &tv);
    if (n == 0) return -2; // timeout!
    if (n == -1) return -1; // error

    // data must be here, so do a normal recv()
    return recvfrom(sd, buf, len, 0, their_addr, &addr_len);
}

int tcp_send(uint32_t data, char* ip, char* port, struct addrinfo * hints, struct addrinfo * servinfo){
    struct addrinfo *p; // pointer to iterate through addrinfo list
    int res; // to store result of getaddrinfo() call
    int sockfd; // to store socket file descriptor
    int numbytes; // to store length of message
    int responsebytes; // to store response length
    uint8_t reply; // to store reply

    hints->ai_socktype = SOCK_STREAM; // use TCP protocol

    res = getaddrinfo(ip, port, hints, &servinfo);

    // Loop through servinfo results and connect to first available
    for (p = servinfo; p != NULL; p = p->ai_next){
        // Try to get sockfd
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("client:");
            continue;
        }

        // Try to connect to socket
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: ");
            continue;
        }

        // If no error is returned, connection is successful so break loop
        break;
    }

    freeaddrinfo(servinfo);

    // Check if loop ended with no successful connection
    if (p == NULL) {
        fprintf(stderr, "Client: failed to connect\n");
        return 1;
    }

    // Send data
    if ((numbytes = send(sockfd, &data, sizeof(data), 0)) == -1){
        fprintf(stderr, "Error: send\n");

        close(sockfd);
        return 1;
    }

    int ret = 0;

    if ((responsebytes = recvtimeout(sockfd, &reply, sizeof(reply), 3)) == -1) {
        fprintf(stderr, "Error: reply\n");
        ret = 1;
    }
    else if (responsebytes == -2) {
        fprintf(stderr, "Error: reply timeout\n");
        ret = 2;
    }
    else {
        if (reply == 1) {
            printf("success!\n");
            ret = 0;
        }
        else {
            fprintf(stderr, "Error: reply\n");
        }
        
    }

    close(sockfd);
    return ret;
};

int udp_send(uint32_t data, char* ip, char* port, struct addrinfo * hints, struct addrinfo * servinfo) {
    int sockfd; // socket file descriptor
    struct addrinfo *p;
    int res; // to store result of getaddrinfo call 
    int numbytes; // number of bytes sent
    int responsebytes; // to store response length
    uint8_t reply; // to store reply

    hints->ai_socktype = SOCK_DGRAM; // use udp socket

    // get address info and check for errors
    if ((res = getaddrinfo(ip, port, hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(res));
        return 1;
    }

    // loop through servinfo results and connect to first available
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }

        break;
    }

    // check if loop ended without successful connection
    if (p == NULL) {
        printf("Error: client failed to create socket\n");
        return 2;
    }

    // send data to socket and check for error
    if ((numbytes = sendto(sockfd, &data, sizeof(data), 0, p->ai_addr, p->ai_addrlen)) == -1){
        fprintf(stderr, "Error: client sendto\n");
        return 1;
    }

    int ret = 0;

    if ((responsebytes = recvfromtimeout(sockfd, &reply, sizeof(reply), 3, p->ai_addr, p->ai_addrlen)) == -1) {
        fprintf(stderr, "Error: reply\n");
        ret = 1;
    }
    else if (responsebytes == -2) {
        fprintf(stderr, "Error: reply timeout\n");
        ret = 2;
    }
    else {
        if (reply == 1) {
            printf("success!\n");
            ret = 0;
        }
        else {
            fprintf(stderr, "Error: reply\n");
        }
        
    }

    close(sockfd);
    return ret;
};