all: termgl

CC=cc
PTHREADS=1

ifeq ($(PTHREADS),1)
        USE_THREADS=-lpthread -DPTHREADS=1
endif

UNAME=$(shell uname)
ifeq ($(UNAME),Darwin)
        OPENGL=-framework OpenGL -I/opt/homebrew/include -L/opt/homebrew/lib
else ifeq ($(UNAME),Windows_NT)
        OPENGL=-lopengl32
else
        OPENGL=-lGL
endif

LIBS=$(USE_THREADS) $(OPENGL) -lGLFW
CFLAGS=-O3 -pipe -Wno-incompatible-function-pointer-types -Wno-incompatible-pointer-types
DEBUGCFLAGS=-Og -pipe -g

INPUT=termgl.c
OUTPUT=termgl

RM=/bin/rm

termgl: $(INPUT)
	$(CC) $(INPUT) -o $(OUTPUT) $(LIBS) $(CFLAGS)
debug: $(INPUT)
	$(CC) $(INPUT) -o $(OUTPUT) $(LIBS) $(DEBUGCFLAGS)
clean: $(OUTPUT)
	if [ -e $(OUTPUT) ]; then $(RM) $(OUTPUT); fi
