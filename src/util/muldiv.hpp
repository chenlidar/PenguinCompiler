#ifndef __MULDIV
#define __MULDIV
#include <unordered_map>
using std::unordered_map;
class MULOPT {
public:
    unordered_map<int, int> m1;
    unordered_map<int, std::pair<int, int>> m2;
    unordered_map<int, std::pair<int, int>> m3;
    MULOPT() {
        for (int i = 0; i <= 30; i++) { m1[(1 << i)] = i; }
        for (int i = 1; i <= 30; i++) {
            for (int j = 0; j < i; j++) {
                m2[(1 << i) + (1 << j)] = {i, j};
                m3[(1 << i) - (1 << j)] = {i, j};
            }
        }
    }
    int optTest(int x) {
        if (m1.count(x)) return 1;
        if (m2.count(x)) return 2;
        if (m3.count(x)) return 3;
        return 0;
    }
    int get1(int x) { return m1[x]; }
    std::pair<int, int> get2(int x) { return m2[x]; }
    std::pair<int, int> get3(int x) { return m3[x]; }
};
struct Multiplier {
    unsigned long long m;
    int sh;
    int l;
};
Multiplier chooseMultiplier(unsigned int d, int p);

int MULoptTest(int x);
int MULget1(int x);
std::pair<int, int> MULget2(int x);
std::pair<int, int> MULget3(int x);
#endif