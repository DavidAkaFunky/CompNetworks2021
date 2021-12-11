#include "client.h"

//É preciso criar o socket TCP primeiro kekW
int ulist(char* IP_ADDRESS, char* GID, struct addrinfo *res, int fd){
    /*char message[20], buffer[BUF_SIZE];
    memset(message, 0, 20);
    memset(buffer, 0, BUF_SIZE);
    sprintf(message,"ULS %s\n", GID);
    if (connect(fd, res, BUF_SIZE) == -1)
        return -1;
    printf("TEST\n");
    if (write(fd,message,sizeof(message)) == -1) //morre aqui ?
        return -1;
    printf("TEST\n"); 
    if (read(fd,buffer,BUF_SIZE)==-1)
        return -1;
    printf("TEST\n");
    close(fd);
    printf("%s",buffer);
    return 0;*/
}