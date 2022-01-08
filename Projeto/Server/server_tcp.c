#include "server.h"
#include "dirent.h"
#include "../common.h"

int tcp_read(int conn_fd, char* message, int size){
    ssize_t nleft = size, nread;
    char *ptr = message;
    while (nleft > 0){
        nread = read(conn_fd, ptr, nleft);
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

int tcp_send(int conn_fd, char* response){
    ssize_t nleft = strlen(response), nwritten;
    char *ptr = response;
    //Caso o servidor não aceite a mensagem completa, manda por packages
    while (nleft > 0){
        nwritten = write(conn_fd, ptr, nleft);
        if (nwritten <= 0){
            puts(SEND_ERR);
            return -1;
        }
        nleft -= nwritten;
        ptr += nwritten;  
    }
    return 1;
}

bool read_string(char* str, int conn_fd){
    int len = strlen(str);
    char recv[len+1];
    bzero(recv, len+1);
    if (tcp_read(conn_fd, recv, len) == -1 || strcmp(str, recv))
        return false;
    return true;
}

int ulist(int conn_fd, bool verbose){
    char gid[3];
    char enter[2];
    char gid_path[10];
    bzero(gid, sizeof(gid));
    bzero(enter, sizeof(enter));
    bzero(gid_path, sizeof(gid_path));
    if (tcp_read(conn_fd, gid, 2) == -1)
        return 0;
    if (tcp_read(conn_fd, enter, 1) == -1)
        return 0;
    if (verbose)    //imprime o resto caso verbose 
        printf("%s%s",gid,enter);   

    sprintf(gid_path,"GROUPS/%s",gid);    
    if (!digits_only(gid, "gid") && access(gid_path, F_OK) == -1)
        return tcp_send(conn_fd, "RUL NOK\n");
    
    //sends all the subscribed users
    DIR *d;
    struct dirent *dir;
    d = opendir(gid_path);
    FILE *fp;

    char name_file[12];
    char uid_path[20];
    char uid_temp[7];
    char uid[8];
    bzero(name_file, sizeof(name_file));
    bzero(uid_path, sizeof(uid_path));
    bzero(uid, sizeof(uid));
    
    sprintf(name_file, "%s_name.txt", gid);
    
    //first sends the RUL OK GNAME
    char send_status[31];
    char name_path[40];
    char gname[24];
    bzero(send_status, sizeof(send_status));
    bzero(name_path, sizeof(name_path));
    bzero(gname, sizeof(gname));
    sprintf(name_path, "GROUPS/%s/%s_name.txt", gid, gid);

    fp = fopen(name_path,"r");
    fgets(gname, 23, fp);
    fclose(fp);
    
    sprintf(send_status,"RUL OK %s",gname);
    if (tcp_send(conn_fd, send_status) == -1)
        return 0;

    if (d){
        while ((dir = readdir(d)) != NULL){

            bzero(uid,8);
            bzero(uid_temp,7);
            bzero(uid_path,20);
            if( !strcmp(dir -> d_name,".") || !strcmp(dir -> d_name, "..") || !strcmp(dir -> d_name, "MSG") || !strcmp(dir -> d_name, name_file))
                continue;
            
            sprintf(uid_path, "GROUPS/%s/%s", gid, dir -> d_name);
            fp = fopen(uid_path,"r");
            fgets(uid_temp, 6, fp);
            sprintf(uid," %s",uid_temp);
            fclose(fp);
            if (tcp_send(conn_fd, uid) == -1)
                return 0;
        }
        if (tcp_send(conn_fd, "\n") == -1)
            return 0;
    }
    return 1;
}

int download_file(int conn_fd, char *path_name, bool verbose){
    char file_name[25];
    bzero(file_name, 25);
    int counter = 0;
    while (1){
        if (tcp_read(conn_fd, file_name + counter, 1) == -1)
            return 0;
        if (file_name[counter] == ' '){
            file_name[counter] = '\0';
            if (!(counter > 0 && is_alphanumerical(file_name, 2)))
                return 0;
            break;
        }
        if (counter == 25)
            return 0;
        ++counter;    
    }
    
    char file_size[11];
    bzero(file_size, 11);
    counter = 0;
    while (1){
        if (tcp_read(conn_fd,file_size + counter, 1) == -1)
            return 0;
        if (file_size[counter] == ' '){
            file_size[counter] = '\0';
            if (!(counter > 0 && digits_only(file_size, "file size")))
                return 0;
            break;
        }
        if (counter == 11)
            return 0;
        ++counter;    
    }
    int fname_strlen = strlen(file_name);
    if (!(file_name[fname_strlen - 4] == '.' && is_alphanumerical(&(file_name[fname_strlen - 3]), 0) && strcmp(file_name,"T E X T.txt") && strcmp(file_name,"A U T H O R.txt")))
        return 0;
    puts("This message contains a file!");
    printf("File name: %s\nFile size: %s bytes\n", file_name, file_size);
    char path[35];
    sprintf(path, "%s/%s", path_name, file_name);
    FILE* fp = fopen(path, "wb");
    if (!fp) {
        return 0;
    }
    
    long total = atoi(file_size);
    char data[1025];
    int j, nread;
    for (j = total; j > 0; j -= nread){ 
        printf("Downloading file: %ld of %ld bytes...\r", total-j, total);
        bzero(data, 1025);
        nread = read(conn_fd, data, j < 1024 ? j : 1024);
        if (nread == -1){
            fclose(fp);
            return 0;
        }
        if (nread == 0){
            if (j > 0){
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

int post(int conn_fd, bool verbose){
    //Checks if uid exists and if the user is logged in
    char uid[6];
    bzero(uid, 6);
    if (tcp_read(conn_fd, uid, 5) == -1)
        return 1;
    if (!(is_correct_arg_size(uid, 5) && digits_only(uid, "uid") && read_string(" ", conn_fd))){
        return 0;
    }

    char path[19];
    bzero(path, 19);
    sprintf(path,"USERS/%s/%s_pass.txt",uid,uid);
    if (access(path, F_OK) == -1){
        tcp_send(conn_fd, "RPT NOK\n");
        return 1;
    }
    // Checks if gid exists and if the user is subscribed    
    char gid[3];
    bzero(gid, 3);
    if (tcp_read(conn_fd, gid, 2) == -1)
        return 1;
        
    if (!(is_correct_arg_size(gid, 2) && digits_only(gid,"gid") && read_string(" ", conn_fd)))
        return 0;

    bzero(path, 19);
    sprintf(path,"GROUPS/%s/%s.txt", gid, uid);
    if (access(path, F_OK) == -1){
        tcp_send(conn_fd, "RPT NOK\n");
        return 1;
    }
    
    // Checks if size is a number up to 3 digits (maximum is 240 characters)
    char text_size[4];
    bzero(text_size, 4);
    int counter = 0;
    while (1){
        if (tcp_read(conn_fd, text_size + counter, 1) == -1)
            return 1;
        if (text_size[counter] == ' '){
            text_size[counter] = '\0';
            if (!(counter > 0 && digits_only(text_size, "counter")))
                return 0;
            break;
        }
        if (counter == 4)
            return 0;
        ++counter;    
    }
    if (counter < 0)
        return 0;
    
    //Transform the string into the number
    int size = atoi(text_size);
    if (size <= 0 || size > 240)
        return 0;
    
    char message[size+1];
    bzero(message, size+1);
    if (tcp_read(conn_fd, message, size) == -1)
        return 1;

    char end[2];
    bzero(end, 2);
    if (tcp_read(conn_fd, end, 1) == -1)
        return 1;
    
    if (strcmp(end, " ") && strcmp(end, "\n"))
        return 0;
    
    char last_msg[5];
    bzero(last_msg, 5);
    find_last_message(gid, last_msg);
    char mid[5];
    bzero(mid, 5);
    add_trailing_zeros(atoi(last_msg)+1, 4, mid);
    // Create message folder
    bzero(path, 19);
    sprintf(path,"GROUPS/%s/MSG/%s",gid,mid);
    if (!access(path, F_OK)){ //Message with given MID already exists
        tcp_send(conn_fd, "RPT NOK\n");
        return 1;
    }
    if (mkdir(path, 0700) == -1){
        tcp_send(conn_fd, "RPT NOK\n");
        return 1;
    }
    char file_path[35];
    bzero(file_path, 35);
    sprintf(file_path, "%s/A U T H O R.txt", path);
    FILE* fp = fopen(file_path, "w");
    if (!fp){
        tcp_send(conn_fd, "RPT NOK\n");
        return 1;
    }
    fprintf(fp,"%s", uid);
    fclose(fp);
    
    bzero(file_path, 35);
    sprintf(file_path, "%s/T E X T.txt", path);
    fp = fopen(file_path, "w");
    if (!fp){
        tcp_send(conn_fd, "RPT NOK\n");
        return 1;
    }
    fprintf(fp,"%s", message);
    fclose(fp);
    // Download file
    if (!strcmp(end, " ") && !download_file(conn_fd, path, verbose)){
        puts("file");
        if (!read_string("\n", conn_fd))
            return 0;
        tcp_send(conn_fd, "RPT NOK\n");
        return 1;
    }

    char response[10];
    bzero(response, 10);
    sprintf(response, "RPT %s\n", mid);
    tcp_send(conn_fd, response);
    return 1;
}