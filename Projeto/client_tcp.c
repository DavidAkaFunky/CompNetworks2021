#include "client.h"

int tcp_socket;

int tcp_send_and_receive(struct addrinfo *res, char* message, char* buffer, int size){
    int bytes;
    tcp_socket = create_socket(SOCK_STREAM);
    if ((bytes = connect(tcp_socket, res->ai_addr, res->ai_addrlen)) == -1){
        puts(CONN_ERR);
        return -1;
    }
    if (write(tcp_socket,message,sizeof(message)) == -1){
        puts(SEND_ERR);
        return -1;
    }
    if (read(tcp_socket,buffer,BUF_SIZE)==-1){
        puts(RECV_ERR);
        return -1;
    }
    close(tcp_socket);
    return bytes;
}

void users(char *ptr){
    if (strlen(ptr) == 0){
        puts(NO_USERS);
        return;
    }  
    char uid[6];
    printf("This group contains the following users: ");
    while (1){
        memset(uid, 0, 6);
        sscanf(ptr, "%s ", uid);
        if (strlen(uid) == 0)
            return;
        printf(", ");
        if (!(is_correct_arg_size(uid, 5) && digits_only(uid, "UID"))){
            puts(INFO_ERR);
            return;
        }
        printf("%s", uid);
        ptr += 6;
    }
}

//É preciso criar o socket TCP primeiro kekW
void ulist(char* IP_ADDRESS, char* GID, struct addrinfo *res){ 
    char message[7], buffer[USERS];
    memset(message, 0, 7);
    memset(buffer, 0, USERS);
    sprintf(message,"ULS %s\n", GID);
    if (tcp_send_and_receive(res, message, buffer, USERS) == -1)
        return;
    if (!strcmp("RUL NOK\n", buffer)){
        puts(GRP_FAIL);
        return;
    }
    char response[4], status[4], group_name[25];
    memset(response, 0, 4);
    memset(status, 0, 4);
    memset(group_name, 0, 25);
    sscanf(buffer, "%s %s %s ", response, status, group_name);
    if (!(!strcmp("RUL", response) && !strcmp("OK", status))){
        puts(INFO_ERR);
        return;
    }
    printf("Group name: %s\n", group_name);
    users(&(buffer[34]));
}