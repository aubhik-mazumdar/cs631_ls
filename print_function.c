#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
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

void print_no_of_links(FTSENT *file){
    struct stat fileStat;
    fileStat = *(file->fts_statp);
    fprintf(stdout," %2d ",fileStat.st_nlink);	
}

void
print_owner(FTSENT *file,struct opts_holder opts){
    struct stat fileStat;
    struct passwd *filePasswd;
    struct group *fileGroup;
    char *uname;
    char *gname;
    fileStat = *(file->fts_statp);
    if(opts._n){
        (void)printf(" %d ",fileStat.st_uid);
        (void)printf(" %d ",fileStat.st_gid);
    } else {
        if((filePasswd = getpwuid(fileStat.st_uid)) == NULL){
            uname = "NO_USER";
        } else {
            uname = filePasswd->pw_name;
        }
        
        if((fileGroup = getgrgid(fileStat.st_gid)) == NULL){
            gname = "NO_GROUP";
        } else {
            gname = fileGroup->gr_name;
        }
        
        (void)printf("%-11s %-6s",uname,gname);
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

    char *output = malloc(sizeof(char)*200);
    (void)sprintf(output, "%.01lf%s", dblBytes, suffix[i]);
    return output;
}

void
print_bytes(FTSENT *file,struct opts_holder opts){
    struct stat fileStat;
    char * bytes;
    fileStat = *(file->fts_statp);
    if(opts._h){
        bytes = human_readable(fileStat.st_size);
        (void)printf("%7s ",bytes);
    } else {
        (void)printf("%7s ",bytes);
    }
};

void
print_time(FTSENT *file,struct opts_holder opts){};

void
print_pathname(FTSENT *file){
    printf("%-20s\n",file->fts_name);
};

void
print_function(FTSENT *file,struct opts_holder opts){
    if(opts._l){
        print_file_mode(file);
        print_no_of_links(file);
        print_owner(file,opts);
        print_bytes(file,opts);
        print_time(file,opts);
        print_pathname(file);
    } else {
        printf("%-s\n",file->fts_name);
    }
}



