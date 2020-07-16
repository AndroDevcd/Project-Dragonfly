echo "compiling sharp source code"
sharpc main.sharp -L core/ -L obsticleAvoidence/ -o ../bin/fc -nd ../generated
echo "compiling c++ code"
g++ -DFLIGHT_CONTROLLER */*.cpp -std=c++11 -shared -o ../bin/dragonfly.so -lrt -lstdc++ -lwiringPi -O3 -Ofast -fPIC 
echo "done!"