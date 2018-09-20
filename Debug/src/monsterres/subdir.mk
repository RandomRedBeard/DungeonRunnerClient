OBJS += src/monsterres/monster.o

src/monsterres/%.o: ../src/monsterres/%.cpp
	$(CC) $(CFLAG) $(LFLAG) -o $@ $<
