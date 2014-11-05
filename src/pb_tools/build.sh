clang -c -g -fpic -Wall -Werror -Wextra *\.c ../common/mmbuf.c -I../common  -Wno-unused-parameter 
clang --shared metaparse.o mmbuf.o -o meta.so
