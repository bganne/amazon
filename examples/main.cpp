#include <iostream>
#include "Stats.hpp"
#include "utils.hpp"

int main()
{
    fr_benou::Stats<> stats;
    double val;
    while (std::cin >> val) {
        stats.add(val);
    }

    for (auto it = stats.begin(); it != stats.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;

    std::cout << stats.get_p70() << std::endl;

    return 0;
}
