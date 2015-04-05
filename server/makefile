CC=g++
CFLAGS=-c -Wall -std=c++11
LDFLAGS=-lsfml-network
SOURCES=sfml_server.cpp game.cpp
OBJECTS=$(SOURCES:.cpp=.o)
	EXECUTABLE1=sfml_server

all: $(SOURCES) $(EXECUTABLE1)
	    
$(EXECUTABLE1): $(OBJECTS) 
	    $(CC) $(LDFLAGS) sfml_server.o game.o -o $@

.cpp.o:
	    $(CC) $(CFLAGS) $< -o $@
