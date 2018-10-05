OG=ls
OBJS=cmp.o ls.o main.o print.o stat_flags.		o util.o

# If commented out, defaults will be used. If uncommented, these values
# # will be used, even though there is no other explicit mention of this
# # variable in this Makefile.
# #CFLAGS=-Wall -g
#
# all: ${PROG}
#
# ${PROG}:	 ${OBJS}
# 	@echo $@ depends on $?
# 		${CC} ${LDFLAGS} ${OBJS} -o ${PROG}
#
# 		clean:
# 			rm -f ls *.o
CC=cc

ls: print_function.o ls.o
	$(CC) $(CFLAGS) ls.c -o ls
