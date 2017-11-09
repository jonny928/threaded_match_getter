OBJS = main.cpp
CC = g++
DEBUG = -g
CFLAGS = -c $(DEBUG)
LFLAGS = -std=c++11 -pthread $(DEBUG)

all: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o main -L/usr/lib/x86_64-linux-gnu -lcurl

clean:
	rm -f main
