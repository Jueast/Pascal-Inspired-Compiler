CC=g++
CPPFLAGS=-g -Wall -std=c++11 -D LOCAL_SFE_TEST
LEXAN_SOURCES=main_lexan.cpp input.cpp lexan.cpp
PARSER_SOURCES=main_parser.cpp input.cpp lexan.cpp ast.cpp parser.cpp
LEXAN_OBJECTS=$(LEXAN_SOURCES:.cpp=.o)
PARSER_OBJECTS=$(PARSER_SOURCES:.cpp=.o)
MAIN= ps

.PHONY: lexan clean parser 
all: $(MAIN)
	@echo Simple compiler has been compiled
$(MAIN): $(OBJECTS)
	$(CC) $(CPPFLAGS) -o $(MAIN) $(OBJECTS)
    
.c.o:
	$(CC) $(CPPFLAGS) $< -o $@

clean:
	rm *.o 
	echo Clean Done!

lexan: $(LEXAN_OBJECTS)
	$(CC) $(CPPFLAGS) -o lexan $(LEXAN_OBJECTS)
	rm *.o
	
parser: $(PARSER_OBJECTS)
	$(CC) $(CPPFLAGS) -o parser $(PARSER_SOURCES)
