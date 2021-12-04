#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>

#define PORT "58001"
#define SIZE 512
#define INVALID_CMD "Invalid command. Try again!"

/* -------------------- client_main -------------------- */
int is_correct_arg_size(char* arg, int size);
int has_correct_arg_sizes(char* arg1, int size1, char* arg2, int size2);
int digits_only(char *s);
void parse(char* command);
int main();

/* -------------------- client_tcp -------------------- */

/* -------------------- client_udp -------------------- */