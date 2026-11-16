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
# Sharp is built on the Pi itself (there is no ready-made ARM build): the VM is compiled here, and the
# compiler and standard library come from the release in the repository. It ends up in ~/.sharp, with
# `sharp` and `sharpc` on the PATH of a NEW terminal.
git clone https://github.com/AndroDevcd/Sharp.git
cd Sharp
git checkout runtime-3.0.0_r8
bash build-scripts/install_from_source.sh
cd ../

echo "done. Open a new terminal, then in Project-Dragonfly:  bash build.sh"
