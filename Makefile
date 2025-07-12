CC=g++
CFLAGS= -std=c++11 -Wno-deprecated -O2
LDFLAGSPAR = -pthread -lpthread -latomic
LDFLAGS=
CPPFLAGS= -I./include

all: vf3 vf3l vf3p

vf3:
	$(CC) $(CFLAGS) $(CPPFLAGS) -o bin/$@ main.cpp -DVF3 $(LDFLAGS)

vf3l:
	$(CC) $(CFLAGS) $(CPPFLAGS) -o bin/$@ main.cpp -DVF3L $(LDFLAGS)

vf3p:
	$(CC) $(CFLAGS) $(CPPFLAGS) -o bin/$@ main.cpp -DVF3P $(LDFLAGSPAR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o bin/$@_bio main.cpp -DVF3BIO -DVF3P $(LDFLAGSPAR)

clean:
	rm bin/*
