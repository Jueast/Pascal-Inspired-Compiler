CC=g++
CPPFLAGS=-g -Wall -std=c++11
SOURCES=main.cpp input.cpp lexan.cpp
OBJECTS=$(SOURCES:.cpp=.o)
MAIN= ps

.PHONY: lexan clean 
all: $(MAIN)
	@echo Simple compiler has been compiled
$(MAIN): $(OBJECTS)
	$(CC) $(CPPFLAGS) -o $(MAIN) $(OBJECTS)
    
.c.o:
	$(CC) $(CPPFLAGS) $< -o $@

clean:
	rm *.o *~ $(MAIN)
	echo Clean Done!

lexan: $(OBJECTS)
	$(CC) $(CPPFLAGS) -o lexan $(OBJECTS)
	rm *.o
	

