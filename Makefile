all: main.c socket.c utils.c x11magic.c
	gcc main.c x11magic.c -lraylib -lm -lX11 -lglfw -o bar


