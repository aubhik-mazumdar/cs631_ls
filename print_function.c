#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <fts.h>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "ls.h"
#include "print_function.h"

void
print_permissions(mode_t mode){
    char modeString[12];
    strmode(mode,modeString);
    printf("%s",modeString);
}


void print_no_of_links(nlink_t fileLinks){
    fprintf(stdout,"%2d ",fileLinks);	
}

void
print_owner(uid_t fileUserId,gid_t fileGroupId,struct opts_holder opts){
    struct passwd *filePasswd;
    struct group *fileGroup;
    char *uname;
    char *gname;
    if(opts._n){
        (void)printf("%-4d ",fileUserId);
        (void)printf(" %-4d",fileGroupId);
    } else {
        if((filePasswd = getpwuid(fileUserId)) == NULL){
            uname = "NO_USER";
        } else {
            uname = filePasswd->pw_name;
        }
        
        if((fileGroup = getgrgid(fileGroupId)) == NULL){
            gname = "NO_GROUP";
        } else {
            gname = fileGroup->gr_name;
        }
        
        (void)printf("%-11s%-6s",uname,gname);
    }
}

void
print_bytes(off_t fileSize,struct opts_holder opts){
    char bytes[5];
    int64_t actual_size = (int64_t)fileSize;
    if(opts._h){
        if(humanize_number(bytes,5,actual_size,"",HN_AUTOSCALE,HN_DECIMAL | HN_NOSPACE | HN_B)==-1){
            printf("%7ld ",fileSize);
        } else
            (void)printf("%5s ",bytes);
    } else {
        (void)printf("%7ld ",fileSize);
    }
};

void
print_time(FTSENT *file,struct opts_holder opts){
    struct stat fileStat;
    char *timeString;
    int i;
    fileStat = *(file->fts_statp);
    if(opts._c){
        //last changed time
        if((timeString = ctime(&fileStat.st_ctime)) == NULL){
            timeString = (char *)fileStat.st_ctime;
        };
    } else {
        timeString = ctime(&fileStat.st_mtime);
    }
    for(i=4;i<16;++i)
       putchar(timeString[i]);
    putchar(' ');
}


void
print_pathname(FTSENT *file,struct opts_holder opts){
    char modeString[12];
    struct stat fileStat;
    fileStat = *(file->fts_statp);
    printf("%s",file->fts_name);
    strmode(fileStat.st_mode,modeString);
    if(opts._F){
        switch((char)modeString[0]){
            case 'd': 
                putchar('/');
                break;
            case 'l': 
                putchar('@');
                break;
            case 'w': 
                putchar('%');
                break;
            case 's': 
                putchar('=');
                break;
            case 'p': 
                putchar('|');
                break;
            default:
                if(modeString[3]=='x' || modeString[6]=='x' || modeString[9]=='x')
                    putchar('*');
                break;
        }
        putchar('\n');
    } else {
        putchar('\n');
    } 
}

void
print_function(FTSENT *file,struct opts_holder opts){
    struct stat fileStat;
    fileStat = *(file->fts_statp);
    if(opts._l){
        print_permissions(fileStat.st_mode);
        print_no_of_links(fileStat.st_nlink);
        print_owner(fileStat.st_uid,fileStat.st_gid,opts);
        print_bytes(fileStat.st_size,opts);
        print_time(file,opts);
        print_pathname(file,opts);
    } else {
        print_pathname(file,opts);
    }
}



