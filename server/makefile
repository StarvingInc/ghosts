CC=g++
CFLAGS=-c -Wall -std=c++11
LDFLAGS=-lsfml-network
SOURCES=sfml_server.cpp game.cpp unix_server.cpp
OBJECTS=$(SOURCES:.cpp=.o)
	EXECUTABLE1=sfml_server

all: $(SOURCES) sfml_server unix_server
	    
sfml_server: $(OBJECTS) 
	    $(CC) $(LDFLAGS) sfml_server.o game.o -o $@

unix_server: $(OBJECTS) 
	    $(CC) $(LDFLAGS) unix_server.o game.o -o $@

.cpp.o:
	    $(CC) $(CFLAGS) $< -o $@
