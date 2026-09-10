NAME		= codexion

CC			= cc
CFLAGS		= -Wall -Wextra -Werror -pthread

SRCS_DIR	= srcs
INC_DIR		= includes

SRCS		= main.c \
			  parsing.c \
			  init.c \
			  logging.c \
			  utils_time.c

OBJS		= $(SRCS:%.c=$(SRCS_DIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(SRCS_DIR)/%.o: $(SRCS_DIR)/%.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re