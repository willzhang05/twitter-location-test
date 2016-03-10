CFLAGS=-Wall -g

all:
	clang++ -lcurl -std=c++11 -o main main.cpp
clean:
	rm -f main; rm -f lookup.json
