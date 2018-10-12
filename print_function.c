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
    printf("%2d ",fileLinks);	
}

void
print_owner(uid_t fileUserId,gid_t fileGroupId,struct opts_holder opts,int *max){
    struct passwd *filePasswd = NULL;
    struct group *fileGroup = NULL;
    int userLen;
    int groupLen;
    userLen = max[0] + 1;
    groupLen = max[1] + 1;
    if(opts._n){
        (void)printf("%-*d ",userLen,fileUserId);
        (void)printf("%-*d",groupLen,fileGroupId);
    } else {
        if((filePasswd = getpwuid(fileUserId)) != NULL){
            (void)printf("%-*s ",userLen,filePasswd->pw_name);
        } else {
            (void)printf("%-*d ",userLen,fileUserId);
        }
        
        if((fileGroup = getgrgid(fileGroupId)) !=  NULL){
            (void)printf("%-*s",groupLen,fileGroup->gr_name);
        } else {
            (void)printf("%-*d",groupLen,fileGroupId);
        }
    }
}

void
print_bytes(off_t fileSize,struct opts_holder opts,int *max){
    char bytes[5];
    int64_t actual_size = (int64_t)fileSize;
    int maxBytesLength;
    maxBytesLength = max[2] + 1;
    if(opts._h){
        if(humanize_number(bytes,5,actual_size,"",HN_AUTOSCALE,HN_DECIMAL | HN_NOSPACE | HN_B)==-1){
            printf("%*ld ",maxBytesLength,fileSize);
        } else
            (void)printf("%*s ",maxBytesLength,bytes);
    } else {
        (void)printf("%*ld ",maxBytesLength,fileSize);
    }
}

void
print_time(FTSENT *file,struct opts_holder opts){
    struct stat fileStat;
    char *timeString;
    int i;
    fileStat = *(file->fts_statp);
    if(opts._c){
        if((timeString = ctime(&fileStat.st_ctime)) == NULL){
            timeString = (char *)fileStat.st_ctime;
        }
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
    int i;
    int namelen;
    char * name;
    struct stat fileStat;
    name = file->fts_name;
    fileStat = *(file->fts_statp);
    namelen = (int)file->fts_namelen;
    if(opts._q){
        for(i=0;i<namelen;i++){
            if(isprint(name[i])==0)
                putchar('?');
            else
                putchar(name[i]);
        }
    }

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
print_function(node head,struct opts_holder opts,int *max){
    struct stat fileStat;
    FTSENT *file;
    while(head != NULL){
        file = head->data;
        if(file->fts_statp){
            fileStat = *(file->fts_statp);
        } else {
            if(stat(file->fts_name,&fileStat) == -1){
                head = head->next;
                continue;
            }
        }       
        if(opts._i){
            printf("%ld ",fileStat.st_ino);
        }
        if(opts._l){
            print_permissions(fileStat.st_mode);
            print_no_of_links(fileStat.st_nlink);
            print_owner(fileStat.st_uid,fileStat.st_gid,opts,max);
            print_bytes(fileStat.st_size,opts,max);
            print_time(file,opts);
            print_pathname(file,opts);
        } else {
            print_pathname(file,opts);
        }
        head = head->next;
    }
}
