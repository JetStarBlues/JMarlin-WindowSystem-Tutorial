CFLAGS = -Werror -Wall -Wno-unused
CFLAGS += -g  # add debug symbols
LIBS   = -lX11 -lGL -lpthread

SRC_FILES =               \
	olcPGE_min_x11_gdi.c  \
	widgets.c             \
	tut.c                 \
	demoBasic.c

all:

	gcc $(CFLAGS) $(SRC_FILES) $(LIBS) -o bin/main.e

	objdump -S -M intel bin/main.e > bin/main.asm



# TODO: run valgrind on all exe's to check no leaks




#	Linux:
#
#		gcc $(CFLAGS) $(SRC_FILES) $(LIBS) -o bin/main.e
#
#	Windows (VS command prompt):
#
#		cd binWin
#		cl ../$(SRC_FILES) /Fe"main.exe"
