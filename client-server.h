

#ifndef CLIENT_SERVER_H
#define CLIENT_SERVER_H

// Standard and System Libraries
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <stdbool.h>
#include <regex.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <signal.h>
#include <arpa/inet.h>
#include <stddef.h>
// Protocol Identifiers
#define TCP 0
#define UDP 1

// Specifies the maximum number of pending connections on the server queue.
#define BACKLOG 10 

// Message Structure Definition
typedef struct data_msg data_msg_t;
struct data_msg{
    uint8_t version;    // 1-byte version value, typically set to 1.
    uint32_t data;      // Data payload as an unsigned 4-byte integer.
};

// Checks the validity of a given IPv4 address format.
bool IPcheck(const char *ipAddress);

// Determines the server type based on string input (either "tcp"/"TCP" or "udp"/"UDP").
int TCP_UDPcheck(char *serv);

// Validates that the provided port number falls within an acceptable range.
int port_check(char *port);

// Packs a data message into a transmittable format.
uint32_t pack(data_msg_t* data);

// Unpacks a received message into the data_msg structure.
void unpack(data_msg_t* data, uint32_t rec);

// Waits for a specified duration to receive a TCP message, handling timeouts and errors.
int recvtimeout(int sd, void *buf, int len, int timeout);

// Similar to recvtimeout, but tailored for UDP messages and includes sender information.
int recvfromtimeout(int sd, void *buf, int len, int timeout, struct sockaddr * their_addr, socklen_t addr_len);

// Facilitates sending a TCP message from the client and awaits server response.
int tcp_send(uint32_t data, char* ip, char* port, struct addrinfo * hints, struct addrinfo * servinfo);

// Facilitates sending a UDP message from the client and awaits server response.
int udp_send(uint32_t data, char* ip, char* port, struct addrinfo * hints, struct addrinfo * servinfo);

// Helps manage child processes by saving and restoring the errno value.
void sigchld_handler(int s);

#endif