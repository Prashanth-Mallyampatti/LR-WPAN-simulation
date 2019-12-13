# Author: Prashanth Mallyampatti
#!bin/bash

set -e

# Update and upgrade packages
apt-get -y update
apt-get -y upgrade

# Install pre-requisites
echo -e "\n\tInstalling G++\n"
apt-get -y install g++
echo -e "\n\tInstalling Python3\n"
apt-get -y install python3
apt-get -y upgrade python3
echo -e "\n\tInstalling Python2\n"
apt-get -y install python2.7    # if python3 is configured incorrectly
echo -e "\n\tInstalling git\n"
apt-get -y install git

# Upgrade installed packages
apt-get -y upgrade

# Get ns3 package
wget https://www.nsnam.org/releases/ns-allinone-3.29.tar.bz2
tar -xvf ns-allinone-3.29.tar.bz2
cd ns-allinone-3.29/ns-3.29/


# Build the packages
./waf configure --enable-examples --enable-tests --disable-werror
./waf

# Run the tests
python test.py
