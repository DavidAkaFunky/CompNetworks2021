#include "server.h"
#include "dirent.h"
#include "../common.h"

/**
 * @brief Receives the message sent from the client to the server (TCP).
 * 
 * @param conn_fd the file descriptor of the accepted socket.
 * @param message the message received from the client.
 * @param size the quantity of bytes that will be read.
 * @return the value that indicates success or failure.
 */
int tcp_read(int conn_fd, char* message, ssize_t size){
    ssize_t nleft = size, nread;
    char* ptr = message;
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

/**
 * @brief Sends the message sent from the client to the server (TCP).
 * 
 * @param conn_fd the file descriptor of the accepted socket.
 * @param response the response that the server sends to the client.
 * @param size the quantity of bytes that will be sent.
 * @return the value that indicates success or failure.
 */
int tcp_send(int conn_fd, char* response, ssize_t size){
    ssize_t nleft = size, nwritten;
    char* ptr = response;
    //In case the client doesn't accept the whole message, sends it in packages.
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

/**
 * @brief Reads a string.
 * 
 * @param str the string to be read by the function tcp_read.
 * @param conn_fd the file descriptor of the accepted socket.
 * @return true, if the function worked without an error.
 * @return false otherwise.
 */
bool read_string(char* str, int conn_fd){
    int len = strlen(str);
    char recv[len+1];
    bzero(recv, len+1);
    if (tcp_read(conn_fd, recv, len) == -1 || strcmp(str, recv))
        return false;
    return true;
}

/**
 * @brief Executes the ulist command.
 * 
 * @param conn_fd the file descriptor of the accepted socket.
 * @param verbose flag that makes the server run in verbose mode.
 * @return true, if the string is well-formatted and there were no errors. 
 * @return false otherwise.
 */
bool ulist(int conn_fd, bool verbose){
    char gid[3];
    char gid_path[10];
    bzero(gid, sizeof(gid));
    bzero(gid_path, sizeof(gid_path));

    if (tcp_read(conn_fd, gid, 2) == -1)
        return true;
    if (verbose)
        printf("GID: %s\n",gid);   

    sprintf(gid_path,"GROUPS/%s",gid);  
    if (!(is_correct_arg_size(gid, NULL, 2) && digits_only(gid, NULL) && read_string("\n", conn_fd)))
        return false;
    if (access(gid_path, F_OK) == -1){
        tcp_send(conn_fd, "RUL NOK\n", 8);
        return true;
    }
    
    //Sends all the subscribed users
    DIR* d = opendir(gid_path);
    struct dirent* dir;
    FILE* fp;

    char name_file[12];
    char uid_path[20];
    char uid_temp[7];
    char uid[8];
    bzero(name_file, sizeof(name_file));
    bzero(uid_path, sizeof(uid_path));
    bzero(uid, sizeof(uid));
    
    sprintf(name_file, "%s_name.txt", gid);
    
    //First sends the RUL OK GNAME
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
    if (tcp_send(conn_fd, send_status, strlen(send_status)) == -1){
        closedir(d);
        return true;
    }
    if (d){
        while ((dir = readdir(d)) != NULL){

            bzero(uid,8);
            bzero(uid_temp,7);
            bzero(uid_path,20);
            
            if(dir->d_name[0] == '.' || !strcmp(dir -> d_name, "MSG") || !strcmp(dir -> d_name, name_file))
                continue;
            
            char dir_name[10];
            bzero(dir_name, 10);
            strcpy(dir_name, dir -> d_name);
            sprintf(uid_path, "GROUPS/%s/%s", gid, dir_name);
            fp = fopen(uid_path,"r");
            fgets(uid_temp, 6, fp);
            sprintf(uid," %s",uid_temp);
            fclose(fp);
            if (tcp_send(conn_fd, uid, strlen(uid)) == -1){
                closedir(d);
                return true;
            }

        }
        closedir(d);
        if (tcp_send(conn_fd, "\n", 1) == -1)
            return true;
    }
    return true;
}

/**
 * @brief Auxiliary function that downloads the file sent from the client to the server.
 * 
 * @param conn_fd the file descriptor of the accepted socket.
 * @param path_name the path to where the file is located.
 * @param verbose flag that makes the server run in verbose mode.
 * @return true, if the string is well-formatted and there were no errors. 
 * @return false otherwise.
 */
bool download_file(int conn_fd, char* path_name, bool verbose){
    char file_name[25];
    bzero(file_name, 25);
    int counter = 0;
    while (true){
        if (tcp_read(conn_fd, file_name + counter, 1) == -1)
            return false;
        if (file_name[counter] == ' '){
            file_name[counter] = '\0';
            if (!(counter > 0 && is_alphanumerical(file_name, 2, false)))
                return false;
            break;
        }
        if (counter == 25)
            return false;
        ++counter;    
    }
    
    char file_size[11];
    bzero(file_size, 11);
    counter = 0;
    while (true){
        if (tcp_read(conn_fd,file_size + counter, 1) == -1)
            return false;
        if (file_size[counter] == ' '){
            file_size[counter] = '\0';
            if (!(counter > 0 && digits_only(file_size, NULL)))
                return false;
            break;
        }
        if (counter == 11)
            return false;
        ++counter;    
    }
    int fname_strlen = strlen(file_name);
    if (!(file_name[fname_strlen - 4] == '.' && is_alphanumerical(&(file_name[fname_strlen - 3]), 0, false) && strcmp(file_name,"T E X T.txt") && strcmp(file_name,"A U T H O R.txt")))
        return false;
    
    if (verbose){
        puts(FILE_IN_MSG);
        printf("File name: %s\nFile size: %s bytes\n", file_name, file_size);
    }
        
    char path[35];
    sprintf(path, "%s/%s", path_name, file_name);
    FILE* fp = fopen(path, "wb");
    if (!fp)
        return false;

    long total = atoi(file_size);
    char data[1025];
    int j, nread;
    for (j = total; j > 0; j -= nread){
        if (verbose)
            printf("Downloading file: %ld of %ld bytes...\r", total-j, total);
        bzero(data, 1025);
        nread = read(conn_fd, data, j < 1024 ? j : 1024);
        if (nread == -1){
            fclose(fp);
            return false;
        }
        if (nread == 0){
            if (j > 0){
                fclose(fp);
                return false;
            }
            break;
        }
        fwrite(data, 1, nread, fp);
    }
    if (verbose){
        printf("Downloading file: %ld of %ld bytes...\r", total-j, total);
        puts(FILE_DOWN_SUC);
    }
    
    fclose(fp);
    return true;
}

/**
 * @brief Executes the post command.
 * 
 * @param conn_fd the file descriptor of the accepted socket.
 * @param verbose flag that makes the server run in verbose mode.
 * @return true, if the string is well-formatted and there were no errors.
 * @return false otherwise.
 */
bool post(int conn_fd, bool verbose){
    //Check if uid exists and if the user is logged in
    char uid[6];
    bzero(uid, 6);
    if (tcp_read(conn_fd, uid, 5) == -1)
        return true;
    if (verbose)
        printf("UID: %s\n", uid);
    if (!(is_correct_arg_size(uid, NULL, 5) && digits_only(uid, NULL) && read_string(" ", conn_fd))){
        return false;
    }

    char path[29];
    bzero(path, 29);
    sprintf(path,"USERS/%s/%s_login.txt",uid,uid);
    if (access(path, F_OK) == -1){
        tcp_send(conn_fd, "RPT NOK\n", 8);
        return true;
    }
    // Check if gid exists and if the user is subscribed    
    char gid[3];
    bzero(gid, 3);
    if (tcp_read(conn_fd, gid, 2) == -1)
        return true;
    if (verbose)
        printf("GID: %s\n", gid);
    if (!(is_correct_arg_size(gid, NULL, 2) && digits_only(gid, NULL) && read_string(" ", conn_fd)))
        return false;

    bzero(path, 29);
    sprintf(path,"GROUPS/%s/%s.txt", gid, uid);
    if (access(path, F_OK) == -1){
        tcp_send(conn_fd, "RPT NOK\n", 8);
        return true;
    }
    
    // Check if size is a number up to 3 digits (maximum is 240 characters)
    char text_size[4];
    bzero(text_size, 4);
    int counter = 0;
    while (true){
        if (tcp_read(conn_fd, text_size + counter, 1) == -1)
            return true;
        if (text_size[counter] == ' '){
            text_size[counter] = '\0';
            if (!(counter > 0 && digits_only(text_size, NULL)))
                return false;
            break;
        }
        if (counter == 4)
            return false;
        ++counter;    
    }
    if (counter < 0)
        return false;
    
    if (verbose)
        printf("Text size: %s\n", text_size);
    //Transform the string into the number
    int size = atoi(text_size);
    if (size <= 0 || size > 240)
        return false;
    
    char message[size+1];
    bzero(message, size+1);
    if (tcp_read(conn_fd, message, size) == -1)
        return true;
    if (verbose)
        printf("Message: %s\n", message);
    char end[2];
    bzero(end, 2);
    if (tcp_read(conn_fd, end, 1) == -1)
        return true;
    
    if (strcmp(end, " ") && strcmp(end, "\n"))
        return false;

    char last_msg[5];
    bzero(last_msg, 5);
    find_last_message(gid, last_msg);
    char mid[5];
    bzero(mid, 5);
    add_trailing_zeros(atoi(last_msg)+1, 4, mid);
    // Create message folder
    bzero(path, 29);
    sprintf(path,"GROUPS/%s/MSG/%s",gid,mid);
    if (!access(path, F_OK)){ //Message with given MID already exists
        tcp_send(conn_fd, "RPT NOK\n", 8);
        return true;
    }
    if (mkdir(path, 0700) == -1){
        tcp_send(conn_fd, "RPT NOK\n", 8);
        return true;
    }
    char file_path[45];
    bzero(file_path, 45);
    sprintf(file_path, "%s/A U T H O R.txt", path);
    FILE* fp = fopen(file_path, "w");
    if (!fp){
        tcp_send(conn_fd, "RPT NOK\n", 8);
        return true;
    }
    fprintf(fp,"%s", uid);
    fclose(fp);
    
    bzero(file_path, 45);
    sprintf(file_path, "%s/T E X T.txt", path);
    fp = fopen(file_path, "w");
    if (!fp){
        tcp_send(conn_fd, "RPT NOK\n", 8);
        return true;
    }
    fprintf(fp,"%s", message);
    fclose(fp);

    // Download file
    if (!strcmp(end, " ") && !download_file(conn_fd, path, verbose)){
        if (!read_string("\n", conn_fd))
            return false;
        tcp_send(conn_fd, "RPT NOK\n", 8);
        return true;
    }

    char response[10];
    bzero(response, 10);
    sprintf(response, "RPT %s\n", mid);
    tcp_send(conn_fd, response, strlen(response));
    return true;
}

/**
 * @brief Returns the number of messages that have been stored in the array.
 * 
 * @param gid the group ID that has the messages.
 * @param first_msg the first message to be read.
 * @param messages the array that stores the messages ID.
 * @return the quantity of messages.
 */
int get_number_of_messages(char* gid, int first_msg, char messages[20][5]){
    char path[15], msg_path[19], file_path[35], msg[5];
    bzero(path, 15);
    sprintf(path,"GROUPS/%s/MSG/", gid);
    int n = 0;
    for (int i = first_msg; i < first_msg + 20; ++i){
        bzero(msg_path, 19);
        bzero(msg, 5);
        add_trailing_zeros(i, 4, msg);
        sprintf(msg_path, "%s%s", path, msg);
        if (!access(msg_path, F_OK)){
            bzero(file_path, 35);
            sprintf(file_path, "%s/A U T H O R.txt", msg_path);
            if (!access(file_path, F_OK)){
                bzero(file_path, 35);
                sprintf(file_path, "%s/T E X T.txt", msg_path);
                if (!access(file_path, F_OK))
                    strcpy(messages[n++], msg);
            }
        }
    }
    return n;
}

/**
 * @brief Auxiliary function that uploads the file from the server to the client.
 * 
 * @param conn_fd the file descriptor of the accepted socket.
 * @param msg_path the path to where the file is located.
 * @param verbose flag that makes the server run in verbose mode.
 */
void upload_file(int conn_fd, char* msg_path, bool verbose){
    char response[40], file_path[43];
    DIR* d = opendir(msg_path);
    struct dirent* dir;
    FILE* fp;
    if (d){
        while ((dir = readdir(d)) != NULL){

            if(dir->d_name[0]=='.' || !strcmp(dir -> d_name, "A U T H O R.txt") || !strcmp(dir -> d_name, "T E X T.txt"))
                continue;
            
            char dir_name[25];
            bzero(dir_name, 25);
            strcpy(dir_name, dir -> d_name);

            bzero(file_path, 43);
            sprintf(file_path, "%s/%s", msg_path, dir_name);
            fp = fopen(file_path, "rb");
            if (!fp)
                return;

            fseek(fp, 0L, SEEK_END);
            char file_size[11];
            sprintf(file_size, "%ld", ftell(fp));
            rewind(fp);

            bzero(response, 40);
            sprintf(response, " / %s %s ", dir_name, file_size);
            if (tcp_send(conn_fd, response, strlen(response)) == -1){
                fclose(fp);
                break;
            }
            char data[1024];
            long total = 0;
            int n;
            while (true){
                bzero(data, 1024);
                n = fread(data, 1, sizeof(data), fp);
                total += n;
                if (verbose)
                    printf("Uploading file: %ld of %s bytes...\r", total, file_size);
                if (n == 0)
                    break;
                if (tcp_send(conn_fd, data, n) == -1){
                    fclose(fp);
                    return;
                }
            }
            if (verbose)
                printf("Uploading file: %ld of %s bytes...\r", total, file_size);
            fclose(fp);
        }
        closedir(d);
    }
}

/**
 * @brief Iterate all messages in the array, and sends their content to the client.
 * 
 * @param conn_fd the file descriptor of the accepted socket.
 * @param gid the group ID that has the messages.
 * @param n the quantity of messages.
 * @param messages the array that stores the messages ID.
 * @param verbose flag that makes the server run in verbose mode.
 */
void get_messages(int conn_fd, char* gid, int n, char messages[20][5], bool verbose){
    char msg_path[19], file_path[35], uid[7], message[242], response[257];
    FILE* fp;
    for (int i = 0; i < n; ++i){
        bzero(msg_path, 19);
        sprintf(msg_path, "GROUPS/%s/MSG/%s", gid, messages[i]);

        bzero(file_path, 35);
        sprintf(file_path, "%s/A U T H O R.txt", msg_path);
        fp = fopen(file_path, "r");
        if (!fp)
            continue;
        bzero(uid, 7);
        fgets(uid, 6, fp);
        fclose(fp);

        bzero(file_path, 35);
        sprintf(file_path, "%s/T E X T.txt", msg_path);
        fp = fopen(file_path, "r");
        if (!fp)
            continue;
        bzero(message, 242);
        fgets(message, 241, fp);
        fclose(fp);
        if (strlen(message) > 240){
            continue;
        }
        bzero(response, 257);
        sprintf(response, " %s %s %ld %s", messages[i], uid, strlen(message), message);
        if (tcp_send(conn_fd, response, strlen(response)) == -1)
            continue;
        upload_file(conn_fd, msg_path, verbose);
    }
    if (tcp_send(conn_fd, "\n", 1) == -1)
        return;
    
}

/**
 * @brief Executes the retrieve command.
 * 
 * @param conn_fd the file descriptor of the accepted socket.
 * @param verbose flag that makes the server run in verbose mode.
 * @return true, if the string is well-formatted and there were no errors.
 * @return false otherwise.
 */
bool retrieve(int conn_fd, bool verbose){
    //Check if uid exists and if the user is logged in
    char uid[6];
    bzero(uid, 6);
    if (tcp_read(conn_fd, uid, 5) == -1)
        return true;
    if (verbose)
        printf("UID: %s\n", uid);
    if (!(is_correct_arg_size(uid, NULL, 5) && digits_only(uid, NULL) && read_string(" ", conn_fd))){
        return false;
    }

    char path[29];
    bzero(path, 29);
    sprintf(path,"USERS/%s/%s_login.txt",uid,uid);
    if (access(path, F_OK) == -1){
        tcp_send(conn_fd, "RRT NOK\n", 8);
        return true;
    }
    //Check if gid exists and if the user is subscribed    
    char gid[3];
    bzero(gid, 3);
    if (tcp_read(conn_fd, gid, 2) == -1)
        return true;
    if (verbose)
        printf("GID: %s\n", gid); 
    if (!(is_correct_arg_size(gid, NULL, 2) && digits_only(gid, NULL) && read_string(" ", conn_fd)))
        return false;
    
    bzero(path, 29);
    sprintf(path,"GROUPS/%s/%s.txt", gid, uid);
    if (access(path, F_OK) == -1){
        tcp_send(conn_fd, "RRT NOK\n", 8);
        return true;
    }
    
    char mid[5];
    bzero(mid, 5);
    if (tcp_read(conn_fd, mid, 4) == -1)
        return true;
    if (verbose)
        printf("MID: %s\n", mid);
    if (!(is_correct_arg_size(mid, NULL, 4) && digits_only(mid, NULL) && read_string("\n", conn_fd)))
        return false;
    char messages[20][5];
    int n = get_number_of_messages(gid, atoi(mid), messages);

    if (n == 0){
        tcp_send(conn_fd, "RRT EOF\n", 8);
        return true;
    }

    char response[10];
    sprintf(response, "RRT OK %d", n);
    tcp_send(conn_fd, response, strlen(response));
    bzero(response,strlen(response));
    get_messages(conn_fd, gid, n, messages, verbose);
    return true;
}