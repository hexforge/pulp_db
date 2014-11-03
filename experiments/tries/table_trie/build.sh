c99 -c -g -fpic -Wall -Werror *\.c ../radix_trie/rtrie.c ../../../pulp_ds__prototype/common/mmbuf/mmbuf.c -I../../../pulp_ds__prototype/common/mmbuf -I../radix_trie -Wno-unused-function

# Unit tests
c99 -O3 -g test.o rtrie.o mmbuf.o -o unit_test.exe

rm -rf *\.o

c99 -O3 -c -fpic -Wall -Werror *\.c ../radix_trie/rtrie.c ../../../pulp_ds__prototype/common/mmbuf/mmbuf.c -I../../../pulp_ds__prototype/common/mmbuf -I../radix_trie -Wno-unused-function 
c99 -O3 --shared -o libttrie.so rtrie.o tabletrie.o mmbuf.o 
c99 -O3 libttrie.so table_trie_write.c -o bug.exe -I../radix_trie

#c99 -g -c -fpic -Wall -Werror *\.c ../radix_trie/rtrie.c ../../../pulp_ds__prototype/common/mmbuf/mmbuf.c -I../../../pulp_ds__prototype/common/mmbuf -I../radix_trie -Wno-unused-function
#c99 -g --shared -o libttrie.so rtrie.o tabletrie.o mmbuf.o
#c99 -g libttrie.so table_trie_write.c -o bug.exe -I../radix_trie


# Clang for better error message if stuck

#clang -c -g -fPIC -Wall -Werror *\.c ../radix_trie/rtrie.c ../../../pulp_ds__prototype/common/mmbuf/mmbuf.c -I../../../pulp_ds__prototype/common/mmbuf -I../radix_trie -Wno-unused-function 

# Unit tests
#clang -g test.o rtrie.o mmbuf.o  -o unit_test.exe

# So
#c99 --shared -o libttrie.so rtrie.o tabletrie.o mmbuf.o

