#include "server.h"

ssize_t bytes;

int reg(char* uid, char* pass){
    char path[12];
    bzero(path, 12);
    sprintf(path,"USERS/%s",uid);

    if (!(digits_only(uid,"uid") && has_correct_arg_sizes(uid, 5, pass, 8) && is_alphanumerical(pass, 0)))
        return 0;

    if (!access(path, F_OK)){//if already exists the uid directory
        send_udp("RRG DUP\n");
        return 1;
    }

    //create a folder with the user uid, and a txt file with his password
    char file_path[26];
    bzero(file_path, 26);
    sprintf(file_path,"%s/%s_pass.txt",path,uid);
    if (mkdir(path, 0700) == -1){
        send_udp("RRG NOK\n");
        return 1;
    }

    FILE* fp = fopen(file_path, "w");
    if (!fp){
        send_udp("RRG NOK\n");
        return 1;
    }
    fprintf(fp,"%s",pass);
    fclose(fp);

    send_udp("RRG OK\n");
    return 1;
}

int unreg(char* uid, char* pass){
    char path[12];
    bzero(path, 12);
    sprintf(path,"USERS/%s",uid);

    if (!(digits_only(uid,"uid") && has_correct_arg_sizes(uid, 5, pass, 8) && is_alphanumerical(pass, 0)))
        return 0;

    if (access(path, F_OK) == -1){    //user doesnt exist
        send_udp("RUN NOK\n");
        return 1;
    }

    //deletes folder with the user uid, and the txt file with his password
    char file_path[26];
    bzero(file_path, 26);
    sprintf(file_path,"%s/%s_pass.txt",path,uid);
    if (access(file_path, F_OK) || remove(file_path)){
        send_udp("RUN NOK\n");
        return 1;
    }
    bzero(file_path, 26);
    sprintf(file_path,"%s/%s_login.txt",path,uid);
    if (access(file_path, F_OK) || remove(file_path)){ //if already exists the uid directory
        send_udp("RUN NOK\n");
        return 1;
    }
    send_udp("RUN OK\n");
    return 1;
}


int login(char* uid, char* pass){
    char path[12];
    bzero(path, 12);
    sprintf(path,"USERS/%s",uid);

    if (!(digits_only(uid,"uid") && has_correct_arg_sizes(uid, 5, pass, 8) && is_alphanumerical(pass, 0)))
        return 0;
    
    if (access(path, F_OK) == -1){//if doesnt exist the uid directory
        send_udp("RLO NOK\n");
        return 1;
    }
    //verifies if the password is correct from the uid_pass.txt
    char file_path[26], pass_temp[9];
    bzero(file_path, 26);
    sprintf(file_path,"%s/%s_pass.txt",path,uid);
    FILE *fp = fopen(file_path,"r");
    if (!fp){
        send_udp("RLO NOK\n");
        return 1;
    }

    fgets(pass_temp,9,fp);
    fclose(fp);

    if (strcmp(pass_temp,pass)){
        send_udp("RLO NOK\n");
        return 1;
    }

    //create a txt file with his log
    bzero(file_path, 26);
    sprintf(file_path,"%s/%s_login.txt",path,uid);
    fp = fopen(file_path, "w");
    if (!fp){
        puts("Here2");
        send_udp("RLO NOK\n");
        return 1;
    }
    fclose(fp);

    send_udp("RLO OK\n");
    return 1;
}

int logout(char* uid, char* pass){
    // if the format is correct
    if (!(digits_only(uid,"uid") && has_correct_arg_sizes(uid, 5, pass, 8) && is_alphanumerical(pass, 0)))
        return 0;
    
    char path[12];
    bzero(path, 12);
    sprintf(path,"USERS/%s",uid);

    if (access(path, F_OK) == -1){//if the uid directory exists
        send_udp("ROU NOK\n");
        return 1;
    }

    //Check if the password is correct from the uid_pass.txt
    char file_path[26], pass_temp[9];
    bzero(file_path, 26);
    sprintf(file_path,"%s/%s_pass.txt",path,uid);
    FILE* fp = fopen(file_path,"r");
    if (!fp){
        send_udp("ROU NOK\n");
        return 1;
    }
    fgets(pass_temp,9,fp);
    fclose(fp);

    if (strcmp(pass_temp,pass)){
        send_udp("ROU NOK\n");
        return 1;
    }

    //delete the txt file with his log
    bzero(file_path, 26);
    sprintf(file_path,"%s/%s_login.txt",path,uid);

    //Check if the login file exists
    if (access(file_path, F_OK) || remove(file_path)){ //checks if login exists and if it can delete it
        send_udp("ROU NOK\n");
        return 1;
    }

    send_udp("ROU OK\n");
    return 1;
}

/* GROUPLIST n existe???
int ListGroupsDir(GROUPLIST *list)
{
    DIR *d;
    struct dirent *dir;
    int i=0;
    FILE *fp;
    char gidname[30];
    list->no_groups=0;
    d = opendir("GROUPS");
    if (d){
        while ((dir = readdir(d)) != NULL){
            if(dir->d_name[0]=='.' || strlen(dir->d_name)>2)
                continue;
            strcpy(list->group_no[i], dir->d_name);
            sprintf(gidname,"GROUPS/%s/%s_name.txt",dir->d_name,dir->d_name);
            fp=fopen(gidname,"r");
            if(fp){
                fscanf(fp,"%24s",list->group_name[i]);
                fclose(fp);
            }
            ++i;   
            if(i==99)
                break;
        }
        list->no_groups=i;
        closedir(d);
    }
    else
        return -1;
    if(list->no_groups>1)
        SortGList(list);
    return list->no_groups;
}*/

void subscribe(char* uid, char* gid, char* group_name, int* global_gid){
    bool new_group = false;

    if (strlen(gid) == 1)
        sprintf(gid, "0%c", gid[0]);

    char uid_path[12];
    bzero(uid_path, 12);
    sprintf(uid_path,"USERS/%s",uid);

    //Check if the UID is well-formatted and is registered
    if (!(digits_only(uid,"uid") && is_correct_arg_size(uid, 5)) || access(uid_path, F_OK) == -1){
        send_udp("RGS E_USR\n");
        return;
    }

    //Check if the GID is well-formatted and is registered
    if (!(digits_only(gid,"gid") && is_correct_arg_size(gid, 2)) || atoi(gid) > *global_gid){
        send_udp("RGS E_GRP\n");
        return;
    }

    //Check if the group name is valid
    if (!(strlen(group_name) <= 24 && is_alphanumerical(group_name, 1))){
        send_udp("RGS E_GNAME\n");
        return;
    }

    //New group case
    if (!(strcmp(gid, "00"))){
        //Group database full
        if (*global_gid == 99){
            send_udp("RGS E_FULL\n");
            return;
        }
        ++(*global_gid);
        bzero(gid, 2);
        if (*global_gid < 10)
            sprintf(gid, "0%d", *global_gid);
        else
            sprintf(gid, "%d", *global_gid);
        new_group = true;
    }

    char path[10], msg_path[14], name_path[22];
    bzero(path, 10);
    sprintf(path,"GROUPS/%s",gid);

    bzero(msg_path, 14);
    sprintf(msg_path,"%s/MSG", path);
 
    bzero(name_path, 22);
    sprintf(name_path,"%s/%s_name.txt", path, gid);

    if (new_group){
        if (!access(path, F_OK) || mkdir(path, 0700) == -1 || !access(msg_path, F_OK) || mkdir(msg_path, 0700) == -1){
            send_udp("RGS NOK\n");
            return;
        }
        FILE* fp = fopen(name_path, "w");
        if (!fp){
            send_udp("RGS NOK\n");
            return;
        }
        fprintf(fp,"%s", group_name);
        fclose(fp);
        char message[12];
        sprintf(message, "RGS NEW %s\n", gid);
        send_udp(message);
    } else {
        if (access(path, F_OK) == -1 || access(msg_path, F_OK) == -1){
            send_udp("RGS NOK\n");
            return;
        }
        FILE* fp = fopen(name_path, "r");
        if (!fp){
            send_udp("RGS NOK\n");
            return;
        }
        char group_name_temp[25];
        fgets(group_name_temp,25,fp);
        fclose(fp);

        if (strcmp(group_name_temp,group_name)){
            send_udp("RGS NOK\n");
            return;
        }
        send_udp("RGS OK\n");
    }
}