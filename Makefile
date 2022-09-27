DOCKER_EXPOSED_PORTS = 8080 8081 80

SRCDIR = src/
OBJDIR = .objs

SRCS = $(wildcard $(SRCDIR)*.cpp)

OBJS := $(SRCS:%.cpp=$(OBJDIR)/%.o)

NAME = webserv

CC = g++

CFLAGS = -Wall -Wextra -Werror -std=c++98 -fsanitize=address -Wno-unused-variable -Wno-unused-private-field -Wno-unused-parameter

RM = rm -f

RMDIR = rm -rf

DOCKER_IMAGE = gcc

DOCKER_CONTAINER_NAME = webserv-$(DOCKER_IMAGE)

DOCKER_EXPOSED_PORTS_CMD := $(foreach p, $(DOCKER_EXPOSED_PORTS), -p $p:$p )

ARGS = $(filter-out $@,$(MAKECMDGOALS))

PRINT_PRE_BUILD = 

PRINT_PRE_RUN =

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

ifeq ($(USE_DOCKER), 1)
	CONTEXT = docker run --name $(DOCKER_CONTAINER_NAME) -v $(shell pwd):/root/webserv -w /root/webserv $(DOCKER_EXPOSED_PORTS_CMD) --rm -i -t --init $(DOCKER_IMAGE)
endif

$(OBJDIR)/%.o: %.cpp
	@$(or $(PRINT_PRE_BUILD),$(eval PRINT_PRE_BUILD := :)$(MAKE) pre-build)
	@mkdir -p '$(@D)'
	$(CONTEXT) $(CC) -c $(CFLAGS) $< -o $@

all: $(NAME)

pre-build:
ifeq ($(USE_DOCKER), 1)
	@printf "$(_BLUE)Building using Docker$(_END)\n"
else
	@printf "$(_RED)Builing using Native System$(_END)\n"
endif

pre-run:
ifeq ($(USE_DOCKER), 1)
	@printf "$(_BLUE)$(_BOLD)Running using Docker$(_END)\n"
	@printf "$(_UNDER)Opened ports:$(_END)\n"
	@$(foreach p, $(DOCKER_EXPOSED_PORTS), echo '\t- $p';)
else
	@printf "$(_RED)Running using Native System$(_END)\n"
endif

$(NAME): $(OBJS)
	$(CONTEXT) $(CC) $(OBJS) $(CFLAGS) $(LIBFT) -o $(NAME)
	
run: $(NAME) pre-run
	$(CONTEXT) ./webserv $(ARGS)

clean:
	$(RMDIR) $(OBJDIR)

fclean: clean
		$(RM) $(NAME)

re: fclean $(NAME)

.PHONY: all clean fclean re run