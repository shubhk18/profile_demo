#!/bin/bash

apt-get update && apt-get install -y g++ && g++ server.cpp -std=c++17 -O2 -pthread -o server
