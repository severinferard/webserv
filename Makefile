DOCKER_EXPOSED_PORTS = 8080 8081 80

_END=\x1b[0m
_BOLD=\x1b[1m
_UNDER=\x1b[4m
_REV=\x1b[7m

# Colors
_GREY=\x1b[30m
_RED=\x1b[31m
_GREEN=\x1b[32m
_YELLOW=\x1b[33m
_BLUE=\x1b[34m
_PURPLE=\x1b[35m
_CYAN=\x1b[36m
_WHITE=\x1b[37m
SRCDIR = src/
OBJDIR = .objs

SRCS = $(wildcard $(SRCDIR)*.cpp)

OBJS := $(SRCS:%.cpp=$(OBJDIR)/%.o)

NAME = webserv

CC = g++

CFLAGS = -Wall -Wextra -Werror -std=c++98 -fsanitize=address -Wno-unused-variable -Wno-unused-private-field 

RM = rm -f

RMDIR = rm -rf

DOCKER_IMAGE = gcc

DOCKER_CONTAINER_NAME = webserv-$(DOCKER_IMAGE)

DOCKER_EXPOSED_PORTS_CMD := $(foreach P, $(DOCKER_EXPOSED_PORTS), -p $P:$P)

ARGS = $(filter-out $@,$(MAKECMDGOALS))

ifeq ($(USE_DOCKER), 1)
	CONTEXT = docker run --name $(DOCKER_CONTAINER_NAME) -v $(shell pwd):/root/webserv -w /root/webserv $(DOCKER_EXPOSED_PORTS_CMD) --rm -i -t --init $(DOCKER_IMAGE)
endif

$(OBJDIR)/%.o: %.cpp
	
	@mkdir -p '$(@D)'
	$(CONTEXT) $(CC) -c $(CFLAGS) $< -o $@

all: $(NAME)

pre:
ifeq ($(USE_DOCKER), 1)
	@printf "$(_BLUE)Using Docker$(_END)\n"
else
	@printf "$(_RED)Using Native System$(_END)\n"
endif

$(NAME): | pre $(OBJS)
	$(CONTEXT) $(CC) $(OBJS) $(CFLAGS) $(LIBFT) -o $(NAME)
	
run: pre
	$(CONTEXT) ./webserv $(ARGS)

clean:
	$(RMDIR) $(OBJDIR)

fclean: clean
		$(RM) $(NAME)

re: fclean $(NAME)


.PHONY: all clean fclean re run