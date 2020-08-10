TOP = ../../..

OUTPUT	:= jforth

SRC	:= jforth.c

OBJ = $(patsubst %.c, %.o, $(SRC))

CC := gcc

CFLAGS = -Wall -Wstrict-prototypes -Wextra -Werror
CFLAGS += -std=c99
CFLAGS += -Os

INCLUDE = -I.

DEFINE =

.c.o:
	$(CC) $(INCLUDE) $(DEFINE) $(CFLAGS) -c $< -o $@

.PHONY: all clean

all: $(OBJ)

clean:
	-rm -f $(OBJ)
