echo "compiling sharp source code"
sharpc -p
cd ../

echo "compiling controller c++ code..."
g++ -o flight_controller/dragonfly.so -shared common/src/cpp/gpio/*.cpp common/src/cpp/network/driver/*.cpp -lwiringPi -lrf24-bcm

cp flight_controller/dragonfly.so /bin/

echo "done..."
