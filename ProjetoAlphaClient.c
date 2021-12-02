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
    //Ainda falta ver os casos em que dá 1!
    return 0;
}

void parse(char* command){
    char name[11];
    sscanf(command, "%s", name);
    printf("Command: %s\n", name);
    int space_no = get_space_no(name);
}

int main(){
    char command[SIZE];
    while(fgets(command, SIZE, stdin)){
        //printf("%s", command);
        parse(command);
    }
}