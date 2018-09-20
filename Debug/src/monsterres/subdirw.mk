OBJS = $(OBJS) \
src\monsterres\monster.o

src\monsterres\monster.o:
	$(CC) $(CFLAG) ..\src\monsterres\monster.cpp $(INCLUDES) /Fo: src\monsterres\monster.o