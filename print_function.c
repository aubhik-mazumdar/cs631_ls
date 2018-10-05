#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
#include <stdio.h>
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
    fprintf(stdout," %d ",fileStat.st_nlink);	
}

void
print_owner(FTSENT *file,struct opts_holder opts){
}

void
print_bytes(FTSENT *file,struct opts_holder opts){};

void
print_time(FTSENT *file,struct opts_holder opts){};

void
print_pathname(FTSENT *file){};

void
print_function(FTSENT *file,struct opts_holder opts){
    if(opts._l){
        //long format
        print_file_mode(file);
        print_no_of_links(file);
        print_owner(file,opts);
        print_bytes(file,opts);
        print_time(file,opts);
        print_pathname(file);
        fprintf(stdout,"%s\n",file->fts_name);
    } else {
        fprintf(stdout,"%s\n",file->fts_name);
    }
}



