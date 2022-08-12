#include "singleton.hpp"
#include "muldiv.hpp"
#include <cmath>
#include <assert.h>
int MULoptTest(int x) {
    if (Singleton::instance().mulopt.m1.count(x)) return 1;
    if (Singleton::instance().mulopt.m2.count(x)) return 2;
    if (Singleton::instance().mulopt.m3.count(x)) return 3;
    return 0;
}
int MULget1(int x) { return Singleton::instance().mulopt.m1[x]; }
std::pair<int, int> MULget2(int x) { return Singleton::instance().mulopt.m2[x]; }
std::pair<int, int> MULget3(int x) { return Singleton::instance().mulopt.m3[x]; }
Multiplier chooseMultiplier(unsigned int d, int p) {
    static const int N = 32;
    assert(d != 0);
    assert(p >= 1 && p <= N);
    int l = ceil(log2(d));
    int sh = l;
    unsigned long long low = (1llu << (N + sh)) / d;
    unsigned long long high = ((1llu << (N + sh)) + (1llu << (N + sh - p))) / d;
    while ((low >> 1) < (high >> 1) && sh > 0) { low >>= 1, high >>= 1, sh--; }
    return {high, sh, l};
}