# CppSpikes

Various programs to investigate specific behaviors of C++ and the interactions with compilers, operating systems and hardware.

## AtomicExchangeSpikes.cpp
>**g++ -O0 -pthread AtomicExchangeSpikes.cpp -o bin/AtomicExchangeSpikes && bin/exchangeSpikes**

Investigations on `atomic::exchange` set of functions and their potential implications on mutex-free data structures
