CC=g++
CFLAGS=-c -Wall -std=c++11
LDFLAGS=-lsfml-network
SOURCES=server.cpp game.cpp
OBJECTS=$(SOURCES:.cpp=.o)
	EXECUTABLE=server

all: $(SOURCES) $(EXECUTABLE)
	    
$(EXECUTABLE): $(OBJECTS) 
	    $(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	    $(CC) $(CFLAGS) $< -o $@
