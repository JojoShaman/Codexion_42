NAME = codexion
CC = gcc
CFLAGS = -Wall -Wextra -Werror -pthread
INCLUDES = -Iheaders
SRCS = 	src/heap/create.c \
		src/heap/heap_utils.c \
		src/threads/init.c \
		src/free.c \
		src/logs.c \
		src/main.c \
		src/parsing.c \
		src/simulation.c \
		src/utils.c
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