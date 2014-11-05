# build/pulp_db is the python build
# build/libs build/headers is the c build


./clean.sh

mkdir build
mkdir build/headers
mkdir build/libs

# Copy headers
cp src/pulp_db/kds/*.h build/headers
cp src/pulp_db/mds/*.h build/headers
cp src/pulp_db/common/*.h build/headers
cp src/pb_tools/*.h build/headers


cd src/pulp_db

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
cd ..

cp src/pulp_db/kds/*.so build/libs
cp src/pulp_db/mds/*.so build/libs
cp src/pb_tools/*.so build/libs

cp -r src/pulp_db build/
