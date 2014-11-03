clang -c -g -fpic -Wall -Werror -Wextra *\.c -Wno-unused-parameter 
clang --shared metaparse.o mmbuf.o -o meta.so
