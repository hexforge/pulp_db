cd ..
gcc  -c -g -Wall -Werror *\.c
cp *o test
cd test
gcc -c *\.c -I../
gcc -g *\.o
