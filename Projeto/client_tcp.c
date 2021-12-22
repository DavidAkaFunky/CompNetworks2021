#include "client.h"

int tcp_socket;

int tcp_connect(struct addrinfo *res){
    int bytes;
    tcp_socket = create_socket(SOCK_STREAM);
    if ((bytes = connect(tcp_socket, res->ai_addr, res->ai_addrlen)) == -1){
        puts(CONN_ERR);
        return -1;
    }
    return bytes;
}

int tcp_send(char* message, int size){
    ssize_t nleft = size, nwritten;
    char *ptr = message;
    //Caso o servidor não aceite a mensagem completa, manda por packages
    while (nleft > 0){
        nwritten = write(tcp_socket, ptr, nleft);
        if (nwritten <= 0){
            puts(SEND_ERR);
            return -1;
        }
        nleft -= nwritten;
        ptr += nwritten;  
    }
    return 1;
}

int tcp_read(char* buffer, ssize_t size){
    ssize_t nleft = size, nread;
    char *ptr = buffer;
    while (nleft > 0){
        nread = read(tcp_socket, ptr, nleft);
        if (nread == -1){
            puts(RECV_ERR);
            close(tcp_socket);
            return -1;
        }
        else if (nread == 0)
            break;
        nleft -= nread;
        ptr += nread;
    }
    return 1;
}

void ulist(char* IP_ADDRESS, char* GID, struct addrinfo *res){
    char message[8];
    memset(message, 0, 8);
    sprintf(message,"ULS %s\n", GID);
    if (tcp_connect(res) == -1 || tcp_send(message, strlen(message)) == -1)
        return;
    char response[5];
    memset(response, 0, 5);
    ssize_t nread = tcp_read(response, 4);
    if (nread == -1)
        return;
    if (!strcmp("ERR\n", response))
        puts(GEN_ERR);
    if (strcmp("RUL ", response)){
        puts(INFO_ERR);
        return;
    }
    char status[4];
    memset(status, 0, 4);
    nread = tcp_read(status, 3);
    if (nread == -1)
        return;
    char end[2];
    memset(end, 0, 2);
    if (!strcmp("NOK", status)){
        nread = tcp_read(end, 1);
        if (nread == -1)
            return;
        if (!strcmp("\n", end)){
            puts(GRP_FAIL);
            return;
        }  
        close(tcp_socket);
        return;
    }
    if (!strcmp("OK ", status)){
        char group_name[25];
        memset(group_name, 0, 25);
        int counter = 0;
        while (1){
            nread = tcp_read(group_name + counter, 1);
            if (nread == -1)
                return;
            if (group_name[counter] == '\n' || group_name[counter] == ' '){
                group_name[counter] = '\0';
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
            close(tcp_socket);
            puts(NO_USERS);
            return;
        }
        putchar('\n');
        puts("This group contains the following users: ");
        char user[6];
        memset(user, 0, 6);  
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
            if (!strcmp("\n", end)){
                puts(user);
                break;
            }
            else if (strcmp(" ", end)){
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

void post(char* IP_ADDRESS, char* GID, char* UID, struct addrinfo *res, char *text, char *fname){
    //puts(text);
    int text_strlen = strlen(text);
    if (text_strlen > 240){
        puts("The text message is too big. Please try again!");
        return;
    }
    if (!(is_alphanumerical(text, 0) && is_alphanumerical(fname, 2) && check_login(UID) && check_select(GID)))
        return;
    int fname_strlen = strlen(fname);
    char message[259];
    memset(message, 0, 259);
    char text_size[4];
    memset(text_size, 0, 4);
    sprintf(text_size, "%d", text_strlen);
    sprintf(message, "PST %s %s %s %s", UID, GID, text_size, text);
    printf("Message = %s\n", message);
    if (tcp_connect(res) == -1 || tcp_send(message,strlen(message)) == -1)
        return;
    if (fname_strlen > 0){
        if (fname[fname_strlen - 4] != '.'){
            puts("We could not detect a file. Please try again!");
            return;
        }
        char* ext = &(fname[fname_strlen - 3]);
        if (!is_alphanumerical(ext, 0))
            return;
        char file_info[37];
        memset(file_info, 0, 37);
        FILE* fp = fopen(fname, "rb");
        if (!fp) {
            puts(NO_FILE);
            return;
        }
        char space[] = " ";
        if (tcp_send(space,strlen(space)) == -1)
            return;
        fseek(fp, 0L, SEEK_END);
        char fsize[11];
        memset(fsize, 0, 11);
        sprintf(fsize, "%ld", ftell(fp));
        sprintf(file_info, "%s %s ", fname, fsize);
        rewind(fp);
        if (tcp_send(file_info,strlen(file_info)) == -1){
            fclose(fp);
            return;
        }
        char data[1025];
        memset(data, 0, 1025);
        while(1){
            int n = fread(data, 1, sizeof(data), fp);
            if (n == 0)
                break;
            if (tcp_send(data,n) == -1){
                fclose(fp);
                return;
            }
            memset(data, 0, 1025);
        }
    }
    if (tcp_send("\n",1) == -1)
        return;
    char response[5];
    memset(response, 0, 5);
    ssize_t nread = tcp_read(response, 4);
    if (nread == -1)
        return;
    if (!strcmp("ERR\n", response))
        puts(GEN_ERR);
    if (strcmp("RPT ", response)){
        puts(INFO_ERR);
        return;
    }
    char status[5];
    memset(status, 0, 5);
    nread = tcp_read(status, 4);
    if (nread == -1)
        return;
    if (!strcmp("NOK\n", status)){
        puts(MSG_SEND_FAIL);
        close(tcp_socket);
        return;
    }
    else {
        if (!digits_only(status, "message ID")){
            puts(INFO_ERR);
            close(tcp_socket);
            return;
        }
        char end[2];
        memset(end, 0, 2);
        nread = tcp_read(end, 1);
        if (nread == -1)
            return;
        if (!strcmp("\n", end))
            printf("Message sent successfully. Message ID: %s\n", status);
        else
            puts(INFO_ERR);
        close(tcp_socket);
    }
}

