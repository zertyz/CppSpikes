#include <iostream>
#include <atomic>
#include <thread>
#include <cstring>
#include <mutex>

#define ITERACTIONS 1<<26	// increase this if your CPU runs it too fast or if the reentrancy problem is not exposed


int              _3_5_or_7     = 3;
std::atomic<int> atomic_3_5_or_7(3);
int map[] = {0,0,0,5,0,7,0,3};

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
        do {
            val = atomic_3_5_or_7.load(std::memory_order_relaxed);
        } while (!atomic_3_5_or_7.compare_exchange_weak(val, map[val],
		                                               std::memory_order_release,
		                                               std::memory_order_relaxed));

        // strategy adapted from https://en.cppreference.com/w/cpp/atomic/atomic/compare_exchange -- a mutexless stack push example
        // (the operation here is equivalent to the pop operation)

		sum[val]++;
	}
	std::cout << "thread " << (threadNumber < 10 ? " ":"") <<  threadNumber << " finished. Summing..." << std::flush;
	partialSum(sum);
	std::cout << " done\n" << std::flush;
}

static struct timespec timespec_now;
static inline unsigned long long getMonotonicRealTimeNS() {
    clock_gettime(CLOCK_MONOTONIC, &timespec_now);
    return (timespec_now.tv_sec*1000000000ll) + timespec_now.tv_nsec;
}


int main(void) {

	std::cout << "std::atomic.exchange(...) spikes. Starting...\n";
	std::cout << "=============================================\n";
	std::cout << "\n";
	std::cout << "This programs attempts to demonstrate a use case of std::atomic suitable for allowing the implementation\n";
	std::cout << "of mutex-free queues, stacks, lists, etc: if one can change the 'head' pointer atomically, these structures\n";
	std::cout << "will, then, be possible with a simple implementation -- as opposed to what we currently see on github (very\n";
	std::cout << "complex implementations, many of which do uses ''fast'' mutexes -- a kind of mutex they implement which don't";
	std::cout << "involve the kernel).\n";
	std::cout << "Here we attempt to change the integer var '_3_5_or_7', whose values comes from the array 'map', which, in turn,\n";
	std::cout << "depend on the value of '_3_5_or_7' to access the next value. In the end, an equal number of all possible values\n";
	std::cout << "must be observed, so we are sure no race condition took place.\n\n";

	std::cout << "Starting the non-reentrant version:\n";

	restartSum();
	long long start = getMonotonicRealTimeNS();
	std::thread nonReentrantThreads[] = {
		std::thread([](int threadNumber) {nonReentrant(threadNumber);}, 1),
		std::thread([](int threadNumber) {nonReentrant(threadNumber);}, 2),
		std::thread([](int threadNumber) {nonReentrant(threadNumber);}, 3),
		std::thread([](int threadNumber) {nonReentrant(threadNumber);}, 4),
		std::thread([](int threadNumber) {nonReentrant(threadNumber);}, 5),
		std::thread([](int threadNumber) {nonReentrant(threadNumber);}, 6),
		std::thread([](int threadNumber) {nonReentrant(threadNumber);}, 7),
		std::thread([](int threadNumber) {nonReentrant(threadNumber);}, 8),
		std::thread([](int threadNumber) {nonReentrant(threadNumber);}, 9),
		std::thread([](int threadNumber) {nonReentrant(threadNumber);}, 10),
		std::thread([](int threadNumber) {nonReentrant(threadNumber);}, 11),
		std::thread([](int threadNumber) {nonReentrant(threadNumber);}, 12),
	};

    for (int i=0; i<sizeof(nonReentrantThreads)/sizeof(nonReentrantThreads[0]); i++) {
    	nonReentrantThreads[i].join();
    }
    long long finish = getMonotonicRealTimeNS();
    printSum();
    std::cout << "--> elapsed time: " << (finish - start) << "ns\n";

    std::cout << "--> Please see that the total sums of all domain values (3, 5 and 7) ARE NOT the same\n\n\n";


	std::cout << "Starting the reentrant version:\n";

	restartSum();
	start = getMonotonicRealTimeNS();
	std::thread reentrantThreads[] = {
		std::thread([](int threadNumber) {reentrant(threadNumber);}, 1),
		std::thread([](int threadNumber) {reentrant(threadNumber);}, 2),
		std::thread([](int threadNumber) {reentrant(threadNumber);}, 3),
		std::thread([](int threadNumber) {reentrant(threadNumber);}, 4),
		std::thread([](int threadNumber) {reentrant(threadNumber);}, 5),
		std::thread([](int threadNumber) {reentrant(threadNumber);}, 6),
		std::thread([](int threadNumber) {reentrant(threadNumber);}, 7),
		std::thread([](int threadNumber) {reentrant(threadNumber);}, 8),
		std::thread([](int threadNumber) {reentrant(threadNumber);}, 9),
		std::thread([](int threadNumber) {reentrant(threadNumber);}, 10),
		std::thread([](int threadNumber) {reentrant(threadNumber);}, 11),
		std::thread([](int threadNumber) {reentrant(threadNumber);}, 12),
	};

    for (int i=0; i<sizeof(reentrantThreads)/sizeof(reentrantThreads[0]); i++) {
    	reentrantThreads[i].join();
    }
    finish = getMonotonicRealTimeNS();
    printSum();
    std::cout << "--> elapsed time: " << (finish - start) << "ns\n";

    std::cout << "--> Please see that the total sums of all domain values (3, 5 and 7) ARE the same\n\n\n";

	std::cout << "Starting the atomic version:\n";

	restartSum();
	start = getMonotonicRealTimeNS();
	std::thread atomicThreads[] = {
		std::thread([](int threadNumber) {atomic(threadNumber);}, 1),
		std::thread([](int threadNumber) {atomic(threadNumber);}, 2),
		std::thread([](int threadNumber) {atomic(threadNumber);}, 3),
		std::thread([](int threadNumber) {atomic(threadNumber);}, 4),
		std::thread([](int threadNumber) {atomic(threadNumber);}, 5),
		std::thread([](int threadNumber) {atomic(threadNumber);}, 6),
		std::thread([](int threadNumber) {atomic(threadNumber);}, 7),
		std::thread([](int threadNumber) {atomic(threadNumber);}, 8),
		std::thread([](int threadNumber) {atomic(threadNumber);}, 9),
		std::thread([](int threadNumber) {atomic(threadNumber);}, 10),
		std::thread([](int threadNumber) {atomic(threadNumber);}, 11),
		std::thread([](int threadNumber) {atomic(threadNumber);}, 12),
	};

    for (int i=0; i<sizeof(atomicThreads)/sizeof(atomicThreads[0]); i++) {
    	atomicThreads[i].join();
    }
    finish = getMonotonicRealTimeNS();
    printSum();
    std::cout << "--> elapsed time: " << (finish - start) << "ns\n";

    std::cout << "--> Can we make this work faster than the reentrant version (which uses mutexes)?\n";


}
