all:
	g++ ./01-thread.cpp -g -O0 -o ./01 -lpthread
	g++ ./02-mutex.cpp -g -O0 -o ./02 -lpthread
	g++ ./03-condition_variable.cpp -g -O0 -o ./03 -lpthread
	./01
	./02
	./03

clean:
	rm -f ./01 02 ./03
