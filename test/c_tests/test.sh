cd gen_data
if [ ! -f "random.input" ];
then
    ./build.sh
fi
cd ..

if [ ! -f "rtrie_consistant.exe" ];
then
    ./build.sh
fi

echo "rtrie Running consistancy tests"
export LD_LIBRARY_PATH=`pwd`/../../build/libs
echo "======================================"
time ./rtrie_consistant.exe -i=gen_data/random.input -m=gen_data/random.missing -e=gen_data/random.expected
echo "======================================"
time ./rtrie_consistant.exe -i=gen_data/wSrcTime.input -m=gen_data/wSrcTime.missing -e=gen_data/wSrcTime.expected

echo "ttrie Running consistancy tests"
echo "======================================"
time ./ttrie_consistant.exe -i=gen_data/random.input -m=gen_data/random.missing -e=gen_data/random.expected
echo "======================================"
time ./ttrie_consistant.exe -i=gen_data/wSrcTime.input -m=gen_data/wSrcTime.missing -e=gen_data/wSrcTime.expected

