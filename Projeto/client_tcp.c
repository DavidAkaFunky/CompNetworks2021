#include "client.h"

int tcp_socket;

int tcp_send_and_receive(struct addrinfo *res, char* message, char* buffer, int size){
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
        nwritten = write(tcp_socket, ptr,nleft);
        if (nwritten <=0) {
            puts(SEND_ERR);
            return -1;
        }
        //printf("%s\n",ptr);
        nleft -= nwritten;
        ptr += nwritten;  
    }
    
    ptr = buffer;
    while(1){
        nread = read(tcp_socket, ptr, USERS);
        printf("Numero de bytes lido %ld\n", nread);
        if (nread == -1){
            puts(RECV_ERR);
            return -1;
        }
        else if(nread == 0)
            break;
        printf("%s\n",buffer);
        //printf("%s\n",ptr);
        ptr += nread;
    }
 
    //printf("%s\n",buffer);
    
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
}