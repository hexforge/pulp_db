./clean.sh

clang -std=c99 -g -fpic consistancy_rtrie.c -I../../build/headers -lrtrie -L../../build/libs -o rtrie_consistant.exe
clang -std=c99 -g -fpic consistancy_ttrie.c -I../../build/headers -lrtrie -lttrie -L../../build/libs -o ttrie_consistant.exe

mkdir tmp
