#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <fts.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

struct ll {
    FTSENT *data;
    struct ll *next;
};

typedef struct ll *node;

node 
create(){
    node temp_node;
    temp_node = (node)malloc(sizeof(struct ll));
    temp_node->next = NULL;
    return temp_node;
}

node 
addNode(node head, FTSENT *value){
    node temp,p;
    temp = create();
    temp->data = value;
    if(head == NULL)
        head = temp;  
    else{
        p  = head;
        while(p->next != NULL){
            p = p->next;
        }
        p->next = temp;
    }
    return head;
}

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
maximize(FTSENT *file,int *maxArray){
    struct passwd *filePasswd;
    struct group *fileGroup;
    struct stat fileStat;
    char *uname;
    char *gname;
    fileStat = *(file->fts_statp);
    if((filePasswd = getpwuid(fileStat.st_uid)) != NULL){
        uname = filePasswd->pw_name;
        if((int)strlen(uname)>maxArray[0]){
            maxArray[0] = (int)strlen(uname);
        }
    }
    
    if((fileGroup = getgrgid(fileStat.st_gid)) != NULL){
        gname = fileGroup->gr_name;
        if((int)strlen(gname)>maxArray[1]){
            maxArray[0] = (int)strlen(gname);
        }  
    }
    
    if(fileStat.st_size>maxArray[2])
        maxArray[2] = (int)fileStat.st_size;

    if((int)strlen(file->fts_name)>maxArray[3]){
        maxArray[3] = (int)strlen(file->fts_name);
    }
}



void
traverse(struct opts_holder opts,char * const *dir_name){
    FTS* file_system = NULL;
    FTSENT* child = NULL;
    FTSENT* parent = NULL;
    FTSENT *temp = NULL;
    int max[4] = {0};
    struct stat readStat;
    if(opts._a){
        if((file_system = fts_open(dir_name,FTS_SEEDOT | FTS_LOGICAL | FTS_NOCHDIR,&compare)) == NULL){
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
    if(opts._l){
        if(stat(*dir_name,&readStat)==-1){
            fprintf(stderr,"stat error on %s: %s\n",*dir_name,strerror(errno));
            exit(EXIT_FAILURE);
        } else {
            printf("total %ld\n",readStat.st_blocks);
        }

    }
    if(parent->fts_info == FTS_F){
        print_function(parent,opts);
        exit(EXIT_SUCCESS);
    }
    /*add error handling*/
    if((child = fts_children(file_system, 0))==NULL){
        fprintf(stderr,"CHILD ERROR: %s",strerror(errno));
    };

    node head = NULL;
    while(child && child->fts_info){
        if(!opts._a && (child->fts_info == FTS_D) && (strnlen(child->fts_name,2)==1 && child->fts_name[0]=='.')){
            child = child->fts_link;
            continue;
        }
        head = addNode(head,child);
        child = child->fts_link;
    }
    while(head != NULL){
        temp = head->data;
        maximize(temp,max);
        printf("%s\n",temp->fts_name);
        head = head->next;
    }
    int i;
    for(i=0;i<4;i++)
        printf("%d\n",max[i]);

    if((fts_close(file_system)==-1)){
        fprintf(stderr,"Error while closing file system %s:%s\n",*dir_name,strerror(errno));
    }
}



void
start_scan(int arg_count,char * const *arg_vector,struct opts_holder opts){
    int idx;
    struct stat fileStat;
    if (arg_count==0){
        char * const default_directory[] = { ".", NULL};
        traverse(opts,default_directory);
    } else if (arg_count==1){
        if(stat(arg_vector[0],&fileStat)==-1){
            fprintf(stderr,"%s: %s: %s\n",getprogname(),arg_vector[0],strerror(errno));
            exit(EXIT_FAILURE);
        } else 
            traverse(opts,&arg_vector[0]);
    } else {
        for(idx=0;idx<arg_count;idx++){
            if(stat(arg_vector[idx],&fileStat)==-1){
                fprintf(stderr,"%s: %s: %s\n",getprogname(),arg_vector[idx],strerror(errno));
                continue;
            } else {
                fprintf(stdout,"%s:\n",arg_vector[idx]);
                traverse(opts,&arg_vector[arg_count]);
            }
        }
    }
}




int
main(int argc, char **argv){
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
