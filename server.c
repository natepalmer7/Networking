/*
    server.c 
    by Nathan Palmer
    for CSPB 3753 PA5
    Code adapted from Beej's Guide to Network Programming
        https://beej.us/guide/bgnet/

    Server program to listen and receive messages through TCP or UDP
*/

#include "client-server.h"

int main(int argc, char *argv[]){
    struct addrinfo hints, *servinfo, *p;
    int sockfd; // socket to listen to
    int numbytes; // number of bytes received
    struct sockaddr_in their_addr; // connector's address information
    uint32_t data; // to store incoming data
    char s[INET_ADDRSTRLEN]; 
    int res; // result of getaddrinfo
    int yes=1; // for reusing port
    struct data_msg message; // structure to unpack incoming message
    int serv_type;

    // variables for command-line args
    int opt;
    char *serv = NULL;
    char *port = NULL;

    while ((opt = getopt(argc, argv, "t:p:")) != -1) {
        switch (opt) {
            case 't':
                serv = optarg;
                break;
            case 'p':
                port = optarg;
                break;
            case '?':
                fprintf(stderr, "Usage: %s -t <udp or tcp> -p <port number>\n", argv[0]);
                return 3;
        }
    }

    if (!serv || !port){
        fprintf(stderr, "Usage: %s -t <udp or tcp> -p <port number>\n", argv[0]);
        return 3;
    }

    serv_type = TCP_UDPcheck(serv);

    port_check(port);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;

    if (serv_type == TCP) {
        int new_fd; // new connection socket
        socklen_t sin_size;
        struct sigaction sa;

        
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE; // use my IP

        if ((res = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(res));
            return 1;
        }

        // loop through all the results and bind to the first we can
        for(p = servinfo; p != NULL; p = p->ai_next) {
            if ((sockfd = socket(p->ai_family, p->ai_socktype,
                    p->ai_protocol)) == -1) {
                perror("server: socket");
                continue;
            }

            // reuse port if address in use
            if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                    sizeof(int)) == -1) {
                perror("setsockopt");
                return 1;
            }

            // bind the socket
            if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
                close(sockfd);
                perror("server: bind");
                continue;
            }

            break;
        }

        freeaddrinfo(servinfo); // all done with this structure

        if (p == NULL)  {
            fprintf(stderr, "server: failed to bind\n");
            exit(1);
        }

        if (listen(sockfd, BACKLOG) == -1) {
            perror("listen");
            exit(1);
        }

        sa.sa_handler = sigchld_handler; // reap all dead processes
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        if (sigaction(SIGCHLD, &sa, NULL) == -1) {
            perror("sigaction");
            exit(1);
        }

        //printf("server: waiting for connections...\n");

        while(1) {  // main accept() loop
            sin_size = sizeof their_addr;
            new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
            if (new_fd == -1) {
                perror("accept");
                continue;
            }

            inet_ntop(their_addr.sin_family,
                &their_addr.sin_addr,
                s, sizeof s);
            //printf("server: got connection from %s\n", s);

            if (!fork()) { // this is the child process
                close(sockfd); // child doesn't need the listener
                if ((numbytes = recv(new_fd, &data, sizeof(data), 0)) == -1) {
                    perror("recv");
                    exit(1);
                }

                unpack(&message, data);

                printf("the sent number is: %u\n", message.data);

                uint8_t version = 1;

                if ((numbytes = send(new_fd, &version, sizeof(version), 0)) == -1) {
                    perror("REPLY send");
                    exit(1);
                }
                close(new_fd);
                exit(0);
            }
            close(new_fd);  // parent doesn't need this
        }

        return 0;
    }
    else {
        socklen_t addr_len;

        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags = AI_PASSIVE;

        if ((res = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
            printf("getaddrinfo: %s\n", gai_strerror(res));
            return 1;
        }

        // loop through all the results and bind to the first we can
        for(p = servinfo; p != NULL; p = p->ai_next) {
            if ((sockfd = socket(p->ai_family, p->ai_socktype,
                    p->ai_protocol)) == -1) {
                perror("server: socket");
                continue;
            }

            // reuse port if address in use
            if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                    sizeof(int)) == -1) {
                perror("setsockopt");
                return 1;
            }

            // bind the socket
            if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
                close(sockfd);
                perror("server: bind");
                continue;
            }

            break;
        }

        freeaddrinfo(servinfo); // all done with this structure

        if (p == NULL)  {
            fprintf(stderr, "server: failed to bind\n");
            exit(1);
        }

        while(1) {
            addr_len = sizeof their_addr;
            if ((numbytes = recvfrom(sockfd, &data, sizeof(data), 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
                perror("recvfrom");
                return 1;
            }

            unpack(&message, data);

            printf("the sent number is: %u\n", message.data);

            uint8_t version = 1;

            if((numbytes = sendto(sockfd, &version, sizeof(version), 0, (struct sockaddr *)&their_addr, addr_len)) == -1) {
                printf("Error: server reply\n");
                return 1;
            }

        }

        close(sockfd);

        return 0;

    }
}

void sigchld_handler(int s){
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}