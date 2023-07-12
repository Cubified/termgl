all: termgl

CC=cc

LIBS=-framework OpenGL -lGLFW -I/opt/homebrew/include -L/opt/homebrew/lib
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
