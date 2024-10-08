NAME=status
SOURCE=main.c
CFLAGS=-O4 -lX11
DEBUGFLAGS=-g -fsanitize=address -fno-omit-frame-pointer -DDEBUG
PREFIX=/usr/local

build:
	cc $(SOURCE) -o $(NAME) $(CFLAGS)

debug:
	cc $(SOURCE) -o $(NAME)-debug $(CFLAGS) $(DEBUGFLAGS)

install:
	cp "$(NAME)" "$(PREFIX)/bin/$(NAME)"

uninstall:
	rm -f "$(PREFIX)/bin/$(NAME)"
