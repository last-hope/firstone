all:crawler
crawler:main.o
	g++ -g main.o -o crawler -lpthread -levent -levent_pthreads 
main.o: main.cpp
	g++ -g -c main.cpp -o main.o
clean:
	rm *.o && rm crawler
