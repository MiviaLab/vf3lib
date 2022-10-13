CC=g++
CFLAGS= -std=c++11 -O3 -Wno-deprecated
LDFLAGS= -lpthread -latomic
CPPFLAGS= -I./include

all: vf3 vf3l 

vf3:
	$(CC) $(CFLAGS) $(CPPFLAGS) -o bin/$@ main.cpp -DVF3

vf3.so:
	$(CC) $(CFLAGS) $(CPPFLAGS) -fPIC -shared -o bin/$@ main.cpp -DVF3

vf3l:
	$(CC) $(CFLAGS) $(CPPFLAGS) -o bin/$@ main.cpp -DVF3L

vf3p:
	$(CC) $(CFLAGS) $(CPPFLAGS) -o bin/$@ main.cpp -DVF3P $(LDFLAGS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o bin/$@_bio main.cpp -DVF3BIO -DVF3P $(LDFLAGS)

clean:
	rm bin/*
