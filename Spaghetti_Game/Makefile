OS		= $(shell uname)
ifeq ($(OS), Linux)
	MLXDIR	= ./minilibx-linux
else
    $(error Need to have Linux for playing)
endif
MLXNAME	= mlx

# Program for compiling C programs
CC		= cc
# Extra flags to give to the C preprocessor and programs that use it
CPPFLAGS= -I. -I$(MLXDIR)
# Extra flags to give to the processor C compiler
CFLAGS	= -g -Wall -Werror -Wextra
# Optimization flag to give to the processor C compiler
COPTFLAGS	= -O3
# Extra flags to give to compilers when they are supposed to invoke the linker, ‘ld’, such as-L. Libraries (-lfoo) should be added to the LDLIBS variable instead
LDFLAGS	= -L$(MLXDIR)
# Library flags or names given to compilers when they are supposed to invoke the linker, ‘ld’. LOADLIBES is a deprecated (but still supported) alternative to LDLIBS. Non-library linker flags, such as-L, should go in the LDFLAGS variable
LDLIBS	= -l$(MLXNAME) -lm -lXext -lX11

#
SRC		= main.c
OBJ		= $(SRC:.c=.o)
NAME	= "Sp(aghetti)lash"
#
.PHONY: all clean fclean cleanall re

.o: .c
	$(CC) -c $(CFLAGS) $(COPTFLAGS) $(CPPFLAGS) $< -o $@

all: lib_check $(NAME)

lib_check:
	@if [ ! -d $(MLXDIR) ]; then \
		tar -xzf $(MLXDIR).tgz; \
	fi

$(NAME): $(OBJ)
	make -C $(MLXDIR)
	$(CC) $(CFLAGS) $(COPTFLAGS) $(LDFLAGS) -o $@ $(OBJ) $(LDLIBS)

clean:
	rm -f *~ $(OBJ)
	make clean -C $(MLXDIR)

start:
	valgrind --leak-check=full --track-origins=yes ./Sp\(aghetti\)lash maps/test.ber

fclean: clean
	rm -f $(NAME)

cleanall: fclean
	rm -fr $(MLXDIR)

re: fclean all