CC=gcc
CFLAGS=-g -Wall
SOURCES=main.c input.c
OBJECTS=$(SOURCES:.cpp=.o)
MAIN=test

.PHONY: clean 
all: $(MAIN)
	@echo Simple compiler has been compiled
$(MAIN): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(MAIN) $(OBJECTS)
    
.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o *~ $(MAIN)
	echo Clean Done!


	
# DO NOT DELETE

main.o: /usr/include/stdio.h /usr/include/features.h
main.o: /usr/include/stdc-predef.h /usr/include/libio.h
main.o: /usr/include/_G_config.h /usr/include/wchar.h ./lib/input.h
