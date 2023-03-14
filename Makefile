NAME = test_task

CFLAGS = -Wall -Wextra -Werror
CC = gcc

all: $(NAME)

$(NAME): folders_sync.o folders_sync_utils.o
	$(CC) -o $(NAME) folders_sync.o folders_sync_utils.o $(CFLAGS)

folders_sync.o: folders_sync.c 
	$(CC) -c folders_sync.c

folders_sync_utils.o: folders_sync_utils.c
	$(CC) -c folders_sync_utils.c

clean:
	rm $(NAME) folders_sync.o folders_sync_utils.o