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
#define NO_ALPH0 "The argument doesn't contain only alphanumerical characters, - or _. Please try again!"
#define NO_ALPH1 "The argument doesn't contain only alphanumerical characters. Please try again!"
#define NO_ALPH2 "The argument doesn't contain only alphanumerical characters, ., - or _. Please try again!"
#define NO_LOGIN "Not logged in. Please try again!"
#define NO_GROUP "No group selected. Please try again!"
#define GEN_ERR "The server returned an error. Please try again!"
#define CONN_ERR "There was an error conecting to the server. Please try again!"
#define SEND_ERR "There was an error sending information from the server. Please try again!"
#define RECV_ERR "There was an error receiving information from the server. Please try again!"
#define INFO_ERR "There was an error collecting information from the server. Please try again!"
#define SOCK_FAIL "Failed creating the socket!"
#define ADDR_FAIL "Failed getting the address' information!"
#define REG_USER_SUC "User successfully registered"
#define REG_USER_DUP "User already registered"
#define REG_USER_FAIL "Registration not accepted (too many users might be registered)."
#define UNR_USER_SUC "User successfully unregistered"
#define UNR_USER_FAIL "Unregister request unsuccessful"
#define LOGIN_SUC "User successfully logged in"
#define LOGIN_FAIL "Log in unsuccessful"
#define LOGIN_ERR "There was an error logging in. Please try again!"
#define LOGOUT_SUC "User successfully logged out"
#define LOGOUT_FAIL "Log out unsuccessful"
#define GRP_FAIL "The group ID does not exist. Please try again!"
#define REG_GRP_INV "The group name is invalid. Please try again!"
#define REG_GRP_FULL  "The group database is full. Please try again!"
#define REG_GRP_ERR1 "There was a problem subscribing. Please try again!"
#define UNR_GRP_FAIL_USR "The user ID does not exist. Please try again!"
#define UNR_GRP_ERR1 "There was a problem unsubscribing. Please try again!"
#define GRP_ERR "Either you're not logged in or your user is invalid. Please try again!"
#define NO_USERS "There are no users registered to this group."
#define NO_FILE "Input path invalid. Please try again!"
#define MSG_SEND_FAIL "Message sent unsuccessfully. Please try again!"

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
void show_groups(char* ptr, char* groups);

/* -------------------- client_tcp -------------------- */
int tcp_connect(struct addrinfo *res);
int tcp_send(char* message, int size);
int tcp_read(char* buffer, ssize_t size);
void ulist(char* IP_ADDRESS, char* GID, struct addrinfo *res);
void post(char* IP_ADDRESS, char* GID, char* UID, struct addrinfo *res, char *text, char *extra_text, char *fname);

#endif