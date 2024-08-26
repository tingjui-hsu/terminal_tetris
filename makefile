NAME    = tetris
SRC     = $(NAME).c

CC      = gcc
# CFLAGS  = -O0 -g
# LDFLAGS = -lncurses

.PHONY: all clean

all: $(NAME)

$(NAME): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -rf $(NAME)
