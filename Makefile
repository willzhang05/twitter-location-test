CFLAGS=-Wall -g

clean:
	rm -f main
all:
	clang -lcurl main.cpp
