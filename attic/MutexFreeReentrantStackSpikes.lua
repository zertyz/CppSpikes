memory_orders={
    "memory_order_acq_rel",
    "memory_order_acquire",
    "memory_order_consume",
    "memory_order_relaxed",
    "memory_order_release",
    "memory_order_seq_cst",
}

exchange_strenghts={
    "weak",
    "strong"
}

print([[

+=========================================== UPDATE ==========================================+
|| This script is no longer necessary since the solution has been found. Look at it in:      ||
|| https://github.com/zertyz/MTL/blob/master/cpp/stack/UnorderedArrayBasedReentrantStack.hpp ||
+=============================================================================================+

Welcome to the 'MutexFreeReentrantStackSpikes'.
When attempting to implement a mutex-free array based stack,
attempting to do it as simple and efficient as possible,
I could get very close, but wasn't able to achive a fully
reentrant structure capable of passing my, sort of, bossal tests.

Still having a faith in that it could be done, but having totally
lost it in regard that I could fully understand the official C++
documentation on the memory orders, barriers and fences, I
decided to write this brute force script to test all possibilities.

It turned out to be huge -- 6^6*2^2 = 186624 -- which, running
a test per minute, yields 129 days worth of brute force testing!

Meanwhile, I'll reach out the stack overflow community and
other github mutex-free stack implementations to try to find
my answer -- so far I only found far too complicated
implementations which I didn't even bored to test.

The essence of my stack implementation are the 'push' and 'pop'
functions, which, according to the official documentation,
should be implemented as follows:

        inline _BackingArrayElementType* push(unsigned elementId) {
            _BackingArrayElementType* elementSlot = &(backingArray[elementId]);
            elementSlot->next = stackHead.load(memory_order_relaxed);
            while(!stackHead.compare_exchange_weak(elementSlot->next, elementId,
                                                   memory_order_release,
                                                   memory_order_relaxed));
            return elementSlot;
        }

        inline unsigned pop(_BackingArrayElementType** headSlot) {
            unsigned headId = stackHead.load(memory_order_relaxed);
            do {
                // is stack empty?
                if (headId == -1) {
                    *headSlot = nullptr;
                    return -1;
                }
                *headSlot = &(backingArray[headId]);
            } while (!stackHead.compare_exchange_weak(headId, (*headSlot)->next,
                                                      memory_order_release,
                                                      memory_order_relaxed));
            return headId;
        }

And the essence of the tests are creating two stacks -- one for free and the
other for used elements -- both sharing the same backing array:
    1) Repeat several times the push to the used stack:
        - Pop from the free stack, to acquire an element
        - Populate it with regard to it's element id & task number
        - Push it to the used stack
    2) Repeat the same number of times the pop from used stack:
        - Pop from the used stack, to acquire an element
        - Check it with regard to it's element id & task number
        - Push it to the free stack
    3) Do all of the above many, many times in parallel
    4) When all tasks are done, check that free stack have
       all elements in it and that the used stack has none.

... and, this test fails for the documented implementation.

Hense, this brute force friend over here...

  NOTE: when you whish to delete the generated files, use:

        echo -en "Deleting the brute-forcees"; for i in {1..6}; do for j in {1..6}; do for k in {1..6}; do for l in {1..2}; do rm MutexFreeStackSpikes_${i}_${j}_${k}_${l}_* && echo -en '.'; done; done; done; done; echo " Done."

]])

io.write("Generating & compiling 6^6*2^2 (="..(6^6*2^2)..") .cpp files")
io.flush()
-- push
for push_load_order_id, push_load_order in ipairs(memory_orders) do
    for push_success_order_id, push_success_order in ipairs(memory_orders) do
        for push_failure_order_id, push_failure_order in ipairs(memory_orders) do
            for push_exchange_strenght_id, push_exchange_strenght in ipairs(exchange_strenghts) do

                -- pop
                for pop_load_order_id, pop_load_order in ipairs(memory_orders) do
                    for pop_success_order_id, pop_success_order in ipairs(memory_orders) do
                        for pop_failure_order_id, pop_failure_order in ipairs(memory_orders) do
                            for pop_exchange_strenght_id, pop_exchange_strenght in ipairs(exchange_strenghts) do
                
                                local fileName = "MutexFreeStackSpikes_"..
                                                 push_load_order_id.."_"..
                                                 push_success_order_id.."_"..
                                                 push_failure_order_id.."_"..
                                                 push_exchange_strenght_id.."_"..
                                                 pop_load_order_id.."_"..
                                                 pop_success_order_id.."_"..
                                                 pop_failure_order_id.."_"..
                                                 pop_exchange_strenght_id

local code=[[#ifndef MTL_STACK_UnorderedArrayBasedReentrantStack_HPP_
#define MTL_STACK_UnorderedArrayBasedReentrantStack_HPP_

#include <atomic>
#include <iostream>
#include <mutex>
using namespace std;

// linux kernel macros for optimizing branch instructions
#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

namespace mutua::MTL::stack {
    /**
     * UnorderedArrayBasedReentrantStack.hpp
     * =====================================
     * created by luiz, Aug 18, 2019
     *
     * Provides a stack with the following attributes:
     *   - Mutex-free (we use std::atomic pointers) but fully reentrant -- multiple threads may push and pop simultaneously
     *   - All slots are members of a provided array
     *   - Stack entries are "unordered" -- they may be added in any order, independent from the array's natural element order
     *     (this implies that the stack entries must have an "int next" field, serving as a pointer to the next stack member;
     *      this pointer should be declared with 'alignas(64)' to prevent false-sharing performance degradation)
     *
    */
    template <typename _BackingArrayElementType, unsigned _BackingArrayLength>
    class UnorderedArrayBasedReentrantStack {

    public:

        _BackingArrayElementType* backingArray;
        atomic<unsigned>&         stackHead;


        /** initiates a stack manipulation object, receiving as argument pointers to the 'backingArray'
         *  and the atomic 'stackHead' pointer.
         *  Please note that 'stackHead' must be declared using 'alignas(64)' to prevent performance
         *  degradation via the false-sharing phenomenon */
        UnorderedArrayBasedReentrantStack(_BackingArrayElementType* backingArray,
                                          atomic<unsigned>&         stackHead)
            : backingArray (backingArray)
            , stackHead    (stackHead) {

            // start with an empty stack
            stackHead.store(-1, memory_order_release);
        }

        /** pushes into the stack one of the elements of the 'backingArray',
         *  returning a pointer to that element */
        inline _BackingArrayElementType* push(unsigned elementId) {

            _BackingArrayElementType* elementSlot = &(backingArray[elementId]);

            elementSlot->next = stackHead.load(]]..push_load_order..[[);

            // if (elementSlot->next == elementId) {
            //     cout << "?"<<elementId<<"," << flush;
            // }

            while(!stackHead.compare_exchange_]]..push_exchange_strenght..[[(elementSlot->next, elementId,
                                                   ]]..push_success_order..[[,
                                                   ]]..push_failure_order..[[));

            return elementSlot;

        }

        /** pops the head of the stack -- returning a pointer to one of the elements of the 'backingArray'.
         *  Returns 'nullptr' if the stack is empty */
        inline unsigned pop(_BackingArrayElementType** headSlot) {

            unsigned headId = stackHead.load(]]..pop_load_order..[[);
            do {
                // is stack empty?
    			if (headId == -1) {
                    *headSlot = nullptr;
    				return -1;
    			}
                *headSlot = &(backingArray[headId]);
            } while (!stackHead.compare_exchange_]]..pop_exchange_strenght..[[(headId, (*headSlot)->next,
			                                          ]]..pop_success_order..[[,
			                                          ]]..pop_failure_order..[[));
            // if ((*headSlot)->next == headId) {
            //     cout << "̣̣̣̣̣̣̣̣̣¿"<<headId<<"," << flush;
            // }
            return headId;
        }

        /** overload method to be used to pop the head from the stack when one doesn't want to know
         *  what index of the 'backingArray' it is stored at */
        inline _BackingArrayElementType* pop() {
            _BackingArrayElementType* headSlot;
            return pop(&headSlot);
        }

    };
}

#undef likely
#undef unlikely

#endif /* MTL_STACK_UnorderedArrayBasedReentrantStack_HPP_ */

#include <iostream>
#include <atomic>
#include <thread>
#include <cstring>
#include <mutex>


#define DOCS "Starting the ']]..fileName..[[' multithreaded tests\n" \
             "===================================================================\n";


// increase both of these if the reentrancy problem is not exposed
// (disabling compilation optimizations might also help)
#define N_THREADS      12
#define BATCH          4096
#define BACK_AND_FORTH (18264)

#define N_ELEMENTS     (65535)   // should be >= N_THREADS * BATCH


// test section
///////////////

static struct timespec timespec_now;
static inline unsigned long long getMonotonicRealTimeNS() {
    clock_gettime(CLOCK_MONOTONIC, &timespec_now);
    return (timespec_now.tv_sec*1000000000ll) + timespec_now.tv_nsec;
}


// spike vars
/////////////

struct MyStruct {
    double    n;
    unsigned  nSqrt;
};
template <typename _OriginalStruct>
struct StackElement {
    unsigned        next;
    _OriginalStruct original;
};
typedef StackElement<MyStruct> MyStackElement;

MyStackElement         backingArray[N_ELEMENTS];

alignas(64) std::atomic<unsigned>  usedStackHead;
mutua::MTL::stack::UnorderedArrayBasedReentrantStack<MyStackElement, N_ELEMENTS> usedStack(backingArray, usedStackHead);
alignas(64) std::atomic<unsigned>  freeStackHead;
mutua::MTL::stack::UnorderedArrayBasedReentrantStack<MyStackElement, N_ELEMENTS> freeStack(backingArray, freeStackHead);


// spike methods
////////////////

void populateFreeStack() {
    for (unsigned i=0; i<N_ELEMENTS; i++) {
        freeStack.push(i);
    }
}

template<typename _StackType, typename _debug>
void dumpStack(_StackType stack, unsigned expected, string listName, _debug debug) {
    if (debug) std::cout << "Checking list '"<<listName<<"'..." << std::flush;
    unsigned count=0;
    MyStackElement* stackEntry;
    while (true && (count <= (N_ELEMENTS*2))) {
        unsigned poppedId = stack.pop(&stackEntry);
        if (stackEntry == nullptr) {
            break;
        }
        count++;
        if (debug) std::cout << poppedId << ",";
    }
    if (debug) std::cout << "\n";
    std::cout << "Result of checking list '"<<listName<<"': " << (expected == count ? "PASSED":"FAILED") << ": expected="<<expected<<"; counted="<<count<<"\n" << std::flush;
}

void backAndForth(unsigned taskId) {
    MyStackElement* stackEntry;
    MyStruct* myData;

    for (int i=0; i<BATCH; i++) {
        unsigned poppedId;
        while ( (poppedId = freeStack.pop(&stackEntry)) == -1 ) {
            // 'freeStack' prematurely ran out of elements
            std::cout << "'freeStack' prematurely ran out of elements. ABORTING...\n" << std::flush;
            exit(1);
        }
        if (stackEntry->next == poppedId) {
            std::cout << "'freeStack' entry #"<<poppedId<<", after a pop operation, stated he was the ->next of himself (meaning the 'stackHead' is still pointing to him). ABORTING due to REENTRANCY BUG...\n" << std::flush;
            exit(1);
        }

        myData = &(stackEntry->original);
        myData->nSqrt = taskId+poppedId;
        myData->n     = (double)myData->nSqrt*(double)myData->nSqrt;

        usedStack.push(poppedId);

        if (stackEntry->next == poppedId) {
            std::cout << "'usedStack' entry #"<<poppedId<<" stated he is the ->next of himself . ABORTING due to REENTRANCY BUG...\n" << std::flush;
            exit(1);
        }
    }

    for (int i=0; i<BATCH; i++) {
        unsigned poppedId;
        while ( (poppedId = usedStack.pop(&stackEntry)) == -1 ) {
            std::cout << "'usedStack' prematurely ran out of elements. ABORTING...\n" << std::flush;
            exit(1);
        }
        if (stackEntry->next == poppedId) {
            std::cout << "'usedStack' entry #"<<poppedId<<", after a pop operation, stated he was the ->next of himself (meaning the 'stackHead' is still pointing to him). ABORTING due to REENTRANCY BUG...\n" << std::flush;
            exit(1);
        }

        myData = &(stackEntry->original);
        unsigned nSqrt = myData->nSqrt;
        double   n     = myData->n;
        if (n != (double)nSqrt*(double)nSqrt) {
            std::cout << "popped element #" << poppedId << " assert failed:" << n << " != " << (double)nSqrt*(double)nSqrt << ". ABORTING...\n";
            exit(1);
        }

        freeStack.push(poppedId);

        if (stackEntry->next == poppedId) {
            std::cout << "'freeStack' entry #"<<poppedId<<" stated he is the ->next of himself . ABORTING due to REENTRANCY BUG...\n" << std::flush;
            exit(1);
        }
    }
}

int main(void) {

    std::cout << DOCS;

    unsigned long long start = getMonotonicRealTimeNS();
    populateFreeStack();
    std::thread threads[N_THREADS];
    for (unsigned _threadNumber=0; _threadNumber<N_THREADS; _threadNumber++) {
        threads[_threadNumber] = std::thread([](int threadNumber) {
            for (unsigned taskId=0; taskId<BACK_AND_FORTH; taskId++) {
                backAndForth(BACK_AND_FORTH*threadNumber+taskId);
            }
        }, _threadNumber);
    }
    for (int _threadNumber=0; _threadNumber<N_THREADS; _threadNumber++) {
        threads[_threadNumber].join();
    }
    unsigned long long finish = getMonotonicRealTimeNS();

    dumpStack(freeStack, N_ELEMENTS, "freeStack", false);
    dumpStack(usedStack, 0, "usedStack", false);

    std::cout << "--> Executed in " << ((finish-start) / (unsigned long long)1000'000) << "ms.\n";

    return 0;
}
]]

                                local hnd=io.open("cpp.cpp", "w")
                                hnd:write(code)
                                hnd:close()

                                os.execute("echo -en \"`date` -- Compiling ./"..fileName.."\n\" &>>log; clang++ -std=c++17 -O3 -march=native -mtune=native -pthread cpp.cpp -o "..fileName.." &>>log && (echo -en \"`date` -- \"; ./"..fileName..") &>>log")

                            end
                        end
                    end
                end
            end
        end
        io.write(".")
        io.flush()
    end
end

print(" Done.")