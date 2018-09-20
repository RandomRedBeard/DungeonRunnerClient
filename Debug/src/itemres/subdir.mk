OBJS += src/itemres/item.o \
src/itemres/weapon.o \
src/itemres/bow.o \
src/itemres/armor.o \
src/itemres/arrow.o 

src/itemres/%.o: ../src/itemres/%.cpp
	$(CC) $(CFLAG) $(LFLAG) -o $@ $<
