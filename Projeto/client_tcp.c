#include "client.h"

int tcp_socket;

int tcp_send(struct addrinfo *res, char* message, char* buffer, int size){
    int bytes;
    ssize_t nbytes,nleft,nwritten,nread;
    char *ptr;    
    tcp_socket = create_socket(SOCK_STREAM);

    if ((bytes = connect(tcp_socket, res->ai_addr, res->ai_addrlen)) == -1){
        puts(CONN_ERR);
        return -1;
    }
    //Caso o servidor não aceite a mensagem completa, manda por packages
    nbytes = sizeof(message);
    ptr = message;
    nleft = nbytes;
    while(nleft > 0) {
        nwritten = write(tcp_socket, ptr, nleft);
        if (nwritten <=0) {
            puts(SEND_ERR);
            return -1;
        }
        //printf("%s\n",ptr);
        nleft -= nwritten;
        ptr += nwritten;  
    }
    return bytes;
}

int tcp_read(char* buffer, int size){
    ssize_t nread = read(tcp_socket, buffer, size);
    if (nread == -1){
        puts(RECV_ERR);
        close(tcp_socket);
        return -1;
    }
    return nread;
}

/* void users(char *ptr){
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

void ulist(char* IP_ADDRESS, char* GID, struct addrinfo *res){ 
    char message[7], buffer[USERS];
    memset(message, 0, 7);
    memset(buffer, 0, USERS);
    sprintf(message,"ULS %s\n", GID);
    if (tcp_send_and_receive(res, message, buffer, USERS) == -1)
        return;
    //printf("%s\n",buffer);
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
} */

void ulist(char* IP_ADDRESS, char* GID, struct addrinfo *res){
    char message[7], buffer[USERS];
    memset(message, 0, 7);
    memset(buffer, 0, USERS);
    sprintf(message,"ULS %s\n", GID);
    if (tcp_send(res, message, buffer, USERS) == -1)
        return;
    char response[4];
    ssize_t nread = tcp_read(response, 4);
    if (nread == -1)
        return;
    if (!strcmp("ERR\n", response))
        puts(GEN_ERR);
    if (strcmp("RUL ", response) || nread == -1){
        puts(RECV_ERR);
        return;
    }
    char status[3];
    nread = tcp_read(status, 3);
    if (nread == -1)
        return;
    char end[1];
    if (!strcmp("NOK", status)){
        nread = tcp_read(end, 1);
        if (nread == -1)
            return;
        if (!strcmp("\n", end))
            puts(GRP_FAIL);
        else
            puts(RECV_ERR);
        close(tcp_socket);
        return;
    }
    if (!strcmp("OK ", status)){
        char* group_name;
        int counter = 0;
        while (1){
            nread = tcp_read(group_name + counter, 1);
            if (nread == -1)
                return;
            if (group_name[counter] == '\n' || group_name[counter] == ' '){
                if (!(counter > 0 && is_alphanumerical(group_name, 1))){
                    close(tcp_socket);
                    puts(INFO_ERR);
                    return;
                }     
                break;
            }
            if (counter == 25){
                close(tcp_socket);
                puts(INFO_ERR);
                return;
            }
            ++counter;    
        }
        printf("Group name: %s", group_name);
        if (group_name[counter] == '\n'){
            puts(NO_USERS);
            close(tcp_socket);
            return;
        }
        putchar('\n');
        printf("This group contains the following users: ");
        char user[5];  
        while (1){
            nread = tcp_read(user, 5);
            if (nread == -1)
                return;
            if (!digits_only(user, "user id")){
                close(tcp_socket);
                puts(INFO_ERR);
                return;
            }
            nread = tcp_read(end, 1);
            if (nread == -1)
                return;
            if (!strcmp("\n", end))
                break;
            else if (!strcmp(" ", end)){
                close(tcp_socket);
                puts(INFO_ERR);
                return;
            }
            puts(user);
        }
    }
    else
        puts(INFO_ERR);
    close(tcp_socket);
}