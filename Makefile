SRCDIR = src/
OBJDIR = .objs

SRCS = $(wildcard $(SRCDIR)*.cpp)

OBJS := $(SRCS:%.cpp=$(OBJDIR)/%.o)

DEPS = $(OBJS:%.o=%.d)

NAME = webserv

CC = c++

CFLAGS = -Wall -Wextra -Werror -std=c++98
RM = rm -f

RMDIR = rm -rf

ARGS = $(filter-out $@,$(MAKECMDGOALS))

YOUPIBANANE_ROOT = www/test/
YOUPIBANANE_F =  YoupiBanane/youpi.bad_extension YoupiBanane/youpi.bla  YoupiBanane/nop/youpi.bad_extension YoupiBanane/nop/other.pouic  YoupiBanane/Yeah/not_happy.bad_extension
YOUPIBANANE_D = YoupiBanane YoupiBanane/nop YoupiBanane/Yeah
YOUPIBANANE_FILES = $(addprefix  $(YOUPIBANANE_ROOT),$(YOUPIBANANE_F))
YOUPIBANANE_DIRS = $(addprefix  $(YOUPIBANANE_ROOT),$(YOUPIBANANE_D))

$(OBJDIR)/%.o: %.cpp
	@mkdir -p '$(@D)'
	$(CONTEXT) $(CC) -c $(CFLAGS) -MMD $< -o $@

all: $(NAME)

-include $(DEPS)

$(NAME): $(OBJS)
	$(CONTEXT) $(CC) $(OBJS) $(CFLAGS) $(LIBFT) -o $(NAME)
	
run: $(NAME)
	$(CONTEXT) ./webserv $(ARGS)

clean:
	$(RMDIR) $(OBJDIR)

fclean: clean
		$(RM) $(NAME)

re: fclean $(NAME)

test: $(NAME)
	pytest -v -x

create_youpibanane:
	mkdir -p $(YOUPIBANANE_DIRS)
	touch $(YOUPIBANANE_FILES)
clear_youibanane:
	rm -rf $(YOUPIBANANE_DIRS)

install_tests:
	@echo 'Run: "pip3 install -U pytest && export PATH=$$PATH:/mnt/nfs/homes/$$(whoami)/.local/bin && pip3 install pytest-timeout"'


.PHONY: all clean fclean re run test create_youpibanane clear_youibanane install_tests