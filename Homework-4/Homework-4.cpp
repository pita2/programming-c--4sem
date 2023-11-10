#include <iostream>
#include <chrono>
#include <random>
#include <thread>
#include "BlockingStack.h"
#include "LockfreeStack.h"
#include <vector>


std::atomic<int> numberOfTasks;

void produce(auto& stack) {
    for (;;) {
        int n = numberOfTasks.load();
        if (n < 0)
            break;
        while (!numberOfTasks.compare_exchange_weak(n, n - 1, std::memory_order_relaxed)) {
            if (n < 0) return;
            std::this_thread::yield();
        }
        stack.push(std::move(n));
    }
}

void produceLockfree(auto& stack) {
    for (;;) {
        int n = numberOfTasks.load();
        if (n < 0)
            break;
        while (!numberOfTasks.compare_exchange_weak(n, n - 1, std::memory_order_relaxed)) {
            if (n < 0) return;
            std::this_thread::yield();
        }
        stack.push(n);
    }
}

std::atomic<int> sum = 0;

void consume(auto& stack) {
    for (;;) {
        int n = numberOfTasks.load();
        if (n < 0 && stack.empty())
            break;
        int tmp = 0;
        stack.pop(tmp);
        sum += tmp;
    }
}

void consumeLockfree(auto& stack) {
    for (;;) {
        int n = numberOfTasks.load();
        if (n < 0 && stack.empty())
            break;
        std::shared_ptr<int> tmp = stack.pop();
        if (tmp) sum += *tmp;
    }
}

int main(int argc, char* argv[]) {

    int NTASKS_KEEPER;
    int numberOfProducers;
    int numberOfConsumers;
    std::cin >> NTASKS_KEEPER >> numberOfProducers >> numberOfConsumers;
    numberOfTasks = NTASKS_KEEPER;


    //Measuring blocking stack
    const auto startBlockingStack = std::chrono::high_resolution_clock::now();
    {
        threadPerf::BlockingStack<int> blockingStack{};


        std::vector<std::thread> producers;
        std::vector<std::thread> consumers;
        for (int i = 0; i < numberOfProducers; ++i)
            producers.emplace_back(produce<threadPerf::BlockingStack<int>>, std::ref(blockingStack));
        for (int i = 0; i < numberOfConsumers; ++i)
            consumers.emplace_back(consume<threadPerf::BlockingStack<int>>, std::ref(blockingStack));
        for (auto& x : producers)
            x.join();
        for (auto& x : consumers)
            x.join();
    }
    const auto endBlockingStack = std::chrono::high_resolution_clock::now();
    const auto timeBlocking = endBlockingStack - startBlockingStack;
    auto blockingSum = sum.load();
    sum = 0;
    numberOfTasks = NTASKS_KEEPER;


    const auto startLockfreeStack = std::chrono::high_resolution_clock::now();
    {
        threadPerf::LockfreeStack<int> lockfreeStack{};
        std::vector<std::thread> producers;
        std::vector<std::thread> consumers;
        for (int i = 0; i < numberOfProducers; ++i)
            producers.emplace_back(produceLockfree<threadPerf::LockfreeStack<int>>, std::ref(lockfreeStack));
        for (int i = 0; i < numberOfConsumers; ++i)
            consumers.emplace_back(consumeLockfree<threadPerf::LockfreeStack<int>>, std::ref(lockfreeStack));
        for (auto& x : producers)
            x.join();
        for (auto& x : consumers)
            x.join();
    }
    const auto endLockfreeStack = std::chrono::high_resolution_clock::now();
    const auto timeLockfree = endLockfreeStack - startLockfreeStack;


    std::cout << "Blocking stack: " << std::chrono::duration_cast<std::chrono::microseconds>(timeBlocking) << '\n'
        << "Lockfree stack: " << std::chrono::duration_cast<std::chrono::microseconds>(timeLockfree) << '\n'
        << "Blocking sum: " << blockingSum << "\n" << "Lockfree sum: " << sum.load() << "\n";
}
