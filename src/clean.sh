rm -rf headers
rm -rf libs
rm -rf core.*

cd pulp_db
cd kds
./clean.sh
cd ..

cd mds
./clean.sh
cd ..

cd common
./clean.sh
cd ..
cd ..

cd pb_tools
./clean.sh
cd ..

rm -rf ./test/__pycache__ ./test/__pycache__/__pycache__ ./pulp_db/__pycache__

