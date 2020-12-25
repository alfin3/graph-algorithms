CC = gcc
CFLAGS = -I. -Wall -Wextra -O3
OBJ = utilities-rand-mod-main.o utilities-rand-mod.o

utilities-ds : $(OBJ)
	$(CC) $(CFLAGS) -o utilities-rand-mod $(OBJ) 

utilities-rand-mod-main.o : utilities-rand-mod.h 
utilities-rand-mod.o : utilities-rand-mod.h

.PHONY : clean clean-all

clean :
	rm $(OBJ)
clean-all : 
	rm -f utilities-rand-mod $(OBJ)
