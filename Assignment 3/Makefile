run_unoptimized: producer consumer main_process
	./main_process 120

run_optimized: producer consumer main_process
	./main_process 120 -optimize

producer: producer.o
	g++ producer.o -o producer

producer.o: producer.cpp
	g++ -c producer.cpp

consumer: consumer.o
	g++ consumer.o -o consumer

consumer.o: consumer.cpp
	g++ -c consumer.cpp

main_process: main_process.o
	g++ main_process.o -o main_process

main_process.o: main_process.cpp
	g++ -c main_process.cpp