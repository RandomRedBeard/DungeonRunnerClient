OBJS = $(OBJS) \
src\iores\mypoll.o \
src\iores\io.o \
src\iores\Socket.o

src\iores\myPoll.o:
	$(CC) $(CFLAG) ..\src\iores\myPoll.cpp $(INCLUDES) /Fo: src\iores\myPoll.o
src\iores\io.o:
	$(CC) $(CFLAG) ..\src\iores\io.cpp $(INCLUDES) /Fo: src\iores\io.o
src\iores\Socket.o:
	$(CC) $(CFLAG) ..\src\iores\Socket.cpp $(INCLUDES) /Fo: src\iores\Socket.o
