./clean.sh

mkdir headers
mkdir libs
cp pulp_db/kds/*.h headers
cp pulp_db/mds/*.h headers
cp pulp_db/common/*.h headers

cp pb_tools/*.h headers

cd pulp_db

cd kds
./build.sh
cd ..
cd mds
./build.sh
cd ..
cd common
./build.sh
cd ..
cd ..

cd pb_tools
./build.sh
cd ..

cp pulp_db/kds/*.so libs
cp pulp_db/mds/*.so libs
cp pb_tools/*.so libs
