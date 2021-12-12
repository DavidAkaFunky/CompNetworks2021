#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>

#define SIZE 512
#define BUF_SIZE 128
#define GROUPS 3275
#define USERS 600033
#define INVALID_CMD "Invalid command. Please try again!"
#define CONN_ERR "There was an error conecting to the server. Please try again!"
#define SEND_ERR "There was an error sending information from the server. Please try again!"
#define RECV_ERR "There was an error receiving information from the server. Please try again!"
#define INFO_ERR "There was an error collecting information from the server. Please try again!"

/* -------------------- client_main -------------------- */
int is_alphanumerical(char* s, int flag);
int is_correct_arg_size(char* arg, int size);
int has_correct_arg_sizes(char* arg1, int size1, char* arg2, int size2);
int digits_only(char *s, char* id);
int check_login(char *UID);
int check_select(char *GID);
void parse(int udp_socket, char* command, char* uid, char* password, char* gid);
int main(int argc, char* argv[]);
int create_socket(int socktype);

/* -------------------- client_udp -------------------- */
int udp_send_and_receive(int fd, struct addrinfo *res, char* message, char* buffer, int size);
void reg(char* IP_ADDRESS, char* UID, char* password, struct addrinfo *res, int fd);
void unreg(char* IP_ADDRESS, char* UID, char* password, struct addrinfo *res, int fd);
int login(char* IP_ADDRESS, char* UID, char* password, struct addrinfo *res, int fd);
int logout(char* IP_ADDRESS, char* UID, char* password, struct addrinfo *res, int fd);
void groups(char* IP_ADDRESS, struct addrinfo *res, int fd);
void subscribe(char* IP_ADDRESS, char* UID, char* GID, char* GName, struct addrinfo *res, int fd);
void unsubscribe(char* IP_ADDRESS, char* UID, char* GID, struct addrinfo *res, int fd);
void my_groups(char* IP_ADDRESS, char* UID, struct addrinfo *res, int fd);
void show_groups(char* ptr, char* groups, char* end);

/* -------------------- client_tcp -------------------- */
void users(char *ptr);
void ulist(char* IP_ADDRESS, char* GID, struct addrinfo *res);
int tcp_send_and_receive(struct addrinfo *res, char* message, char* buffer, int size);

#endif