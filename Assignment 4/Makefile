sns: sns.o
	g++ sns.o -o sns -lpthread

sns.o: sns.cpp
	g++ -c sns.cpp -o sns.o

run:
	./sns

clean:
	rm -rf sns.o sns sns.log