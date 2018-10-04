#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
void
print_function(FTSENT* file, struct opts_holder *opts){
        printf("PRINT FUNCTION CALLED");
	/*if(opts._l){
                 print_file_mode(file);
                 print_no_of_links();
                 print_owner();
                 print_bytes();
                 print_time();
                 print_pathname();
         }*/
}

