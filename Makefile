CFLAGS = -Wall -Wextra -ggdb3 -std=c++11 -fno-exceptions -fno-rtti -fvisibility=hidden -march=native -pipe
main: main.cpp
	clang++ $(CFLAGS) -lcurl -lcrypto -ljsoncpp -o $@ $^
clean:
	rm -f main lookup.json list.json
