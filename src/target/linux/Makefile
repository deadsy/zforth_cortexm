TOP = ../../..

OUTPUT	:= zforth

SRC	:= main.c
SRC += ../../zforth/zforth.c

OBJ = $(patsubst %.c, %.o, $(SRC))

CC := gcc

CFLAGS += -I. -I../../zforth
CFLAGS += -Wall -Wextra
CFLAGS += -Os

#LIBS = -lm

.c.o:
	$(CC) $(INCLUDE) $(DEFINE) $(CFLAGS) -c $< -o $@

.PHONY: all clean

all: $(OBJ)
	$(CC) $(LDFLAGS) -o $(OUTPUT) $(OBJ) $(LIBS)

clean:
	-rm -f $(OUTPUT) $(OBJ)
	-rm -f zforth.save
