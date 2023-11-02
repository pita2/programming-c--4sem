#include <iostream>
#include "Measurer.h"

int main(int argc, const char* argv[]) {


    std::cout << "Input number of tasks: ";
    unsigned int numberOfTasks;
    std::cin >> numberOfTasks;

    unsigned int genTime;
    std::cout << "Input generation time: ";
    std::cin >> genTime;

    unsigned int compTime;
    std::cout << "Input computation time: ";
    std::cin >> compTime;

    //Measuring distribution
    auto ans = threadPerf::measureDistribution(numberOfTasks, genTime, compTime);


    auto result = ans.get_min_element();
    std::cout << "\nBest configuration:\n"
        << "Generators: " << result.numberOfConsumers
        << "\nWorkers: " << result.numberOfProducers
        << "\nTime: " << result.count() << " microseconds" << '\n';
}