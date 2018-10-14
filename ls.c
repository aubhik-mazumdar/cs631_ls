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
#include <math.h>
#include "ls.h"
#include "print_function.h"
#include "cmp.h"
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

#define FTS_OPTIONS_A FTS_PHYSICAL | FTS_NOCHDIR | FTS_SEEDOT | FTS_WHITEOUT
#define FTS_OPTIONS FTS_PHYSICAL | FTS_NOCHDIR | FTS_WHITEOUT


static void usage(void);
int compare(const FTSENT**,const FTSENT**);

void
delete_list(node head){
    node temp;
    while(head != NULL){
        temp = head;
        head = head->next;
        free(temp);
    }
}


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
    if(getuid()==0)
        opts->_A = true;
    else
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
(*get_compare(struct opts_holder opts))(const FTSENT **,const FTSENT **){
    if(opts._f)
        return NULL;
    if(opts._t){
        if(opts._c){
            if(opts._r)
                return &rctime_cmp;
            else
                return &ctime_cmp;
        }
        if(opts._t){
            if(opts._r)
                return &rmtime_cmp;
            else
                return &mtime_cmp;
        }
        if(opts._u){
            if(opts._r)
                return &ratime_cmp;
            else
                return &atime_cmp;
        }
    }

    if(opts._S){
        if(opts._r)
            return &rsize_cmp;
        else
            return &size_cmp;
    }
    
    if(opts._r)
        return &rcompare;
    else
        return &compare;
    
}


void
maximize(FTSENT *file,int *maxArray,struct opts_holder opts){
    struct passwd *filePasswd;
    struct group *fileGroup;
    struct stat fileStat;
    int temp;
    char *uname;
    char *gname;
    char bytes[5];
    fileStat = *(file->fts_statp);

    if(opts._n){
        temp = floor(log10(fabs((int)fileStat.st_uid))) + 1;

        if(temp > maxArray[0]){
            maxArray[0] = temp;
        }

        temp = floor(log10(fabs((int)fileStat.st_gid))) + 1;
        if(temp > maxArray[1]){
            maxArray[1] = temp;
        }
    } else {
        if((filePasswd = getpwuid(fileStat.st_uid)) != NULL){
            uname = filePasswd->pw_name;
            if((int)strlen(uname)>maxArray[0]){
                maxArray[0] = (int)strlen(uname);
            }
        }

        if((fileGroup = getgrgid(fileStat.st_gid)) != NULL){
            gname = fileGroup->gr_name;
            if((int)strlen(gname) > maxArray[1]){
                maxArray[1] = (int)strlen(gname);
            }  
        }

    }

    if(opts._h){
        if(humanize_number(bytes,5,(int64_t)fileStat.st_size,"",HN_AUTOSCALE,HN_DECIMAL | HN_NOSPACE | HN_B)!=-1){
            if((int)strlen(bytes) > maxArray[2])
                maxArray[2] = (int)strlen(bytes);
        }
    } else {
        temp = floor(log10(fabs(fileStat.st_size))) + 1;
        if(temp > maxArray[2])
            maxArray[2] = temp;
    }    
    
    if(opts._h)
        maxArray[3] += fileStat.st_size;
    else
        maxArray[3] += fileStat.st_blocks;
    if(opts._s){
        if(opts._h){
            if(humanize_number(bytes,5,(int64_t)fileStat.st_size*512,"",HN_AUTOSCALE,HN_DECIMAL | HN_NOSPACE | HN_B)!= -1){
                if((int)strlen(bytes) > maxArray[4])
                    maxArray[4] = (int)strlen(bytes);
            }
        } else {
            temp = floor(log10(fabs(fileStat.st_blocks))) + 1;
            if(temp > maxArray[4]){
                maxArray[4] = temp;
            }
        }
    }
}

void
Rtraverse(struct opts_holder opts,char * const *dir_name){
    FTS* file_system = NULL;
    FTSENT* parent = NULL;
    FTSENT *child = NULL; 
    int (*comparefxn)(const FTSENT **,const FTSENT **);
    
    comparefxn = get_compare(opts);
    
    if(opts._a)
        file_system = fts_open(dir_name,FTS_OPTIONS_A ,comparefxn);
    else
        file_system = fts_open(dir_name,FTS_OPTIONS,comparefxn); 
    
    if (file_system != NULL)
    {
        while((parent = fts_read(file_system)))
        {
            switch(parent->fts_info){
                case FTS_DC:
                    fprintf(stderr,"directory is cyclic: %s\n",parent->fts_name);
                    break;
                case FTS_D:
                    if(opts._a && parent->fts_level != 0 && parent->fts_name[0] == '.')
                        break;

                    if(parent->fts_name[0] != '.' && strnlen(parent->fts_name,2)!=1)
                        printf("\n%s:\n",parent->fts_path);

                    node head= NULL; 
                    int max[5] = {0};
                    child = fts_children(file_system,0);
                    while(child != NULL){
                        if(!opts._A && !opts._a){
                            if(child->fts_name[0]=='.'){
                                child = child->fts_link;
                                continue;
                            }
                        }
                        head = addNode(head,child);
                        maximize(child,max,opts);
                        child = child->fts_link;
                    }
                    print_function(head,opts,max);
                    delete_list(head);
                    break;
                default:
                    break;
            }
        }
        fts_close(file_system);
    }
}

void
traverse(struct opts_holder opts,char * const *dir_name){
    FTS* file_system = NULL;
    FTSENT* child = NULL;
    FTSENT* parent = NULL;
    FTSENT *p = NULL;
    int (*comparefxn)(const FTSENT **,const FTSENT **);
    node head = NULL;
    comparefxn = get_compare(opts);
    int max[5] = {0};
    if(opts._a){
        if((file_system = fts_open(dir_name,FTS_OPTIONS_A,comparefxn)) == NULL){
            fprintf(stderr,"could not open %s:%s\n",*dir_name,strerror(errno));
            exit(EXIT_FAILURE);
        }
    } else {
        if((file_system = fts_open(dir_name,FTS_OPTIONS,comparefxn)) == NULL){
            fprintf(stderr,"could not open %s:%s\n",*dir_name,strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    if((parent = fts_read(file_system))==NULL){
        fprintf(stderr,"could not read directory %s:%s\n",*dir_name,strerror(errno));
    }

    if(parent->fts_info == FTS_F || opts._d){
        opts._d = true;
        head = addNode(head,parent);
        maximize(parent,max,opts);
        print_function(head,opts,max);
        exit(EXIT_SUCCESS);
    }


    if((child = fts_children(file_system, 0))==NULL){
        fprintf(stderr,"CHILD ERROR: %s",strerror(errno));
    }
    p = child;
    while(p){
        if(!opts._A && !opts._a){
            if(p->fts_name[0]=='.'){
                p = p->fts_link;
                continue;
            }
        }
        head = addNode(head,p);
        maximize(p,max,opts);
        p = p->fts_link;
    }

    print_function(head,opts,max);
    delete_list(head);
    
    if((fts_close(file_system)==-1)){
        fprintf(stderr,"Error while closing file system %s:%s\n",*dir_name,strerror(errno));
    }
}



void
start_scan(int arg_count,char * const *arg_vector,struct opts_holder opts){
    int idx;
    struct stat fileStat;
    char * const default_directory[] = { ".", NULL};
    if (arg_count==0){
        if(opts._R)
            Rtraverse(opts,default_directory);
        else
            traverse(opts,default_directory);
    } else if (arg_count==1){
        if(stat(arg_vector[0],&fileStat)==-1){
            fprintf(stderr,"%s: %s: %s\n",getprogname(),arg_vector[0],strerror(errno));
            exit(EXIT_FAILURE);
        } else{
            if(opts._R){
                Rtraverse(opts,&arg_vector[0]);
            }else
                traverse(opts,&arg_vector[0]);
        }
    } else {
        for(idx=0;idx<arg_count;idx++){
            if(stat(arg_vector[idx],&fileStat)==-1){
                fprintf(stderr,"%s: %s: %s\n",getprogname(),arg_vector[idx],strerror(errno));
                continue;
            } else {
                fprintf(stdout,"%s:\n",arg_vector[idx]);
                if(opts._R){
                    Rtraverse(opts,&arg_vector[idx]);
                } else 
                    traverse(opts,&arg_vector[idx]);
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
                opts._u = false;
                break;
            case 'C':
                opts._C = true;
                opts._1 = false;
                opts._l = false;
                opts._n = false;
                opts._x = false;
                break;
            case 'd':
                opts._d = true;
                break;
            case 'F':
                opts._F = true;
                break;
            case 'f':
                opts._f = true;
                opts._a = true;
                break;
            case 'h':
                opts._h = true;
                opts._k = false;
                break;
            case 'i':
                opts._i = true;
                break;
            case 'k':
                opts._k = true;
                opts._h = false;
                break;
            case 'l':
                opts._l = true;
                opts._1 = false;
                opts._C = false;
                opts._x = false;
                break;
            case 'n':
                opts._n = true;
                opts._l = true;
                opts._1 = false;
                opts._C = false;
                opts._x = false;
                break;
            case 'q':
                opts._q = true;
                opts._w = false;
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
                opts._c = false;
                break;
            case 'w':
                opts._w = true;
                opts._q = false;
                break;
            case 'x':
                opts._x = true;
                opts._1 = false;
                opts._C = false;
                opts._l = false;
                opts._n = false;
                break;
            case '1':
                opts._1 = true;
                opts._C = false;
                opts._l = false;
                opts._n = false;
                opts._x = false;
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
