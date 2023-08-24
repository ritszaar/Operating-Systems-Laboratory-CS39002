all: prog

prog: mainthread.o guestthread.o cleanerthread.o
	g++ mainthread.o guestthread.o cleanerthread.o -o prog -lpthread -lrt

mainthread.o: mainthread.cpp
	g++ -c mainthread.cpp -o mainthread.o

guestthread.o: guestthread.cpp
	g++ -c guestthread.cpp -o guestthread.o

cleanerthread.o: cleanerthread.cpp
	g++ -c cleanerthread.cpp -o cleanerthread.o

run: prog
	./prog

clean:
	rm -f *.o prog