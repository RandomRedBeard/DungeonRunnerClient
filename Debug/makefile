CC = gcc
CFLAG = -Wall -c
LFLAG = -lstdc++ -std=c++14 -lpthread -lm -lcurses
TARGET = drclient

OBJS = 

include src/subdir.mk
include src/iores/subdir.mk
include src/itemres/subdir.mk
include src/monsterres/subdir.mk
include src/topt/subdir.mk

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LFLAG) -o $(TARGET)

clean:
	rm -f $(OBJS)
