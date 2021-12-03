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
#define INVALID_CMD "Invalid command. Try again!"

int has_correct_args(char* arg1, char* arg2){
    // Talvez se vá usar: Verificar se o(s) arg(s) que precisamos não são vazios
}

int is_correct_arg_size(char* arg, int size){
    if (strlen(arg) != size){
        printf("%s's size is not %d. Try again!\n", arg, size);
        return 0;
    }
    return 1;
}

void parse(char* command){
    char name[11];
    char arg1[SIZE];
    char arg2[SIZE];
    sscanf(command, "%s %s %s", name, arg1, arg2);
    puts(name);
    if (!strcmp(name, "reg")){
        //Register (UDP): UID (tam 5), pass (tam 8)
        if (!(is_correct_arg_size(arg1, 5) && is_correct_arg_size(arg2, 8)))
            return;
    } else if (!strcmp(name, "unregister") || !strcmp(name, "unr")){
        //Unegister (UDP): UID (tam 5), pass (tam 8)
        if (!(is_correct_arg_size(arg1, 5) && is_correct_arg_size(arg2, 8)))
            return;
    } else if (!strcmp(name, "login")){
        //Login (UDP): UID (tam 5), pass (tam 8)
        if (!(is_correct_arg_size(arg1, 5) && is_correct_arg_size(arg2, 8)))
            return;
    } else if (!strcmp(name, "logout")){
        //Logout (UDP): (nada)
    } else if (!strcmp(name, "exit")){
        //Exit: (nada)
    } else if (!strcmp(name, "groups") || !strcmp(name, "gl")){
        //Groups (UDP): (nada)
    } else if (!strcmp(name, "subscribe") || !strcmp(name, "s")){
        //Subscribe (UDP): GID (tam 2), GName (tam 24)
        if (!(is_correct_arg_size(arg1, 2) && is_correct_arg_size(arg2, 24)))
            return;
    } else if (!strcmp(name, "unsubscribe") || !strcmp(name, "u")){
        //Unsubscribe (UDP): GID (tam 2)
        if (!is_correct_arg_size(arg1, 2))
            return;
    } else if (!strcmp(name, "my_groups") || !strcmp(name, "mgl")){
        //My groups (UDP): (nada)
    } else if (!strcmp(name, "select") || !strcmp(name, "sag")){
        //Select (UDP): GID (tam 2)
        if (!is_correct_arg_size(arg1, 2))
            return;
    } else if (!strcmp(name, "ulist") || !strcmp(name, "ul")){
        //User list (TCP): (nada)
    } else if (!strcmp(name, "post")){
        //Post (TCP): "text" (Verificar as aspas, talvez?), [FName] (Verificar os parênteses, talvez?)
    } else if (!strcmp(name, "retrieve") || !strcmp(name, "r")){
        //Retrieve (TCP): MID
    } else
        puts(INVALID_CMD);
}

int main(){
    char command[SIZE];
    while(fgets(command, SIZE, stdin)){
        parse(command);
    }
}