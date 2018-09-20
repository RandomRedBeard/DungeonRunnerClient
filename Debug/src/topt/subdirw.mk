OBJS = $(OBJS) \
src\topt\topt.o

src\topt\topt.o:
	$(CC) $(CFLAG) ..\src\topt\topt.cpp $(INCLUDES) /Fo: src\topt\topt.o