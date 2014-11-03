clang -c -g -Wall -Werror *\.c ../../../pulp_ds__prototype/common/mmbuf/mmbuf.c -I../../../pulp_ds__prototype/common/mmbuf

clang -O3 -g test.o mmbuf.o cpref.o

#clang -O3 -g cpref.o 
