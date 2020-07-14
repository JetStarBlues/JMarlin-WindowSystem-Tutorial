CFLAGS = -Werror -Wall -Wno-unused
LIBS   = -lX11 -lGL -lpthread

SRC_FILES =               \
	olcPGE_min_x11_gdi.c  \
	tut.c

all:

	gcc $(CFLAGS) $(SRC_FILES) $(LIBS) -o bin/main.e


# TODO: run valgrind on all exe's to check no leaks




#	Linux:
#
#		gcc $(CFLAGS) $(SRC_FILES) $(LIBS) -o bin/main.e
#
#	Windows (VS command prompt):
#
#		cd binWin
#		cl ../$(SRC_FILES)
