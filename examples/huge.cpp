#include <iostream>
#include <chrono>
#include <thread>
#include <cstdlib>
#include "Stats.hpp"
#include "utils.hpp"

#define SAMPLES         100000
#define DURATION        90

int main()
{
    fr_benou::Stats<> stats;

    for (int i=0; i<DURATION; i++) {
        for (int j=0; j<SAMPLES; ++j) {
            stats.add(i);
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << stats.get_p70() << std::endl;

    return 0;
}
