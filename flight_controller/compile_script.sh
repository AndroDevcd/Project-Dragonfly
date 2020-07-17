echo "compiling sharp source code"
sharpc main.sharp -L ../common/ -L device/ -o ../bin/fc -nd ../generated
echo "compiling c++ code"
g++ generated/*.cpp flight_controller/device/*.cpp common/gpio/*.cpp common/network/driver/*.cpp -std=c++11 -shared -o bin/dragonfly.so -lrt -lstdc++ -lwiringPi -lrf24 -O3 -Ofast -fPIC 
echo "done!"
