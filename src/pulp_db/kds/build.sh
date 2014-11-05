

# Debugging
clang -std=c99 -c -g -fpic -Wall -Wextra -Werror *\.c ../../common/mmbuf.c -I../../common -Wno-unused-function -Wno-unused-parameter 

clang -std=c99 -g --shared mmbuf.o rtrie.o -g -o librtrie.so
clang -std=c99 -g --shared mmbuf.o rtrie.o ttrie.o -o libttrie.so
clang -std=c99 -g --shared mmbuf.o dpref.o -o libdpref.so
clang -std=c99 -g --shared mmbuf.o cpref.o -o libcpref.so

#clang -std=c99 -g --shared -Wl,-soname,libkds.so.1 kds.o rtrie.o ttrie.o dpref.o cpref.o mmbuf.o  -o libkds.so
clang -std=c99 -g --shared kds.o rtrie.o ttrie.o dpref.o cpref.o mmbuf.o  -o libkds.so

gcc check_build.c libkds.so -I../../common -Wl,-rpath=`pwd`
