#include "server.h"
#include "dirent.h"
#include "../common.h"

int ulist(int conn_fd){
    char gid[3];
    char enter[2];
    char gid_path[10];
    bzero(gid, sizeof(gid));
    bzero(enter, sizeof(enter));
    bzero(gid_path, sizeof(gid_path));
    ssize_t nread;
    nread = recv_tcp(conn_fd, gid, 2);
    if (nread == -1)
        return -1;
    nread = recv_tcp(conn_fd, enter, 1);
    if (nread == -1)
        return -1;
    
        //printf("%s",gid);
        //printf("%s",enter);     //imprime o resto caso verbose 
    
    sprintf(gid_path,"GROUPS/%s",gid);    
    if (!digits_only(gid, "gid") && access(gid_path, F_OK) == -1)
        return send_tcp(conn_fd, "RUL NOK\n", 8);
    
    //sends all the subscribed users
    DIR *d;
    struct dirent *dir;
    d = opendir(gid_path);
    FILE *fp;

    char name_file[12];
    char uid_path[20];
    char uid_temp[7];
    char uid[7];
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
    nread = send_tcp(conn_fd, send_status, sizeof(send_status));
    if (nread == -1)
        return -1;

    if (d){
        while ((dir = readdir(d)) != NULL){

            bzero(uid,7);
            bzero(uid_temp,7);
            bzero(uid_path,20);
            if( !strcmp(dir -> d_name,".") || !strcmp(dir -> d_name, "..") || !strcmp(dir -> d_name, "MSG") || !strcmp(dir -> d_name, name_file))
                continue;
            
            sprintf(uid_path, "GROUPS/%s/%s", gid, dir -> d_name);
            fp = fopen(uid_path,"r");
            fgets(uid_temp, 6, fp);
            sprintf(uid," %s",uid_temp);
            fclose(fp);
            nread = send_tcp(conn_fd, uid, 6);
            if (nread == -1)
                return -1;
        }
        nread = send_tcp(conn_fd, "\n", 1);
        if (nread == -1)
            return -1;
    }
    return 1;
}