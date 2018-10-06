OG=ls
OBJS=ls.o print_function.o
CFLAGS=-Wall -Werror -Wall -Werror -Wextra
CC=cc

ls: print_function.o ls.o
	$(CC) $(CFLAGS) ls.c -o ls
