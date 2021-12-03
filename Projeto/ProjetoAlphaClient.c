#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#define PORT "58001"
#define SIZE 512

int get_space_no(char* name){
    if (!strcmp(name, "reg") || !strcmp(name, "unr") || !strcmp(name, "unregister") || !strcmp(name, "login")
        || !strcmp(name, "subscribe") || !strcmp(name, "post")){
        puts("2");
        return 2;
    }
    if (!strcmp(name, "unsubscribe") || !strcmp(name, "u") || !strcmp(name, "select") || !strcmp(name, "sag")
        || !strcmp(name, "retrieve") || !strcmp(name, "r")){
        puts("1");
        return 1;
    }
    if (!strcmp(name, "logout") || !strcmp(name, "exit") || !strcmp(name, "groups") || !strcmp(name, "gl")
        || !strcmp(name, "my_groups") || !strcmp(name, "my_gl") || !strcmp(name, "ulist") || !strcmp(name, "ul")){
        puts("0");
        return 0;
    }
    return -1;
}

void parse(char* command){
    char name[11];
    sscanf(command, "%s", name);
    printf("Command: %s\n", name);
    int space_no;
    // Se calhar isto ficaria mais simples se fizessemos
    // estes testes caso a caso, ja que temos que fazer
    // diferentes leituras conforme o nome do comando...
    while(1){
        space_no = get_space_no(name);
        if (space_no < 0)
            puts("Invalid command. Try again!");
    }
}

int main(){
    char command[SIZE];
    while(fgets(command, SIZE, stdin)){
        //printf("%s", command);
        parse(command);
    }
}