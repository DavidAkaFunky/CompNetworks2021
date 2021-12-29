#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>
#include <stdbool.h>

#define SIZE 512
#define BUF_SIZE 128
#define GROUPS 3275
#define USERS 600033
#define INVALID_CMD "Invalid command. Please try again!"
#define ARGV_ERR "You are running the program with incorrect arguments. Please try again!"
#define SOCK_FAIL "Failed creating the socket!"
#define RECV_ERR "There was an error receiving information from the client. Please try again!"
#define SEND_ERR "There was an error sending information from the server. Please try again!"


/* -------------------- server_main -------------------- */
int recv_udp();
int socket_bind(int socktype);
int parse_argv(int argc, char* argv[]);
void parse();
int main(int argc, char* argv[]);
/* -------------------- server_udp --------------------- */

/* -------------------- server_tcp --------------------- */



#endif