CC=g++
CFLAGS=-g -Wall
SOURCES=main.cpp input.cpp lexan.cpp
OBJECTS=$(SOURCES:.cpp=.o)
MAIN= ps

.PHONY: lexan clean 
all: $(MAIN)
	@echo Simple compiler has been compiled
$(MAIN): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(MAIN) $(OBJECTS)
    
.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o *~ $(MAIN)
	echo Clean Done!

lexan: $(OBJECTS)
	$(CC) $(CFLAGS) -o lexan $(OBJECTS)
	rm *.o
	

