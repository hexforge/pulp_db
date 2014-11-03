c99 -O3 -fpic -c -Wall -Werror *\.c ../../../pulp_ds__prototype/common/mmbuf/mmbuf.c -I../../../pulp_ds__prototype/common/mmbuf
c99 -O3 test.o mmbuf.o trie.o -o first.exe

c99 -O3 --shared mmbuf.o trie.o -g -o first.so

#Debug
#rm -rf .*\.o
#c99 -fpic -c -g -Wall -Werror *\.c ../../../pulp_ds__prototype/common/mmbuf/mmbuf.c -I../../../pulp_ds__prototype/common/mmbuf
#c99 --shared mmbuf.o trie.o -g -o first.so
#c99 -g trie1_bench.c first.so
