OBJS = $(OBJS) \
src\Source.o \
src\log.o \
src\player.o \
src\point.o \
src\key_val.o

src\Source.o:
	$(CC) $(CFLAG) ..\src\Source.cpp $(INCLUDES) /Fo: src\Source.o
src\log.o:
	$(CC) $(CFLAG) ..\src\log.cpp $(INCLUDES) /Fo: src\log.o
src\player.o:
	$(CC) $(CFLAG) ..\src\player.cpp $(INCLUDES) /Fo: src\player.o
src\point.o:
	$(CC) $(CFLAG) ..\src\point.cpp $(INCLUDES) /Fo: src\point.o
src\key_val.o:
	$(CC) $(CFLAG) ..\src\key_val.cpp $(INCLUDES) /Fo: src\key_val.o
