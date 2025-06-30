CC=g++
CFLAGS= -std=c++11 -Wno-deprecated -O2 #-pg -g
LDFLAGS= -pthread -lpthread -latomic
CPPFLAGS= -I./include

all: vf3 vf3l 

vf3:
	$(CC) $(CFLAGS) $(CPPFLAGS) -o bin/$@ main.cpp -DVF3 $(LDFLAGS)

vf3l:
	$(CC) $(CFLAGS) $(CPPFLAGS) -o bin/$@ main.cpp -DVF3L $(LDFLAGS)

vf3p:
	$(CC) $(CFLAGS) $(CPPFLAGS) -o bin/$@ main.cpp -DVF3P $(LDFLAGS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o bin/$@_bio main.cpp -DVF3BIO -DVF3P $(LDFLAGS)

clean:
	rm bin/*
