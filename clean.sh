rm -rf build 
rm -rf core.*

cd src/pulp_db
cd kds
./clean.sh
cd ..

cd mds
./clean.sh
cd ..

cd ..

cd common
./clean.sh
cd ..


cd pb_tools
./clean.sh
cd ..

cd ..

find . -name '__pycache__' | xargs rm -rf 


