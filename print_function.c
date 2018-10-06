#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include "ls.h"
#include "print_function.h"

void
print_permissions(mode_t mode){
    putchar((mode & S_IRUSR) ? 'r' : '-');
    putchar((mode & S_IWUSR) ? 'w' : '-');
    putchar((mode & S_IXUSR) ? 'x' : '-');
    putchar((mode & S_IRGRP) ? 'r' : '-');
    putchar((mode & S_IWGRP) ? 'w' : '-');
    putchar((mode & S_IXGRP) ? 'x' : '-');
    putchar((mode & S_IROTH) ? 'r' : '-');
    putchar((mode & S_IWOTH) ? 'w' : '-');
    putchar((mode & S_IXOTH) ? 'x' : '-');	
}

void
print_file_mode(FTSENT *file){
    struct stat fileStat;
    fileStat = *(file->fts_statp);
    switch (fileStat.st_mode & S_IFMT)
    {
        case S_IFREG: putchar('-'); break;
        case S_IFDIR: putchar('d'); break;
        case S_IFLNK: putchar('l'); break;
        case S_IFCHR: putchar('c'); break;
        case S_IFBLK: putchar('b'); break;
        case S_IFSOCK: putchar('s'); break;
        case S_IFIFO: putchar('f'); break;
    }

    switch(file->fts_info){
        case FTS_W: putchar('w');break;
    }
    print_permissions(fileStat.st_mode);
}

void print_no_of_links(nlink_t fileLinks){
    fprintf(stdout," %2d ",fileLinks);	
}

void
print_owner(uid_t fileUserId,gid_t fileGroupId,struct opts_holder opts){
    struct passwd *filePasswd;
    struct group *fileGroup;
    char *uname;
    char *gname;
    if(opts._n){
        (void)printf(" %d ",fileUserId);
        (void)printf(" %d ",fileGroupId);
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

/* taken from: https://gist.github.com/dgoguerra/7194777 */
char *
human_readable(off_t bytes){
    char *suffix[] = {"B", "KB", "MB", "GB", "TB"};
    char length = sizeof(suffix) / sizeof(suffix[0]);

    int i = 0;
    double dblBytes = bytes;

    if (bytes > 1024) {
        for (i = 0; (bytes / 1024) > 0 && i<length-1; i++, bytes /= 1024)
            dblBytes = bytes / 1024.0;

    }
    if(i==0){
        char *output = malloc(sizeof(char)*200);
        (void)sprintf(output, "%d%s", (int)dblBytes, suffix[i]);
        return output;
    } else {
        char *output = malloc(sizeof(char)*200);
        (void)sprintf(output, "%.01lf%s", dblBytes, suffix[i]);
        return output;

    }
}

void
print_bytes(off_t fileSize,struct opts_holder opts){
    char * bytes;
    if(opts._h){
        bytes = human_readable(fileSize);
        (void)printf("%7s ",bytes);
    } else {
        (void)printf("%7d ",fileSize);
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
        timeString = ctime(&fileStat.st_ctime);
    } else {
        timeString = ctime(&fileStat.st_mtime);
    }
    for(i=4;i<16;++i)
       putchar(timeString[i]);
    putchar(' ');
}


void
print_pathname(FTSENT *file){
    printf("%-20s\n",file->fts_name);
};

void
print_function(FTSENT *file,struct opts_holder opts){
    struct stat fileStat;
    fileStat = *(file->fts_statp);
    if(opts._l){
        print_file_mode(file);
        print_no_of_links(fileStat.st_nlink);
        print_owner(fileStat.st_uid,fileStat.st_gid,opts);
        print_bytes(fileStat.st_size,opts);
        print_time(file,opts);
        print_pathname(file);
    } else {
        printf("%-s\n",file->fts_name);
    }
}



