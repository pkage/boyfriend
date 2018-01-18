all: build

clean:
	rm -f ./bf
	rm -rf ./bf.dSYM

build: clean
	clang boyfriend.c -o bf

run: build
	./bf test/helloworld.b

debug: clean
	clang boyfriend.c -o bf -g
	sudo gdb -tui --args ./bf test/test.b
