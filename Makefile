NAME = codexion
CC = gcc
CFLAGS = -Wall -Wextra -Werror -pthread
INCLUDES = -Iheaders
SRCS = 	src/heap/create.c \
		src/heap/heap_utils.c \
		src/threads/display.c \
		src/threads/init.c \
		src/threads/monitor.c \
		src/threads/routine.c \
		src/threads/scheduling_utils.c \
		src/threads/scheduling.c \
		src/threads/time.c \
		src/threads/utils.c \
		src/free.c \
		src/main.c \
		src/parsing.c
OBJS = ${SRCS:.c=.o}

all: ${NAME}

${NAME}: ${OBJS}
	$(CC) $(OBJS) -o $(NAME)
%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@
clean:
	rm -f ${OBJS}
fclean: clean
	rm -f ${NAME}
re: fclean all

.PHONY: all clean fclean re