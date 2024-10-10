CFLAGS= -Wall -pedantic -std=gnu99 `pkg-config --cflags glib-2.0 libnotify`
LDFLAGS= `pkg-config --libs glib-2.0 libnotify`

all: src/main

build-dir:
	if [ ! -d build ]; then mkdir build; fi

main: src/main.c
	gcc $(CFLAGS) src/main.c -o build/watchdog.out $(LDFLAGS)

