gcc -c -Wall -Werror -fpic main.c 
gcc --shared -o foobar.so main.o
