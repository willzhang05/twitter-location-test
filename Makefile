CFLAGS=-Wall -g

all:
	clang++ -lcurl -o main main.cpp
clean:
	rm -f main
