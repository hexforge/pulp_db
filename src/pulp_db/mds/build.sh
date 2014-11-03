#-Weverything
clang -std=c99 -pedantic -fpic -c -g -Wall -Wextra -Werror *.c ../common/mmbuf.c -I../common  -Wno-unused-parameter
clang -std=c99 -g --shared bitpack.o hex.o  mds.o  mmbuf.o -o libmds.so
