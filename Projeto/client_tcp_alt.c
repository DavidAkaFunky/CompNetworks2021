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

int read_space(){
    char end[2];
    memset(end, 0, 2);
    ssize_t nread = tcp_read(end, 1);
    if (nread == -1)
        return -1;
    if (strcmp(" ", end)){
        puts(INFO_ERR);
        close(tcp_socket);
        return -1;
    }
    return 0;
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
        if (counter < 0)
            return;
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
    int text_strlen = strlen(text);
    if (text_strlen > 240){
        puts(BIG_TEXT);
        return;
    }
    if (!(is_alphanumerical(fname, 2) && check_login(UID) && check_select(GID)))
        return;
    int fname_strlen = strlen(fname);
    char message[259];
    memset(message, 0, 259);
    char textsize[4];
    memset(textsize, 0, 4);
    sprintf(textsize, "%d", text_strlen);
    sprintf(message, "PST %s %s %s %s", UID, GID, textsize, text);
    if (tcp_connect(res) == -1 || tcp_send(message, strlen(message)) == -1)
        return;
    if (fname_strlen > 24){
        close(tcp_socket);
        puts(ERR_FNAME_LEN);
        return;
    }
    if (fname_strlen > 0){
        if (fname[fname_strlen - 4] != '.'){
            close(tcp_socket);
            puts(INV_FILE);
            return;
        }
        char* ext = &(fname[fname_strlen - 3]);
        if (!is_alphanumerical(ext, 0)){
            close(tcp_socket);
            return;
        }
        char file_info[37];
        memset(file_info, 0, 37);
        FILE* fp = fopen(fname, "rb");
        if (!fp) {
            close(tcp_socket);
            puts(NO_FILE);
            return;
        }
        fseek(fp, 0L, SEEK_END);
        char fsize[11];
        memset(fsize, 0, 11);
        sprintf(fsize, "%ld", ftell(fp));
        sprintf(file_info, " %s %s ", fname, fsize);
        rewind(fp);
        if (tcp_send(file_info, strlen(file_info)) == -1){
            close(tcp_socket);
            fclose(fp);
            return;
        }
        char data[1025];
        while (1){
            memset(data, 0, 1025);
            int n = fread(data, 1, sizeof(data), fp);
            if (n == 0)
                break;
            if (tcp_send(data, n) == -1){
                close(tcp_socket);
                fclose(fp);
                return;
            }
        }
        fclose(fp);
    }
    if (tcp_send("\n", 1) == -1)
        return;
    char response[5];
    memset(response, 0, 5);
    ssize_t nread = tcp_read(response, 4);
    if (nread == -1)
        return;
    if (!strcmp("ERR\n", response)){
        close(tcp_socket);
        puts(GEN_ERR);
        return;
    } 
    if (strcmp("RPT ", response)){
        close(tcp_socket);
        puts(INFO_ERR);
        return;
    }
    char status[5];
    memset(status, 0, 5);
    nread = tcp_read(status, 4);
    if (nread == -1)
        return;
    if (!strcmp("NOK\n", status))
        puts(MSG_SEND_FAIL);
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
    }
    close(tcp_socket);
}

void retrieve(char* IP_ADDRESS, char* GID, char* UID, char* MID, struct addrinfo *res){
    char message[19];
    memset(message, 0, 19);
    sprintf(message, "RTV %s %s %s\n", UID, GID, MID);
    if (tcp_connect(res) == -1 || tcp_send(message, strlen(message)) == -1)
        return;
    char response[5];
    memset(response, 0, 5);
    ssize_t nread = tcp_read(response, 4);
    if (nread == -1)
        return;
    if (!strcmp("ERR\n", response)){
        close(tcp_socket);
        puts(GEN_ERR);
        return;
    }
    if (strcmp("RRT ", response)){
        close(tcp_socket);
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
    if (!strcmp("EOF", status)){
        nread = tcp_read(end, 1);
        if (nread == -1)
            return;
        if (!strcmp("\n", end))
            puts(NO_MSGS);
        else
            puts(INFO_ERR);
    }
    else if (!strcmp("NOK", status)){
        nread = tcp_read(end, 1);
        if (nread == -1)
            return;
        if (!strcmp("\n", end))
            puts(RTV_ERR);
        else
            puts(INFO_ERR);
    }
    else if (!strcmp("OK ", status)){
        char n[5];
        memset(n, 0, 5);
        int counter = 0;
        while (1){
            nread = tcp_read(n + counter, 1);
            if (nread == -1)
                return;
            if (n[counter] == '\n' || n[counter] == ' '){
                n[counter] = '\0';
                if (!(counter > 0 && digits_only(n, "number of messages"))){
                    close(tcp_socket);
                    puts(INFO_ERR);
                    return;
                }   
                break;
            }
            if (counter == 5){
                close(tcp_socket);
                puts(INFO_ERR);
                return;
            }
            ++counter;    
        }
        if (!strcmp(n, "0")){
            puts(NO_MSGS);
            close(tcp_socket);
            return;
        }
        else if (!strcmp(n, "1"))
            printf("Downloading %s message:\n", n);
        else
            printf("Downloading %s messages:\n", n);
        int messages = atoi(n), max = messages < 20 ? messages : 20, i, flag = 0;
        char rtv_MID[5], rtv_UID[6], tsize[4], text[241], fname[25], fsize[11];
        for (i = 0; i < max; ++i){
            memset(&(rtv_MID[flag]), 0, 5-flag);
            nread = tcp_read(&(rtv_MID[flag]), 4-flag);
            flag = 0;
            if (nread == -1)
                return;
            if (!(is_correct_arg_size(rtv_MID, 4) && digits_only(rtv_MID, "message ID"))){
                puts(INFO_ERR);
                close(tcp_socket);
                return;
            }
            if (read_space() == -1)
                return;
            memset(rtv_UID, 0, 6);
            nread = tcp_read(rtv_UID, 5);
            if (nread == -1)
                return;
            if (read_space() == -1)
                return;
            if (!(is_correct_arg_size(rtv_UID, 5) && digits_only(rtv_UID, "user ID"))){
                puts(INFO_ERR);
                close(tcp_socket);
                return;
            }
            memset(tsize, 0, 4);
            counter = 0;
            while (1){
                nread = tcp_read(tsize + counter, 1);
                if (nread == -1)
                    return;
                if (tsize[counter] == ' '){
                    tsize[counter] = '\0';
                    if (!(counter > 0 && digits_only(tsize, "text size"))){
                        puts(INFO_ERR);
                        close(tcp_socket);
                        return;
                    }   
                    break;
                }
                if (counter == 4){
                    puts(INFO_ERR);
                    close(tcp_socket);
                    return;
                }
                ++counter;    
            }
            memset(text, 0, 241);
            tcp_read(text, atoi(tsize));
            printf("Message ID: %s\nSent by: %s\nMessage size: %d\nMessage: %s", rtv_MID, rtv_UID, atoi(tsize)-1, text);
            memset(end, 0, 2);
            nread = tcp_read(end, 1);
            if (nread == -1)
                return;
            if (!strcmp("\n", end)){
                ++i;
                break;
            } else if (!strcmp(" ", end)){
                memset(end, 0, 2);
                nread = tcp_read(end, 1);
                if (!strcmp("/", end)){
                    //Tratar do ficheiro
                    if (read_space() == -1)
                        return;
                    memset(fname, 0, 25);
                    counter = 0;
                    while (1){
                        nread = tcp_read(fname + counter, 1);
                        if (nread == -1)
                            return;
                        if (fname[counter] == ' '){
                            fname[counter] = '\0';
                            if (!(counter > 0 && is_alphanumerical(fname, 2))){
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
                    memset(fsize, 0, 11);
                    counter = 0;
                    while (1){
                        nread = tcp_read(fsize + counter, 1);
                        if (nread == -1)
                            return;
                        if (fsize[counter] == ' '){
                            fsize[counter] = '\0';
                            if (!(counter > 0 && digits_only(fsize, "file size"))){
                                close(tcp_socket);
                                puts(INFO_ERR);
                                return;
                            }   
                            break;
                        }
                        if (counter == 11){
                            close(tcp_socket);
                            puts(INFO_ERR);
                            return;
                        }
                        ++counter;    
                    }
                    FILE* fp = fopen(fname, "wb");
                    if (!fp) {
                        puts(ERR_FILE);
                        close(tcp_socket);
                        return;
                    }
                    int nread, j;
                    char data[1025];
                    for (j = atoi(fsize); j > 0;){ //porque nao fazer tcp read do tamanho em vez de ciclo ? 
                        memset(data, 0, 1025);
                        nread = read(tcp_socket, end, j < 1024 ? j : 1024);
                        if (nread == -1){
                            puts(INFO_ERR);
                            fclose(fp);
                            close(tcp_socket);
                            return;
                        }
                        if (nread == 0){
                            if (j > 0){
                                puts(INFO_ERR);
                                fclose(fp);
                                close(tcp_socket);
                                return;
                            }
                            break;
                        }
                        fwrite(end, 1, nread, fp);
                        j -= nread;
                    }
                    fclose(fp);
                    puts("This message contains a file!");
                    printf("File name: %s\nFile size: %s\n", fname, fsize);
                    memset(end, 0, 2);
                    nread = tcp_read(end, 1);
                    if (nread == -1)
                        return;
                    if (!strcmp("\n", end)){
                        ++i;
                        break;
                    } else if (strcmp(" ", end)){
                        puts(INFO_ERR);
                        close(tcp_socket);
                        return;
                    }
                } else {
                    rtv_MID[0] = end[0];
                    flag = 1;
                }
                puts("");
            } else{
                puts(INFO_ERR);
                close(tcp_socket);
                return;
            }
        }
        if (i < max){
            printf("i = %d, max = %d\n", i, max);
            puts(INFO_ERR);
        }
    }
    else
        puts(INFO_ERR);
    close(tcp_socket);
}