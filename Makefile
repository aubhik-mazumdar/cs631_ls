OG=ls
OBJS=ls.o print_function.o
CFLAGS=-Wall -Werror -Wall -Werror -Wextra -lm -g
CC=cc

$(OG): $(OBJS)
	$(CC) $(CFLAGS) print_function.c ls.c -o ls
