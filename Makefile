NAME		= codexion

CC			= cc
CFLAGS		= -Wall -Wextra -Werror -pthread

SRCS_DIR	= srcs
INC_DIR		= includes

SRCS		= main.c \
			  parsing.c \
			  parsing_utils.c \
			  init.c \
			  init_control.c \
			  init_utils.c \
			  logging.c \
			  utils_time.c \
			  heap.c \
			  heap_utils.c \
			  scheduler.c \
			  scheduler_dongle.c \
			  scheduler_wait.c \
			  dongle.c \
			  coder.c \
			  coder_state.c \
			  coder_utils.c \
			  monitor.c

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