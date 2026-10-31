CC=gcc
CFLAGS= -c  
LFLAGS= 
SOURCES=main.c params.c for_solution.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=main.exe

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LFLAGS) $(OBJECTS) -o $@ && rm *.o

.c.o:
	$(CC) $(CFLAGS) $< -o $@