#include "client.h"
#include "../common.h"

int tcp_connect(char* IP_ADDRESS, char* PORT, int* fd, struct addrinfo *res){
    int bytes;
    *fd = create_socket(&res, SOCK_STREAM, IP_ADDRESS, PORT);
    if ((bytes = connect(*fd, res->ai_addr, res->ai_addrlen)) == -1){
        puts(CONN_ERR);
        return -1;
    }
    return bytes;
}

int tcp_send(int fd, char* message, int size){
    ssize_t nleft = size, nwritten;
    char *ptr = message;
    //Caso o servidor não aceite a mensagem completa, manda por packages
    while (nleft > 0){
        nwritten = write(fd, ptr, nleft);
        if (nwritten <= 0){
            puts(SEND_ERR);
            return -1;
        }
        nleft -= nwritten;
        ptr += nwritten;  
    }
    return 1;
}

int tcp_read(int fd, char* buffer, ssize_t size){
    ssize_t nleft = size, nread;
    char *ptr = buffer;
    while (nleft > 0){
        nread = read(fd, ptr, nleft);
        if (nread == -1){
            puts(RECV_ERR);
            return -1;
        }
        else if (nread == 0)
            break;
        nleft -= nread;
        ptr += nread;
    }
    return 1;
}

int read_space(int tcp_socket){
    char end[2];
    bzero(end, 2);
    ssize_t nread = tcp_read(tcp_socket, end, 1);
    if (nread == -1)
        return -1;
    if (strcmp(" ", end)){
        puts(INFO_ERR);
        return -1;
    }
    return 0;
}

void ulist(char* IP_ADDRESS, char* PORT, char* gid, struct addrinfo *res, int tcp_socket){
    char message[8];
    bzero(message, 8);
    sprintf(message,"ULS %s\n", gid);
    if (tcp_connect(IP_ADDRESS, PORT, &tcp_socket, res) == -1 || tcp_send(tcp_socket, message, strlen(message)) == -1)
        return;
    char response[5];
    bzero(response, 5);
    ssize_t nread = tcp_read(tcp_socket, response, 4);
    if (nread == -1)
        return;
    if (!strcmp("ERR\n", response))
        puts(GEN_ERR);
    if (strcmp("RUL ", response)){
        puts(INFO_ERR);
        return;
    }
    char status[4];
    bzero(status, 4);
    nread = tcp_read(tcp_socket, status, 3);
    if (nread == -1)
        return;
    char end[2];
    bzero(end, 2);
    if (!strcmp("NOK", status)){
        nread = tcp_read(tcp_socket, end, 1);
        if (nread == -1)
            return;
        if (!strcmp("\n", end)){
            puts(GRP_FAIL);
            return;
        }  
        return;
    }
    if (!strcmp("OK ", status)){
        char group_name[25];
        bzero(group_name, 25);
        int counter = 0;
        while (1){
            nread = tcp_read(tcp_socket, group_name + counter, 1);
            if (nread == -1)
                return;
            if (group_name[counter] == '\n' || group_name[counter] == ' '){
                group_name[counter] = '\0';
                if (!(counter > 0 && is_alphanumerical(group_name, 1))){
                    puts(INFO_ERR);
                    return;
                }   
                break;
            }
            if (counter == 25){
                puts(INFO_ERR);
                return;
            }
            ++counter;    
        }
        if (counter < 0)
            return;
        printf("Group name: %s", group_name);
        if (group_name[counter] == '\n'){
            puts(NO_USERS);
            return;
        }
        putchar('\n');
        puts("This group contains the following users: ");
        char user[6];
        bzero(user, 6);  
        while (1){
            nread = tcp_read(tcp_socket, user, 5);
            if (nread == -1)
                return;
            if (!digits_only(user, "user id")){
                puts(INFO_ERR);
                return;
            }
            nread = tcp_read(tcp_socket, end, 1);
            if (nread == -1)
                return;
            if (!strcmp("\n", end)){
                puts(user);
                break;
            }
            else if (strcmp(" ", end)){
                puts(INFO_ERR);
                return;
            }
            puts(user);
        }
    }
    else
        puts(INFO_ERR);
}

int upload_file(int tcp_socket, char* file_name){
    char file_info[37];
    bzero(file_info, 37);
    FILE* fp = fopen(file_name, "rb");
    if (!fp) {
        puts(NO_FILE);
        return 0;
    }
    fseek(fp, 0L, SEEK_END);
    char file_size[11];
    bzero(file_size, 11);
    sprintf(file_size, "%ld", ftell(fp));
    sprintf(file_info, " %s %s ", file_name, file_size);
    rewind(fp);
    if (tcp_send(tcp_socket, file_info, strlen(file_info)) == -1){
        fclose(fp);
        return 0;
    }
    char data[1025];
    long total = 0;
    int n;
    while (1){
        bzero(data, 1025);
        n = fread(data, 1, sizeof(data), fp);
        total += n;
        printf("Uploading file: %ld of %s bytes...\r", total, file_size);
        if (n == 0)
            break;
        if (tcp_send(tcp_socket, data, n) == -1){
            fclose(fp);
            return 0;
        }
    }
    puts(FILE_UP_SUC);
    fclose(fp);
    return 1;
}

void post(char* IP_ADDRESS, char* PORT, char* gid, char* uid, struct addrinfo *res, char *text, char *file_name, int tcp_socket){
    int text_strlen = strlen(text);
    if (text_strlen > 240){
        puts(BIG_TEXT);
        return;
    }
    if (!(is_alphanumerical(file_name, 2) && check_login(uid, true) && check_select(gid)))
        return;
    int fname_strlen = strlen(file_name);
    char message[259];
    bzero(message, 259);
    char text_size[4];
    bzero(text_size, 4);
    sprintf(text_size, "%d", text_strlen);
    sprintf(message, "PST %s %s %s %s", uid, gid, text_size, text);
    if (tcp_connect(IP_ADDRESS, PORT, &tcp_socket, res) == -1 || tcp_send(tcp_socket, message, strlen(message)) == -1)
        return;
    if (fname_strlen > 24){
        puts(FNAME_LEN_ERR);
        return;
    }
    if (fname_strlen > 0){
        if (file_name[fname_strlen - 4] != '.'){
            puts(INV_FILE);
            return;
        }
        char* ext = &(file_name[fname_strlen - 3]);
        if (!(is_alphanumerical(ext, 0) && upload_file(tcp_socket,file_name)))
            return;
    }
    if (tcp_send(tcp_socket, "\n", 1) == -1)
        return;
    char response[5];
    bzero(response, 5);
    ssize_t nread = tcp_read(tcp_socket, response, 4);
    if (nread == -1)
        return;
    if (!strcmp("ERR\n", response)){
        puts(GEN_ERR);
        return;
    } 
    if (strcmp("RPT ", response)){
        puts(INFO_ERR);
        return;
    }
    bzero(response, 5);
    nread = tcp_read(tcp_socket, response, 4);
    if (nread == -1)
        return;
    if (!strcmp("NOK\n", response))
        puts(MSG_SEND_FAIL);
    else {
        if (!digits_only(response, "message ID")){
            puts(INFO_ERR);
            return;
        }
        char end[2];
        bzero(end, 2);
        nread = tcp_read(tcp_socket, end, 1);
        if (nread == -1)
            return;
        if (!strcmp("\n", end))
            printf("Message sent successfully. Message ID: %s\n", response);
        else
            puts(INFO_ERR);
    }
}

int download_file(int tcp_socket){
    if (read_space(tcp_socket) == -1)
        return 0;
    char file_name[25], file_size[11];
    bzero(file_name, 25);
    int counter = 0, nread;
    while (1){
        nread = tcp_read(tcp_socket, file_name + counter, 1);
        if (nread == -1)
            return 0;
        if (file_name[counter] == ' '){
            file_name[counter] = '\0';
            if (!(counter > 0 && is_alphanumerical(file_name, 2))){
                puts(INFO_ERR);
                return 0;
            }   
            break;
        }
        if (counter == 25){
            puts(INFO_ERR);
            return 0;
        }
        ++counter;    
    }
    bzero(file_size, 11);
    counter = 0;
    while (1){
        nread = tcp_read(tcp_socket, file_size + counter, 1);
        if (nread == -1)
            return 0;
        if (file_size[counter] == ' '){
            file_size[counter] = '\0';
            if (!(counter > 0 && digits_only(file_size, "file size"))){
                puts(INFO_ERR);
                return 0;
            }   
            break;
        }
        if (counter == 11){
            puts(INFO_ERR);
            return 0;
        }
        ++counter;    
    }
    puts("This message contains a file!");
    printf("File name: %s\nFile size: %s bytes\n", file_name, file_size);
    char file_path[35];
    sprintf(file_path, "DOWNLOADS/%s", file_name);
    FILE* fp = fopen(file_path, "wb");
    if (!fp) {
        puts(ERR_FILE);
        return 0;
    }
    int j;
    long total = atoi(file_size);
    char data[1025];
    for (j = total; j > 0; j -= nread){ //porque nao fazer tcp read do tamanho em vez de ciclo ? 
        printf("Downloading file: %ld of %ld bytes...\r", total-j, total);
        bzero(data, 1025);
        nread = read(tcp_socket, data, j < 1024 ? j : 1024);
        if (nread == -1){
            puts(INFO_ERR);
            fclose(fp);
            return 0;
        }
        if (nread == 0){
            if (j > 0){
                puts(INFO_ERR);
                fclose(fp);
                return 0;
            }
            break;
        }
        fwrite(data, 1, nread, fp);
    }
    puts(FILE_DOWN_SUC);
    fclose(fp);
    return 1;
}

void retrieve(char* IP_ADDRESS, char* PORT, char* gid, char* uid, char* MID, struct addrinfo *res, int tcp_socket){
    char message[19];
    bzero(message, 19);
    sprintf(message, "RTV %s %s %s\n", uid, gid, MID);
    if (tcp_connect(IP_ADDRESS, PORT, &tcp_socket, res) == -1 || tcp_send(tcp_socket, message, strlen(message)) == -1)
        return;
    char response[5];
    bzero(response, 5);
    ssize_t nread = tcp_read(tcp_socket, response, 4);
    if (nread == -1)
        return;
    if (!strcmp("ERR\n", response)){
        puts(GEN_ERR);
        return;
    }
    if (strcmp("RRT ", response)){
        puts(INFO_ERR);
        return;
    }
    char status[4];
    bzero(status, 4);
    nread = tcp_read(tcp_socket, status, 3);
    if (nread == -1)
        return;
    char end[2];
    bzero(end, 2);
    if (!strcmp("EOF", status)){
        nread = tcp_read(tcp_socket, end, 1);
        if (nread == -1)
            return;
        if (!strcmp("\n", end))
            puts(NO_MSGS);
        else
            puts(INFO_ERR);
    }
    else if (!strcmp("NOK", status)){
        nread = tcp_read(tcp_socket, end, 1);
        if (nread == -1)
            return;
        if (!strcmp("\n", end))
            puts(RTV_ERR);
        else
            puts(INFO_ERR);
    }
    else if (!strcmp("OK ", status)){
        char n[5];
        bzero(n, 5);
        int counter = 0;
        while (1){
            nread = tcp_read(tcp_socket, n + counter, 1);
            if (nread == -1)
                return;
            if (n[counter] == '\n' || n[counter] == ' '){
                n[counter] = '\0';
                if (!(counter > 0 && digits_only(n, "number of messages"))){
                    puts(INFO_ERR);
                    return;
                }   
                break;
            }
            if (counter == 5){
                puts(INFO_ERR);
                return;
            }
            ++counter;    
        }
        if (!strcmp(n, "0")){
            puts(NO_MSGS);
            return;
        }
        else if (!strcmp(n, "1"))
            printf("Downloading %s message:\n", n);
        else
            printf("Downloading %s messages:\n", n);
        int messages = atoi(n), max = messages < 20 ? messages : 20, i, flag = 0;
        char rtv_MID[5], rtv_uid[6], tsize[4], text[241];
        for (i = 0; i < max; ++i){
            bzero(&(rtv_MID[flag]), 5-flag);
            nread = tcp_read(tcp_socket, &(rtv_MID[flag]), 4-flag);
            flag = 0;
            if (nread == -1)
                return;
            if (!(is_correct_arg_size(rtv_MID, 4) && digits_only(rtv_MID, "message ID"))){
                puts(INFO_ERR);
                return;
            }
            if (read_space(tcp_socket) == -1)
                return;
            bzero(rtv_uid, 6);
            nread = tcp_read(tcp_socket, rtv_uid, 5);
            if (nread == -1)
                return;
            if (read_space(tcp_socket) == -1)
                return;
            if (!(is_correct_arg_size(rtv_uid, 5) && digits_only(rtv_uid, "user ID"))){
                puts(INFO_ERR);
                return;
            }
            bzero(tsize, 4);
            counter = 0;
            while (1){
                nread = tcp_read(tcp_socket, tsize + counter, 1);
                if (nread == -1)
                    return;
                if (tsize[counter] == ' '){
                    tsize[counter] = '\0';
                    if (!(counter > 0 && digits_only(tsize, "text size"))){
                        puts(INFO_ERR);
                        return;
                    }   
                    break;
                }
                if (counter == 4){
                    puts(INFO_ERR);
                    return;
                }
                ++counter;    
            }
            bzero(text, 241);
            tcp_read(tcp_socket, text, atoi(tsize));
            printf("Message ID: %s\nSent by: %s\nMessage size: %d\nMessage: %s", rtv_MID, rtv_uid, atoi(tsize)-1, text);
            bzero(end, 2);
            nread = tcp_read(tcp_socket, end, 1);
            if (nread == -1)
                return;
            if (!strcmp("\n", end)){
                ++i;
                break;
            } else if (!strcmp(" ", end)){
                bzero(end, 2);
                nread = tcp_read(tcp_socket, end, 1);
                if (!strcmp("/", end)){
                    //Tratar do ficheiro
                    if (!download_file(tcp_socket))
                        return;
                    bzero(end, 2);
                    nread = tcp_read(tcp_socket, end, 1);
                    if (nread == -1)
                        return;
                    if (!strcmp("\n", end)){
                        ++i;
                        break;
                    } else if (strcmp(" ", end)){
                        puts(INFO_ERR);
                        return;
                    }
                } else {
                    rtv_MID[0] = end[0];
                    flag = 1;
                }
                puts("");
            } else{
                puts(INFO_ERR);
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
}