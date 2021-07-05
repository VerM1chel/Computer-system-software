all:
	clear
	gcc -fPIC -c my_aio.c
	gcc -shared  -nostartfiles -o my_aio.so my_aio.o -lrt
	gcc -g -c Lab5.c
	gcc -g -rdynamic Lab5.o  -o main -ldl -lpthread
