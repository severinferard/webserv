SRCDIR = src/
OBJDIR = .objs

SRCS = $(wildcard $(SRCDIR)*.cpp)

OBJS := $(SRCS:%.cpp=$(OBJDIR)/%.o)

DEPS = $(OBJS:%.o=%.d)

NAME = webserv

CC = g++

CFLAGS = -Wall -Wextra -Werror -std=c++98 -fsanitize=address -Wno-unused-variable -Wno-unused-private-field -Wno-unused-parameter

RM = rm -f

RMDIR = rm -rf

ARGS = $(filter-out $@,$(MAKECMDGOALS))

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


$(OBJDIR)/%.o: %.cpp
	@mkdir -p '$(@D)'
	$(CONTEXT) $(CC) -c $(CFLAGS) -MMD $< -o $@

all: $(NAME)

-include $(DEPS)

test:
	echo $(DEP)

$(NAME): $(OBJS)
	$(CONTEXT) $(CC) $(OBJS) $(CFLAGS) $(LIBFT) -o $(NAME)
	
run: $(NAME)
	$(CONTEXT) ./webserv $(ARGS)

clean:
	$(RMDIR) $(OBJDIR)

fclean: clean
		$(RM) $(NAME)

re: fclean $(NAME)

.PHONY: all clean fclean re run