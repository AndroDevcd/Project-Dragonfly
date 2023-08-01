#! /bin/bash

echo "compiling controller source code..."
sharpc -p
cd ../

echo "compiling controller c++ code..."
g++ -DCONTROLLER -o controller/dragonfly.so -shared controller/src/cpp/ui/driver/*.cpp common/src/cpp/gpio/*.cpp common/src/cpp/network/driver/*.cpp controller/generated/*.cpp -lwiringPi -lrf24-bcm

#cp controller/dragonfly.so bin/

echo "done..."