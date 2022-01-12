#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define SIZE 512
#define BUF_SIZE 128
#define GROUPS 3275
#define USERS 600033
#define ARGV_ERR "You are running the program with incorrect arguments. Please try again!"
#define INVALID_CMD "Invalid command. Please try again!"
#define NO_ALPH0 "The argument doesn't contain only alphanumerical characters, - or _. Please try again!"
#define NO_ALPH1 "The argument doesn't contain only alphanumerical characters. Please try again!"
#define NO_ALPH2 "The argument doesn't contain only alphanumerical characters, ., - or _. Please try again!"
#define SOCK_FAIL "Failed creating the socket!"
#define ADDR_FAIL "Failed getting the address' information!"
#define FILE_IN_MSG "This message contains a file!"
#define FILE_UP_SUC "\nFile successfully uploaded!"
#define FILE_DOWN_SUC "\nFile successfully downloaded!"

int digits_only(char *s, char* name);
int is_alphanumerical(char* s, int flag, bool log);
int is_correct_arg_size(char* arg, char* name, int size);
int has_correct_arg_sizes(char* arg1, char* name1, int size1, char* arg2, char* name2, int size2);
void add_trailing_zeros(int number, int len, char* result);

#endif