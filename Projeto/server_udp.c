#include "server.h"

ssize_t bytes;

int reg(char* uid, char* pass){
    char path[12];
    bzero(path, 12);
    sprintf(path,"USERS/%s",uid);

    if (!(digits_only(uid,"UID") && has_correct_arg_sizes(uid, 5, pass, 8) && is_alphanumerical(pass, 0))){
        send_udp("RRG NOK\n");
        return 1;
    }
    if (!access(path, F_OK)){//if already exists the uid directory
        send_udp("RRG DUP\n");
        return 1;
    }
    //create a folder with the user uid, and a txt file with his password
    FILE *fp;
    char filepath[26];
    bzero(filepath, 26);
    sprintf(filepath,"%s/%s_pass.txt",path,uid);
    mkdir(path, 0700);
    fp = fopen(filepath, "w");
    fprintf(fp,"%s",pass);
    fclose(fp);
    send_udp("RRG OK\n");
    return 1;
}

int unreg(char* uid, char* pass){
    char path[12];
    bzero(path, 12);
    sprintf(path,"USERS/%s",uid);

    if (!(digits_only(uid,"UID") && has_correct_arg_sizes(uid, 5, pass, 8) && is_alphanumerical(pass, 0))){
        send_udp("RUN NOK\n");
        return 1;
    }
    if (access(path, F_OK)){    //user doesnt exist
        send_udp("RUN NOK\n");
        return 1;
    }
    //deletes folder with the user uid, and the txt file with his password
    char filepath[26];
    bzero(filepath, 26);
    sprintf(filepath,"%s/%s_pass.txt",path,uid);
    if (access(filepath, F_OK) || remove(filepath) == -1)
        return 0;
    bzero(filepath, 26);
    sprintf(filepath,"%s/%s_login.txt",path,uid);
    if (!access(filepath, F_OK) && remove(filepath) == -1) //if already exists the uid directory
        return 0;
    if (remove(path))
        return 0;
    send_udp("RUN OK\n");
    return 1;
}

/*
int log(char* uid, char* pass){
    char path[12];
    bzero(path, 12);
    sprintf(path,"USERS/%s",uid);

    if (!(digits_only(uid,"UID") && has_correct_arg_sizes(uid, 5, pass, 8) && is_alphanumerical(pass, 0))){
        send_udp("RLO NOK\n");
        return 1;
    }
    if (!access(path, F_OK)){//if doesnt exist the uid directory
        send_udp("RLO NOK\n");
        return 1;
    }
    //verifies id the password is correct from the uid_pass.txt

        //create a txt file with his log
        FILE *fp;
        char filepath[26];
        bzero(filepath, 26);
        sprintf(filepath,"%s/%s_login.txt",path,uid);
        mkdir(path, 0700);
        fp = fopen(filepath, "w");
        fprintf(fp,"%s",pass);
        fclose(fp);
    send_udp("RRG OK\n");
    return 1;
}*/