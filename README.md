# CppSpikes

Various programs to investigate specific behaviors of C++ and the interactions with compilers, operating systems and hardware.


## AtomicExchangeSpikes.cpp

Investigations on `atomic::exchange` set of functions and their potential implications for mutex-free data structures

>**g++ -std=c++17 -O3 -march=native -mtune=native -pthread AtomicExchangeSpikes.cpp -o bin/AtomicExchangeSpikes && bin/AtomicExchangeSpikes**