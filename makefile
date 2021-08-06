all: node

node: node.o message.o main.o select.o
	g++ -g -o node $^

node.o: node.cpp node.hpp
	g++ -g -c node.cpp

message.o: message.cpp message.hpp
	g++ -g -c message.cpp

main.o: main.cpp
	g++ -g -c main.cpp

select.o: select.c select.h
	g++ -g -c select.c

clean:
	rm -f *.o