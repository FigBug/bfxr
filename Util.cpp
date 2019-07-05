#include "Util.h"

double uniformRandom()
{
    static std::random_device rd;
    static std::mt19937 e2 {rd()};
    static std::uniform_real_distribution<> dist {0, 1};

    return dist (e2);
}
