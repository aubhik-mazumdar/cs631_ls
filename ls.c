#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <fts.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sysexits.h>
#include <unistd.h>
#include "ls.h"
#include "print_function.h"
/* ls [ -AacCdFfhiklnqRrSstuwx1] */
/*
 * PRINT OPTIONS
 *  -C -> multi column
 *  -F -> Display / after directory, * after executable, @ after symbolic link, % after whiteout, = after socket, | after FIFO
 *  -s -> size displayed in human readable format with -s and -l, overrides -k. If both -k and -h are specified, the rightmost is considered
 *  -l -> display long format
 *  -n -> group and owner IDs are displayed numerically
 *  -q -> non-printable characters as ?
 *  LOGIC OPTIONS
 *  -c -> use time when file status was last changed for sorting and printing
 *  -f -> not sorted
 *    
 */
static void usage(void);
int compare(const FTSENT**,const FTSENT**);


static void
initialize(struct opts_holder *opts){
    opts->_A = false;
    opts->_a = false;
    opts->_c = false;
    if(isatty(1)){
        opts->_C = true;
        opts->_q = true;
        opts->_w = false;
        opts->_1 = false;
    } else {
        opts->_C = false;
        opts->_q = false;
        opts->_w = true;
        opts->_1 = true;
    }	
    opts->_d = false;
    opts->_F = false;
    opts->_f = false;
    opts->_h = false;
    opts->_i = false;
    opts->_k = false;
    opts->_l = false;
    opts->_n = false;
    opts->_R = false;
    opts->_r = false;
    opts->_S = false;
    opts->_s = false;
    opts->_t = false;
    opts->_u = false;
    opts->_x = false;
}

int
compare(const FTSENT** first,const FTSENT** second){
    return (strcmp((*first)->fts_name,(*second)->fts_name));
}



void
traverse(struct opts_holder opts,char * const *dir_name){
    FTS* file_system = NULL;
    FTSENT* child = NULL;
    FTSENT* parent = NULL;
    if(opts._a){
        if((file_system = fts_open(dir_name,FTS_SEEDOT | FTS_LOGICAL | FTS_NOCHDIR,&compare)) == NULL){
            fprintf(stderr,"could not open %s:%s\n",*dir_name,strerror(errno));
            exit(EXIT_FAILURE);
        }
    } else if(opts._A){
        if((file_system = fts_open(dir_name,FTS_LOGICAL | FTS_NOCHDIR,&compare)) == NULL){
            fprintf(stderr,"could not open %s:%s\n",*dir_name,strerror(errno));
            exit(EXIT_FAILURE);
        }
    } else {
        if((file_system = fts_open(dir_name,FTS_LOGICAL | FTS_NOCHDIR,&compare)) == NULL){
            fprintf(stderr,"could not open %s:%s\n",*dir_name,strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    if((parent = fts_read(file_system))==NULL){
        fprintf(stderr,"could not read directory %s:%s\n",*dir_name,strerror(errno));
    }

    if(parent->fts_info == FTS_F){
        print_function(parent,opts);
        exit(EXIT_SUCCESS);
    }
    /*add error handling*/
    if((child = fts_children(file_system, 0))==NULL){
        fprintf(stderr,"CHILD ERROR: %s",strerror(errno));
    };


    while(child && child->fts_info){
        if(!opts._a && (child->fts_info == FTS_D) && (child->fts_name[0]=='.'))
            child = child->fts_link;
        print_function(child,opts);		
        child = child->fts_link;
    }		

    if((fts_close(file_system)==-1)){
        fprintf(stderr,"Error while closing file system %s:%s\n",*dir_name,strerror(errno));
    }
}



void
start_scan(int arg_count,char * const *arg_vector,struct opts_holder opts){
    if (arg_count==0){
        char * const default_directory[] = { ".", NULL};
        traverse(opts,default_directory);
    } else if (arg_count==1){
        traverse(opts,&arg_vector[0]);
    } else {
        int idx=0;	
        for(idx=0;idx<arg_count;idx++){
            fprintf(stdout,"%s:\n",arg_vector[idx]);
            traverse(opts,&arg_vector[arg_count]);
        }
    }
}




int
main(int argc, char * const *argv){
    int c;
    struct opts_holder opts; 
    initialize(&opts);
    setprogname(argv[0]);
    while((c = getopt (argc,argv, "AacCdFfhiklnqRrSstuwx1")) != -1)
        switch(c)
        {
            case 'A':
                opts._A = true;
                break;
            case 'a':
                opts._a = true;
                break;
            case 'c':
                opts._c = true;
                break;
            case 'C':
                opts._C = true;
                break;
            case 'd':
                opts._d = true;
                break;
            case 'F':
                opts._F = true;
                break;
            case 'f':
                opts._f = true;
                break;
            case 'h':
                opts._h = true;
                break;
            case 'i':
                opts._i = true;
                break;
            case 'k':
                opts._k = true;
                break;
            case 'l':
                opts._l = true;
                break;
            case 'n':
                opts._n = true;
                break;
            case 'q':
                opts._q = true;
                break;
            case 'R':
                opts._R = true;
                break;
            case 'r':
                opts._r = true;
                break;
            case 'S':
                opts._S = true;
                break;
            case 's':
                opts._s = true;
                break;
            case 't':
                opts._t = true;
                break;
            case 'u':
                opts._u = true;
                break;
            case 'w':
                opts._w = true;
                break;
            case 'x':
                opts._x = true;
                break;
            case '1':
                opts._1 = true;
                break;
            case '?':
            default:
                usage();
        }
    argc -= optind;
    argv += optind;
    start_scan(argc,argv,opts);
    exit(EXIT_SUCCESS);
}


static void
usage(void){
    (void)fprintf(stderr,"usage: %s [-AacCdFfhiklnqRrSstuwx1] [file ...]\n",getprogname()); 
    exit(EX_USAGE);
}

