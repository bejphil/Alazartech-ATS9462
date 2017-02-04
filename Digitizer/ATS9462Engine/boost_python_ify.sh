#!/bin/bash

#script to generate python module from c++ file
L_FLAGS=" -L/usr/local/lib \
-L/usr/lib \
-L/usr/lib/x86_64-linux-gnu/ \
-L/usr/local/AlazarTech/lib \
-L/usr/local/lib64
"

LIB_FLAGS+=" -lboost_iostreams \
-lboost_filesystem \
-lboost_system \
-lboost_thread \
-pthread"

LIB_FLAGS+=" -lfftw3_threads -lfftw3 -lm"

LIB_FLAGS+=" -lATSApi"

INCLUDE_FLAGS="-I/home/admx/Qt-Projects/JASPL \
-I/home/admx/Qt-Projects/ATS9462"

g++ -std=c++11 -DDEBUG -o ats9462.o -fPIC -c ats9462engine_python_wrapper.cpp -Wall -fPIC -I/usr/include/python3.4m -I/home/admx/Qt-Projects/JASPL -I/home/admx/Qt-Projects/ATS9462

g++ -fPIC -std=c++11 -shared -o ats9462.so ats9462.o  $L_FLAGS-L/usr/lib/python3.4/config-3.4m-x86_64-linux-gnu -lpython3.4m -lboost_python-py34 -lboost_system $LIB_FLAGS
