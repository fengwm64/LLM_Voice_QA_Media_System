cd ./client
make clean;make
echo "bulid client done!"
echo

cd ../server
export LD_LIBRARY_PATH=$(pwd)/libs/x64/
make clean;make LINUX64=1
echo "bulid server done!"
cd ./bin/msc && rm *log 
echo

cd ../ && ./server_main