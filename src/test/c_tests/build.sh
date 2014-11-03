./clean.sh

clang -std=c99 -g -fpic consistancy_rtrie.c -I../../headers -lrtrie -L../../libs -o rtrie_consistant.exe
clang -std=c99 -g -fpic consistancy_ttrie.c -I../../headers -lrtrie -lttrie -L../../libs -o ttrie_consistant.exe

mkdir tmp
