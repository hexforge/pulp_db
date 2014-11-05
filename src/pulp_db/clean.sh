rm -rf __pycache__

cd kds
./clean.sh
cd ..

cd mds
./clean.sh
cd ..

cd ../common
./clean.sh
cd ../pulp_db
