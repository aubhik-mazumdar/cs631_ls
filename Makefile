CFLAGS=-ansi -g -Wall -Werror -Wextra -Wformat=2 -Wjump-misses-init -Wlogical-op -Wpedantic -Wshadow

ls: ls.c
	cc $(CFLAGS) ls.c -o ls
