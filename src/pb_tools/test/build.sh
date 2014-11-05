cd ..
./build.sh
cp *o test
cd test
gcc -c *\.c -I../ -I../../common
gcc -g *\.o
