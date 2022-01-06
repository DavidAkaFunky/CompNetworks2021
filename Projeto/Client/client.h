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
#include <sys/stat.h>
#include <ctype.h>
#include <dirent.h>
#include <stdbool.h>

#define DOWNLOADS_FAIL "Failed creating the DOWNLOADS folder. Please try again!"
#define NO_LOGIN "Not logged in. Please try again!"
#define NO_GROUP "No group selected. Please try again!"
#define GEN_ERR "The server returned an error. Please try again!"
#define CONN_ERR "There was an error conecting to the server. Please try again!"
#define SEND_ERR "There was an error sending information to the server. Please try again!"
#define INFO_ERR "There was an error collecting information from the server. Please try again!"
#define ADDR_FAIL "Failed getting the address' information!"
#define REG_USER_SUC "You have registered successfully!"
#define REG_USER_DUP "A user with this uid has already been registered. Please try another one!"
#define REG_USER_FAIL "Registration not accepted (too many users might be registered)."
#define UNR_USER_SUC "You have unregistered successfully."
#define UNR_USER_FAIL "Unregister request unsuccessful."
#define LOGIN_SUC "You are now logged in!"
#define LOGIN_FAIL "Log in unsuccessful. Please confirm that this user exists and try again!"
#define LOGIN_ERR "There was an error logging in. Please try again!"
#define LOGIN_DOUBLE "You are currently logged in. To login again please logout first."
#define LOGOUT_SUC "You have logged out succesfully."
#define LOGOUT_FAIL "Log out unsuccessful."
#define GRP_FAIL "The group ID does not exist. Please try again!"
#define REG_GRP_INV "The group name is invalid. Please try again!"
#define REG_GRP_FULL  "The group database is full. Please try again!"
#define REG_GRP_ERR1 "There was a problem subscribing. Either you're not logged in or there's a group with the given ID but a different name. Please try again!"
#define UNR_GRP_FAIL_USR "The user ID does not exist. Please try again!"
#define UNR_GRP_ERR1 "There was a problem unsubscribing. Please confirm that you're logged in and try again!"
#define GRP_ERR "Either you're not logged in or your user is invalid. Please try again!"
#define NO_USERS "There are no users registered to this group."
#define NO_GROUPS "There are no registered groups."
#define NO_FILE "Input path invalid. Please try again!"
#define FORMAT_ERR "Incorrect text format: You must add quotation marks (\") around the text. Please try again!"
#define NO_TEXT "The text argument is empty. Please try again!"
#define BIG_TEXT "The text message is too big. Please try again!"
#define FNAME_LEN_ERR "Your file name is over 24 characters. Please try again!"
#define INV_FILE "The argument does not match a valid file. Please try again!"
#define MSG_SEND_FAIL "Message sent unsuccessfully. Please confirm that you're logged in and subscribed to this group, then try again!"
#define FILE_DOWN_SUC "\nFile successfully downloaded!"
#define FILE_UP_SUC "\nFile successfully uploaded!"
#define NO_MSGS "There are no messages in this group."
#define RTV_ERR "There was a problem retrieving the messages. Please confirm that you're logged in and subscribed to this group and try again!"
#define ERR_FILE "There was a problem writing to this file. Please confirm that you have writing permissions for it and try again!"

/* -------------------- client_main -------------------- */
int timer_on(int sd);
int timer_off(int sd);
int check_login(char *uid, bool log);
int check_select(char *gid);
int create_socket(struct addrinfo **res, int socktype, char* IP_ADDRESS, char* PORT);
int get_IP(char* IP_ADDRESS);
int get_local_IP(char* IP_ADDRESS);
int parse_argv(char* IP_ADDRESS, char* PORT, int argc, char** argv);
void parse(int udp_socket, int tcp_socket, struct addrinfo *res, char* IP_ADDRESS, char* PORT, char* command, char* uid, char* password, char* gid);
int main(int argc, char** argv);

/* -------------------- client_udp -------------------- */
int udp_send_and_receive(int fd, struct addrinfo *res, char* message, char* buffer, int size);
void reg(char* IP_ADDRESS, char* uid, char* password, struct addrinfo *res, int fd);
void unreg(char* IP_ADDRESS, char* uid, char* password, struct addrinfo *res, int fd);
int login(char* IP_ADDRESS, char* uid, char* password, struct addrinfo *res, int fd);
int logout(char* IP_ADDRESS, char* uid, char* password, struct addrinfo *res, int fd);
void groups(char* IP_ADDRESS, struct addrinfo *res, int fd);
void subscribe(char* IP_ADDRESS, char* uid, char* gid, char* group_name, struct addrinfo *res, int fd);
void unsubscribe(char* IP_ADDRESS, char* uid, char* gid, struct addrinfo *res, int fd);
void my_groups(char* IP_ADDRESS, char* uid, struct addrinfo *res, int fd);
void show_groups(char* ptr, char* groups);

/* -------------------- client_tcp -------------------- */
int tcp_connect(char* IP_ADDRESS, char* PORT, int* fd, struct addrinfo *res);
int tcp_send(char* message, int size);
int tcp_read(char* buffer, ssize_t size);
int read_space();
void ulist(char* IP_ADDRESS, char* PORT, char* gid, struct addrinfo *res);
int upload_file(char* fname);
void post(char* IP_ADDRESS, char* PORT, char* gid, char* uid, struct addrinfo *res, char *text, char *fname);
void retrieve(char* IP_ADDRESS, char* PORT, char* gid, char* uid, char* MID, struct addrinfo *res);

#endif