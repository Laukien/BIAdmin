GCC := gcc
#CFLAGS += -ggdb3
CFLAGS += -static -s
CFLAGS += -O3
CFLAGS += -Wall -Werror -W
CFLAGS += -pedantic
CFLAGS += -I /usr/local/include
LDFLAGS += src/command.c
LDFLAGS += src/config.c
LDFLAGS += src/show.c
LDFLAGS += src/system.c
NAME := BIAdmin

all: init linux windows nice

init:
	mkdir -p bin


linux:
	$(GCC) $(CFLAGS) -o bin/$(NAME) src/main.c $(LDFLAGS) ../la-C/lib/libla.a

windows:
	x86_64-w64-mingw32-windres res/icon.rc -O coff -o res/icon.res
	x86_64-w64-mingw32-gcc $(CFLAGS) -o bin/$(NAME).exe src/main.c res/icon.res $(LDFLAGS) ../la-C/lib/la.lib

nice:
	strip --strip-all bin/*
