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
#include <dirent.h>
#include <sys/time.h>
#include <sys/stat.h>

#define BIND_FAIL "Failed binding information to the server socket!"
#define USERS_FAIL "Failed creating the USERS folder. Please try again!"
#define GROUPS_FAIL "Failed creating the GROUPS folder. Please try again!"
#define SEND_ERR "There was an error sending information to the client. Please try again!"
#define RECV_ERR "There was an error receiving information from the client. Please try again!"

typedef struct{
    char group_name[25];
    char gid[3];
    char last_msg[5];
} group;

/* -------------------- server_main -------------------- */

int udp_receive(int udp_socket, char* message);
int udp_send(int udp_socket, char* message);
int socket_bind(int socktype, char* port, struct addrinfo** res);
int parse_argv(int argc, char** argv, char* port, bool* verbose);
int parse_udp(int udp_socket, char* message);
int parse_tcp(int conn_fd, char* message, bool verbose);
int main(int argc, char** argv);

/* -------------------- server_udp --------------------- */
int reg(int udp_socket, char* uid, char* pass);
int unreg(int udp_socket, char* uid, char* pass);
int login(int udp_socket, char* uid, char* pass);
int logout(int udp_socket, char* uid, char* pass);
void find_last_message(char* gid, char* last_msg);
int comparer(const void* x1, const void* x2);
int list_groups_dir(group* list, bool my_groups, char* uid);
void send_groups(int udp_socket, group* list, int groups, char* message);
int groups(int udp_socket);
int my_groups(int udp_socket, char* uid);
int subscribe(int udp_socket, char* uid, char* gid, char* group_name);
int unsubscribe(int udp_socket, char* uid, char* gid);


/* -------------------- server_tcp --------------------- */
int tcp_read(int conn_fd, char* message, int size);
int tcp_send(int conn_fd, char* response, int size);
bool read_string(char* str, int conn_fd);
bool ulist(int conn_fd, bool verbose);
bool download_file(int conn_fd, char* path_name, bool verbose);
bool post(int conn_fd, bool verbose);
int get_number_of_messages(char* gid, int first_msg, char messages[20][5]);
void upload_file(int conn_fd, char* msg_path);
void get_messages(int conn_fd, char* gid, int n, char messages[20][5]);
bool retrieve(int conn_fd, bool verbose);

#endif