CC=g++
CFLAGS=-c -Wall -O2 -std=c++11
LFLAGS=
PROGS= highscore
OBJS= highscore.o serialport.o

# executable targets
all: highscore

highscore: highscore.o serialport.o
	$(CC) highscore.o serialport.o -o highscore $(LFLAGS)

# object targets
highscore.o: highscore.cpp
	$(CC) highscore.cpp -o highscore.o $(CFLAGS)

serialport.o: serialport.cpp serialport.h
	$(CC) serialport.cpp -o serialport.o $(CFLAGS)

clean:
	@rm $(OBJS) $(PROGS)
