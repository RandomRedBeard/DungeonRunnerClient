OBJS = $(OBJS) \
src\itemres\item.o \
src\itemres\weapon.o \
src\itemres\armor.o \
src\itemres\bow.o \
src\itemres\arrow.o

src\itemres\item.o:
	$(CC) $(CFLAG) ..\src\itemres\item.cpp $(INCLUDES) /Fo: src\itemres\item.o
src\itemres\weapon.o:
	$(CC) $(CFLAG) ..\src\itemres\weapon.cpp $(INCLUDES) /Fo: src\itemres\weapon.o
src\itemres\armor.o:
	$(CC) $(CFLAG) ..\src\itemres\armor.cpp $(INCLUDES) /Fo: src\itemres\armor.o
src\itemres\bow.o:
	$(CC) $(CFLAG) ..\src\itemres\bow.cpp $(INCLUDES) /Fo: src\itemres\bow.o
src\itemres\arrow.o:
	$(CC) $(CFLAG) ..\src\itemres\arrow.cpp $(INCLUDES) /Fo: src\itemres\arrow.o