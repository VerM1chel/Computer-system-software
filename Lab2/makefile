compiler:= gcc
flg:= -W -Wall -Wextra -xc -std=c11 -ggdb -g3 -D_DEFAULT_SOURCE

main: cfind.c
	$(compiler) $(flg) -o cfind cfind.c
clean:
	rm -rf -v cfind
all: clean main
