c99 -O3 -fpic -c -Wall -Werror *\.c ../../../pulp_ds__prototype/common/mmbuf/mmbuf.c -I../../../pulp_ds__prototype/common/mmbuf
c99 -O3 test.o mmbuf.o rtrie.o -o rtrie.exe
c99 -O3 --shared mmbuf.o rtrie.o -g -o rtrie.so
c99 -O3 rtrie_bench.c rtrie.so

#Debug
#rm -rf .*\.o
#c99 -fpic -c -g -Wall -Werror *\.c ../../../pulp_ds__prototype/common/mmbuf/mmbuf.c -I../../../pulp_ds__prototype/common/mmbuf
#c99 --shared mmbuf.o rtrie.o -g -o rtrie.so
#c99 -g rtrie_bench.c rtrie.so
