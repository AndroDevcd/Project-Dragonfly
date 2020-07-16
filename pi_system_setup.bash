#! /bin/bash

apt-get update
apt-get upgrade

echo "Installing git"
apt-get install git

echo "Installing make"
apt-get install cmake
apt-get install make

echo "Installijg c++ compiler"
apt-get install g++
apt-get install gcc

echo "Installing wiringPi"
apt-get install wiringpi
git clone https://github.com/AndroDevcd/WiringPi.git
cd WiringPi
chmod +x build
./build
cd ../

echo "Installing nrf24"
mkdir rf24
cd rf24
wget http://tmrh20.github.io/RF24Installer/RPi/install.sh 
chmod +x install.sh
./install.sh
cd ../

echo "Installing sharp"
git clone https://github.com/AndroDevcd/Sharp.git

cd Sharp
git checkout remastered
chmod u+x build-scripts/linux-x64-compile-script.bash
./build-scripts/linux-x64-compile-script.bash
