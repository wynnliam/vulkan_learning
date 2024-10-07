#!/bin/bash

# use -DNDEBUG if you want release

rm *.out
g++ -std=c++17 -O2  main.cpp application.cpp -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXi
