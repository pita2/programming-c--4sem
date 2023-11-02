
#ifndef HW_2_4__MEASURER_H
#define HW_2_4__MEASURER_H

#include <chrono>
#include <thread>
#include <utility>
#include <vector>
#include <algorithm>
#include <iostream>
#include "ThreadsafeQueue.h"

namespace threadPerf
{

    struct Task {
        using Time = std::chrono::microseconds;
        Time creationTime;
        Time runTime;

        Task() = default;
        Task(unsigned int creation, unsigned int run): Task{std::chrono::microseconds(creation), std::chrono::microseconds(run)} {}
        Task(Time creation, Time run): creationTime{creation}, runTime{run} {
            std::this_thread::sleep_for(creationTime);
        }

        void operator()() {
            std::this_thread::sleep_for(runTime);
        }
    };

    void runTasks(unsigned int numberOfTasks, unsigned int generationTime, unsigned int runTime, unsigned int numberOfConsumers, unsigned int numberOfProducers) {
        threadsafe_queue<Task> queue;
        std::vector<std::thread> generators;
        generators.reserve(numberOfConsumers);
        std::vector<std::thread> workers;
        workers.reserve(numberOfProducers);
        const std::atomic<unsigned int> maxNumber = numberOfTasks;
        std::atomic<unsigned int> counter = 0;

        for (int i = 0; i < numberOfConsumers; ++i) {
            generators.emplace_back([&]() {
                while (counter < maxNumber) {
                    queue.push({generationTime, runTime});
                    ++counter;
                }
            });
        }

        std::atomic<int> counter2 = numberOfTasks;
        for (int i = 0; i < numberOfProducers; ++i) {
            workers.emplace_back([&]() {
                while (counter2 > 0) {
                    Task task;
                    auto done = queue.try_pop(task);
                    if (done) task();
                    --counter2;
                }
            });
        }
        for (auto &x: generators)
            x.join();
        for (auto &x: workers)
            x.join();
    }

    struct Measurment {
        std::chrono::nanoseconds time;
        int numberOfConsumers;
        int numberOfProducers;

        auto count() const noexcept(noexcept(std::chrono::duration_cast<std::chrono::microseconds>(time))) {
            return std::chrono::duration_cast<std::chrono::microseconds>(time).count();
        }
    };

    struct MeasurmentResult {
        std::vector<Measurment> result;
        auto get_min_element() const noexcept {
            return *std::min_element(result.begin(), result.end(), [](auto rhs, auto lhs) { return rhs.time < lhs.time; });
        }
        void print() const noexcept {
            for (const auto& x : result)
                std::cout << x.count() << " mcrs Producers: " << x.numberOfConsumers << " Consumers: " << x.numberOfProducers << '\n';

        }
    };

    auto measureDistribution(unsigned int numberOfTasks, unsigned int generationTime, unsigned int runTime) {
        std::vector<Measurment> result;
        result.reserve(std::thread::hardware_concurrency());
        for (int i = 1; i < std::thread::hardware_concurrency() - 1; ++i)
        {
            int numberOfConsumers = i;
            int numberOfProducers = std::thread::hardware_concurrency() - i - 1;
            auto start = std::chrono::high_resolution_clock::now();

            runTasks(numberOfTasks, generationTime, runTime, numberOfConsumers, numberOfProducers);

            auto end = std::chrono::high_resolution_clock::now();
            result.push_back({end - start, numberOfConsumers, numberOfProducers});
        }

        return MeasurmentResult{std::move(result)};
    }
}

#endif //HW_2_4__MEASURER_H
