#!/bin/bash
set -e

echo "🔧 Installing compiler..."
apt-get update
apt-get install -y g++

echo "🔧 Building server..."
g++ server.cpp -std=c++17 -O2 -pthread -o server

echo "📂 Files:"
ls -l

echo "🚀 Starting server..."
./server
