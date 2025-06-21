all:
	arm-linux-gnueabi-g++ main.cpp -g3 -O0 -o a.out-arm
	arm-linux-gnueabi-objdump --no-show-raw-insn -SCI --prefix=./  ./a.out-arm  > main.dis
	g++ main.cpp -O0 -o a.out
	./a.out

clean:
	rm -rf a.out main.o main.dis a.out-arm
