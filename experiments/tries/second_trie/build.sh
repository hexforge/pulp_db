c99 -O3 -fpic -c -Wall -Werror *\.c ../../../pulp_ds__prototype/common/mmbuf/mmbuf.c -I../../../pulp_ds__prototype/common/mmbuf
c99 -O3 test.o mmbuf.o trie.o -o second.exe

c99 -O3 --shared mmbuf.o trie.o -g -o second.so

#Debug
#rm -rf .*\.o
#c99 -fpic -c -g -Wall -Werror *\.c ../../../pulp_ds__prototype/common/mmbuf/mmbuf.c -I../../../pulp_ds__prototype/common/mmbuf
#c99 --shared mmbuf.o trie.o -g -o second.so
#c99 -g trie2_bench.c second.so
