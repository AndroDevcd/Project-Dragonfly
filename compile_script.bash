echo "compiling sharp source code"
sharpc -debug -O main.sharp -L src/
echo "compiling c++ code"
# g++ src/core/*.cpp src/obsticleAvoidence/*.cpp src/snb/*.cpp -std=c++11 -shared -o dragonfly.so -pthread -lstdc++ -lrt -lstdc++ -O3 -Ofast -rpath='$ORIGIN'
echo "done!"
