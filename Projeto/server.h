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
#include <sys/time.h>
#include <dirent.h>
#include <sys/stat.h>

#define SIZE 512
#define BUF_SIZE 128
#define GROUPS 3275
#define USERS 600033
#define USERS_FAIL "Failed creating the USERS folder. Please try again!"
#define GROUPS_FAIL "Failed creating the GROUPS folder. Please try again!"
#define INVALID_CMD "Invalid command. Please try again!"
#define ARGV_ERR "You are running the program with incorrect arguments. Please try again!"
#define SOCK_FAIL "Failed creating the socket!"
#define RECV_ERR "There was an error receiving information from the client. Please try again!"
#define SEND_ERR "There was an error sending information from the server. Please try again!"
#define NO_ALPH0 "The argument doesn't contain only alphanumerical characters, - or _. Please try again!"
#define NO_ALPH1 "The argument doesn't contain only alphanumerical characters. Please try again!"
#define NO_ALPH2 "The argument doesn't contain only alphanumerical characters, ., - or _. Please try again!"


/* -------------------- server_main -------------------- */
int digits_only(char *s, char* id);
int is_alphanumerical(char* s, int flag);
int is_correct_arg_size(char* arg, int size);
int has_correct_arg_sizes(char* arg1, int size1, char* arg2, int size2);

int recv_udp(char* message);
int send_udp(char* message);
int recv_tcp(char* message);
int send_tcp(char* message);
int socket_bind(int socktype);
int parse_argv(int argc, char* argv[]);
int parse(char* message, char* response);
int main(int argc, char* argv[]);
/* -------------------- server_udp --------------------- */
int reg(char* uid, char* pass);
int unreg(char* uid, char* pass);

/* -------------------- server_tcp --------------------- */



#endif