NAME=status
SOURCE=main.c
CFLAGS=-O4 -s -lX11
PREFIX=/usr/local

build:
	cc $(SOURCE) -o $(NAME) $(CFLAGS)

install:
	cp "$(NAME)" "$(PREFIX)/bin/$(NAME)"

uninstall:
	rm -f "$(PREFIX)/bin/$(NAME)"
