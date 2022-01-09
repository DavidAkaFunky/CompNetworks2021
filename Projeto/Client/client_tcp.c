#include "client.h"
#include "../common.h"

int tcp_socket;

int tcp_connect(char* ip_address, char* port, int* fd, struct addrinfo *res){
    int bytes;
    tcp_socket = create_socket(&res, SOCK_STREAM, ip_address, port);
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
        timer_on(tcp_socket);
        nread = read(tcp_socket, ptr, nleft);
        timer_off(tcp_socket);
        if (nread == -1){
            close(tcp_socket);
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

bool read_space(){
    char end[2];
    bzero(end, 2);
    if (tcp_read(end, 1) == -1)
        return false;
    if (strcmp(" ", end)){
        close(tcp_socket);
        puts(INFO_ERR);
        return false;
    }
    return true;
}

void ulist(char* ip_address, char* port, char* gid, struct addrinfo *res){
    char message[8];
    bzero(message, 8);
    sprintf(message,"ULS %s\n", gid);
    if (tcp_connect(ip_address, port, &tcp_socket, res) == -1 || tcp_send(message, strlen(message)) == -1)
        return;
    char response[5];
    bzero(response, 5);
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
    bzero(status, 4);
    nread = tcp_read(status, 3);
    if (nread == -1)
        return;
    char end[2];
    bzero(end, 2);
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
        bzero(group_name, 25);
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
        bzero(user, 6);  
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

int upload_file(char* file_name){
    char file_info[37];
    bzero(file_info, 37);
    FILE* fp = fopen(file_name, "rb");
    if (!fp){
        close(tcp_socket);
        puts(NO_FILE);
        return 0;
    }
    fseek(fp, 0L, SEEK_END);
    char file_size[11];
    bzero(file_size, 11);
    sprintf(file_size, "%ld", ftell(fp));
    sprintf(file_info, " %s %s ", file_name, file_size);
    rewind(fp);
    if (tcp_send(file_info, strlen(file_info)) == -1){
        close(tcp_socket);
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
        if (tcp_send(data, n) == -1){
            close(tcp_socket);
            fclose(fp);
            return 0;
        }
    }
    puts(FILE_UP_SUC);
    fclose(fp);
    return 1;
}

void post(char* ip_address, char* port, char* gid, char* uid, struct addrinfo *res, char *text, char *file_name){
    int text_strlen = strlen(text);
    if (text_strlen > 240){
        puts(BIG_TEXT);
        return;
    }
    if (!(is_alphanumerical(file_name, 2) && check_login(uid, true) && check_group(gid)))
        return;
    int fname_strlen = strlen(file_name);
    char message[259];
    bzero(message, 259);
    char text_size[4];
    bzero(text_size, 4);
    sprintf(text_size, "%d", text_strlen);
    sprintf(message, "PST %s %s %s %s", uid, gid, text_size, text);
    if (tcp_connect(ip_address, port, &tcp_socket, res) == -1 || tcp_send(message, strlen(message)) == -1)
        return;
    if (fname_strlen > 24){
        close(tcp_socket);
        puts(FNAME_LEN_ERR);
        return;
    }
    if (fname_strlen > 0){
        if (file_name[fname_strlen - 4] != '.'){
            close(tcp_socket);
            puts(INV_FILE);
            return;
        }
        if (!(is_alphanumerical(&(file_name[fname_strlen - 3]), 0))){
            close(tcp_socket);
            puts(INV_FILE);
            return;
        }
        if (!upload_file(file_name))
            return;
    }
    if (tcp_send("\n", 1) == -1)
        return;
    char response[5];
    bzero(response, 5);
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
    bzero(response, 5);

    nread = tcp_read(response, 4);
    if (nread == -1)
        return;
    if (!strcmp("NOK\n", response))
        puts(MSG_SEND_FAIL);
    else {
        if (!digits_only(response, "message ID")){
            close(tcp_socket);
            puts(INFO_ERR);
            return;
        }
        char end[2];
        bzero(end, 2);
        nread = tcp_read(end, 1);
        if (nread == -1)
            return;
        if (!strcmp("\n", end))
            printf("Message sent successfully. Message ID: %s\n", response);
        else
            puts(INFO_ERR);
    }
    close(tcp_socket);
}

int download_file(){
    if (!read_space())
        return 0;
    char file_name[25], file_size[11];
    bzero(file_name, 25);
    int counter = 0, nread;
    while (1){
        if (tcp_read(file_name + counter, 1) == -1)
            return 0;
        if (file_name[counter] == ' '){
            file_name[counter] = '\0';
            if (!(counter > 0 && is_alphanumerical(file_name, 2))){
                close(tcp_socket);
                puts(INFO_ERR);
                return 0;
            }   
            break;
        }
        if (counter == 25){
            close(tcp_socket);
            puts(INFO_ERR);
            return 0;
        }
        ++counter;    
    }
    bzero(file_size, 11);
    counter = 0;
    while (1){
        if (tcp_read(file_size + counter, 1) == -1)
            return 0;
        if (file_size[counter] == ' '){
            file_size[counter] = '\0';
            if (!(counter > 0 && digits_only(file_size, "file size"))){
                close(tcp_socket);
                puts(INFO_ERR);
                return 0;
            }   
            break;
        }
        if (counter == 11){
            close(tcp_socket);
            puts(INFO_ERR);
            return 0;
        }
        ++counter;    
    }
    if (file_name[strlen(file_name) - 4] != '.'){
        close(tcp_socket);
        puts(INV_FILE);
        return 0;
    }
    if (!(is_alphanumerical(&(file_name[strlen(file_name) - 3]), 0))){
        close(tcp_socket);
        puts(INV_FILE);
        return 0;
    }
    puts(FILE_IN_MSG);
    printf("File name: %s\nFile size: %s bytes\n", file_name, file_size);
    char file_path[35];
    sprintf(file_path, "DOWNLOADS/%s", file_name);
    FILE* fp = fopen(file_path, "wb");
    if (!fp) {
        close(tcp_socket);
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
            close(tcp_socket);
            fclose(fp);
            puts(INFO_ERR);
            return 0;
        }
        if (nread == 0){
            if (j > 0){
                close(tcp_socket);
                puts(INFO_ERR);
                fclose(fp);
                return 0;
            }
            break;
        }
        fwrite(data, 1, nread, fp);
    }
    printf("Downloading file: %ld of %ld bytes...\r", total-j, total);
    puts(FILE_DOWN_SUC);
    fclose(fp);
    return 1;
}

void retrieve(char* ip_address, char* port, char* gid, char* uid, char* MID, struct addrinfo *res){
    char message[19];
    bzero(message, 19);
    sprintf(message, "RTV %s %s %s\n", uid, gid, MID);
    if (tcp_connect(ip_address, port, &tcp_socket, res) == -1 || tcp_send(message, strlen(message)) == -1)
        return;
    char response[5];
    bzero(response, 5);
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
    bzero(status, 4);
    nread = tcp_read(status, 3);
    if (nread == -1)
        return;
    char end[2];
    bzero(end, 2);
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
        bzero(n, 5);
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
            close(tcp_socket);
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
            nread = tcp_read(&(rtv_MID[flag]), 4-flag);
            flag = 0;
            if (nread == -1)
                return;
            if (!(is_correct_arg_size(rtv_MID, 4) && digits_only(rtv_MID, "message ID"))){
                close(tcp_socket);
                puts(INFO_ERR);
                return;
            }
            if (!read_space())
                return;
            bzero(rtv_uid, 6);
            nread = tcp_read(rtv_uid, 5);
            if (nread == -1)
                return;
            if (!read_space())
                return;
            if (!(is_correct_arg_size(rtv_uid, 5) && digits_only(rtv_uid, "user ID"))){
                close(tcp_socket);
                puts(INFO_ERR);
                return;
            }
            bzero(tsize, 4);
            counter = 0;
            while (1){
                nread = tcp_read(tsize + counter, 1);
                if (nread == -1)
                    return;
                if (tsize[counter] == ' '){
                    tsize[counter] = '\0';
                    if (!(counter > 0 && digits_only(tsize, "text size"))){
                        close(tcp_socket);
                        puts(INFO_ERR);
                        return;
                    }   
                    break;
                }
                if (counter == 4){
                    close(tcp_socket);
                    puts(INFO_ERR);
                    return;
                }
                ++counter;    
            }
            bzero(text, 241);
            tcp_read(text, atoi(tsize));
            printf("Message ID: %s\nSent by: %s\nMessage size: %d\nMessage: %s\n", rtv_MID, rtv_uid, atoi(tsize), text);
            bzero(end, 2);
            nread = tcp_read(end, 1);

            if (nread == -1)
                return;
            if (!strcmp("\n", end)){
                ++i;
                break;
            } else if (!strcmp(" ", end)){
                bzero(end, 2);
                nread = tcp_read(end, 1);
                if (!strcmp("/", end)){
                    //Tratar do ficheiro
                    if (!download_file(tcp_socket))
                        return;
                    bzero(end, 2);
                    nread = tcp_read(end, 1);
                    if (nread == -1)
                        return;
                    if (!strcmp("\n", end)){
                        ++i;
                        break;
                    } else if (strcmp(" ", end)){
                        close(tcp_socket);
                        puts(INFO_ERR);
                        return;
                    }
                } else {
                    rtv_MID[0] = end[0];
                    flag = 1;
                }
                puts("");
            } else{
                close(tcp_socket);
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
    close(tcp_socket);
}