CC=g++

rash: rash.o history.o wildcard.o parser.o signal_handler.o command.o pipeline.o delep.o sb.o
	$(CC) rash.o history.o wildcard.o parser.o signal_handler.o command.o pipeline.o delep.o sb.o -o rash -lreadline -lstdc++fs

rash.o: rash.cpp
	$(CC) -c rash.cpp -o rash.o

history.o: history.cpp history.h
	$(CC) -c history.cpp -o history.o

wildcard.o: wildcard.cpp wildcard.h
	$(CC) -c wildcard.cpp -o wildcard.o

parser.o: parser.cpp parser.h
	$(CC) -c parser.cpp -o parser.o

signal_handler.o: signal_handler.cpp signal_handler.h
	$(CC) -c signal_handler.cpp -o signal_handler.o

command.o: command.cpp command.h
	$(CC) -c command.cpp -o command.o

pipeline.o: pipeline.cpp pipeline.h
	$(CC) -c pipeline.cpp -o pipeline.o

delep.o: delep.cpp delep.h
	$(CC) -c delep.cpp -o delep.o

sb.o: sb.cpp sb.h
	$(CC) -c sb.cpp -o sb.o

trojan: trojan.cpp
	$(CC) trojan.cpp -o trojan

delep_test: delep_test.cpp
	$(CC) delep_test.cpp -o delep_test
	
.PHONY: clean

clean:
	rm *.o
	
