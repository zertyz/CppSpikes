#include <iostream>
#include <atomic>
#include <thread>
#include <cstring>
#include <mutex>

#define DOCS "std::atomic.exchange(...) spikes\n" \
             "================================\n" \
             "\n" \
             "This program attempts to demonstrate some use cases of std::atomic.exchange set of functions,\n"   \
             "suitable for allowing the implementation of mutex-free queues, stacks, lists, etc: if one can\n"   \
             "change the 'head' pointer atomically, these structures will, then, be possible with a simple\n"    \
             "implementation -- as opposed to what we currently see as 'lock-free queues' on github.\n"          \
             "Here we attempt to change the integer var '_3_5_or_7', whose values comes from the array 'map',\n" \
             "which, in turn, depend on the value of '_3_5_or_7' to access the next value. In the end, an\n"     \
             "equal number of all possible values must be observed, so we are sure no race condition took place.\n"


// increase both of these if the reentrancy problem is not exposed
// (disabling compilation optimizations might also help)
#define N_THREADS   12
#define ITERACTIONS 1<<24


// test section
///////////////

long long  finalSum[8];
std::mutex sumGuard;
void restartSum() {
	memset(&finalSum, 0, sizeof finalSum);
}

void partialSum(int partial[8]) {
	sumGuard.lock();
	for (unsigned i=0; i<8; i++) {
		finalSum[i] += partial[i];
	}
	sumGuard.unlock();
}

void printSum() {
	std::cout << "final sum := {";
	for (int i=0; i<8; i++) {
		std::cout << finalSum[i] << ",";
	}
	std::cout << "}\n";
}

void reentrancyCheck() {
	std::cout << "--> Reentrancy check: "; 
	if ( (finalSum[3] == finalSum[5]) && (finalSum[5] == finalSum[7]) ) {
		std::cout << "PASSED\n"; 
	} else {
		std::cout << "FAILED\n"; 
	}
}

static struct timespec timespec_now;
static inline unsigned long long getMonotonicRealTimeNS() {
    clock_gettime(CLOCK_MONOTONIC, &timespec_now);
    return (timespec_now.tv_sec*1000000000ll) + timespec_now.tv_nsec;
}


// spike vars
/////////////

int              _3_5_or_7     = 3;
std::atomic<int> atomic_3_5_or_7(3);
int map[] = {0,0,0,5,0,7,0,3};


// spike methods
////////////////

void nonReentrant(int threadNumber) {
	int sum[] = {0,0,0,0,0,0,0,0};
	for (unsigned int i=0; i < ITERACTIONS; i++) {
		int val = _3_5_or_7;
		_3_5_or_7 = map[val];
		sum[val]++;
	}
	std::cout << "thread " << (threadNumber < 10 ? " ":"") << threadNumber << " finished. Summing..." << std::flush;
	partialSum(sum);
	std::cout << " done\n" << std::flush;
}

std::mutex opGuard;
void reentrant(int threadNumber) {
	int sum[] = {0,0,0,0,0,0,0,0};
	for (unsigned i=0; i < ITERACTIONS; i++) {
		opGuard.lock();
		int val = _3_5_or_7;
		_3_5_or_7 = map[val];
		opGuard.unlock();
		sum[val]++;
	}
	std::cout << "thread " << (threadNumber < 10 ? " ":"") <<  threadNumber << " finished. Summing..." << std::flush;
	partialSum(sum);
	std::cout << " done\n" << std::flush;
}

void atomic(int threadNumber) {
	int sum[] = {0,0,0,0,0,0,0,0};
	int val;
	for (unsigned i=0; i < ITERACTIONS; i++) {
		val = atomic_3_5_or_7.load(std::memory_order_relaxed);
        while (!atomic_3_5_or_7.compare_exchange_weak(val, map[val],
		                                              std::memory_order_release,
		                                              std::memory_order_relaxed));

		// the solution to the problem presented on this spike ended up being solved in
		// https://en.cppreference.com/w/cpp/atomic/atomic/compare_exchange -- a mutexless stack push example
		// --> the operation implemented here is equivalent to the 'pop' operation.

		sum[val]++;
	}
	std::cout << "thread " << (threadNumber < 10 ? " ":"") <<  threadNumber << " finished. Summing..." << std::flush;
	partialSum(sum);
	std::cout << " done\n" << std::flush;
}


int main(void) {

	std::cout << DOCS << "\n";

	std::cout << "Starting the non-reentrant version:\n";
	restartSum();
	long long start = getMonotonicRealTimeNS();
	std::thread nonReentrantThreads[N_THREADS];
	for (unsigned i=0; i<N_THREADS; i++) {
		nonReentrantThreads[i] = std::thread([](int threadNumber) {nonReentrant(threadNumber);}, i+1);
	}
    for (int i=0; i<N_THREADS; i++) {
    	nonReentrantThreads[i].join();
    }
    long long finish = getMonotonicRealTimeNS();
    printSum();
	reentrancyCheck();
    std::cout << "--> elapsed time: " << (finish - start) << "ns\n";
    std::cout << "--> Please see that the total sums of all domain values (3, 5 and 7) ARE NOT the same\n\n\n";


	std::cout << "Starting the reentrant version:\n";
	restartSum();
	start = getMonotonicRealTimeNS();
	std::thread reentrantThreads[N_THREADS];
	for (unsigned i=0; i<N_THREADS; i++) {
		reentrantThreads[i] = std::thread([](int threadNumber) {reentrant(threadNumber);}, i+1);
	}
    for (int i=0; i<N_THREADS; i++) {
    	reentrantThreads[i].join();
    }
    finish = getMonotonicRealTimeNS();
    printSum();
	reentrancyCheck();
    std::cout << "--> elapsed time: " << (finish - start) << "ns\n";
    std::cout << "--> Please see that the total sums of all domain values (3, 5 and 7) ARE the same\n";
    std::cout << "--> On the other hand, observe the huge 'kernel times', due to the use of mutexes\n\n\n";
	

	std::cout << "Starting the atomic version:\n";
	restartSum();
	start = getMonotonicRealTimeNS();
	std::thread atomicThreads[N_THREADS];
	for (unsigned i=0; i<N_THREADS; i++) {
		atomicThreads[i] = std::thread([](int threadNumber) {atomic(threadNumber);}, i+1);
	}
    for (int i=0; i<N_THREADS; i++) {
    	atomicThreads[i].join();
    }
    finish = getMonotonicRealTimeNS();
    printSum();
	reentrancyCheck();
    std::cout << "--> elapsed time: " << (finish - start) << "ns\n";
    std::cout << "--> Can we make this work faster than the reentrant version (which uses mutexes)?\n";

}
