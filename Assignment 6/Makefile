all: libgoodmalloc.a mergesort

libgoodmalloc.a: goodmalloc.o
	ar -rcs libgoodmalloc.a goodmalloc.o

goodmalloc.o: goodmalloc.cpp goodmalloc.h
	g++ -c goodmalloc.cpp -lpthread

mergesort: mergesort.o libgoodmalloc.a
	g++ -o mergesort mergesort.o -L. -lgoodmalloc -lpthread

mergesort.o: mergesort.cpp
	g++ -c mergesort.cpp
run: mergesort
	./mergesort
clean:
	rm -f libgoodmalloc.a goodmalloc.o mergesort mergesort.o