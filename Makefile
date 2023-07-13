all: termgl

CC=cc

UNAME=$(shell uname)
ifeq ($(UNAME),Darwin)
        OPENGL=-framework OpenGL
else ifeq ($(UNAME),Windows_NT)
        OPENGL=-lopengl32
else
        OPENGL=-lGL
endif
LIBS=$(OPENGL) -lGLFW -I/opt/homebrew/include -L/opt/homebrew/lib
CFLAGS=-O3 -pipe
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
