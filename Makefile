SRC_FILES =               \
	olcPGE_min_x11_gdi.c  \
	tut.c

all:

	gcc -Werror -Wall -Wno-unused $(SRC_FILES) -lX11 -lGL -lpthread -o bin/main.e


# TODO: run valgrind on all exe's to check no leaks




#	Linux:
#
#		gcc $(SRC_FILES) -lX11 -lGL -lpthread -o bin/main.e
#
#	Windows (VS command prompt):
#
#		cd binWin
#		cl ../$(SRC_FILES)
