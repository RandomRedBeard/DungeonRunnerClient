OBJS += src/Source.o \
src/log.o \
src/player.o \
src/point.o

src/%.o: ../src/%.cpp
	$(CC) $(CFLAG) $(LFLAG) -o $@ $<
