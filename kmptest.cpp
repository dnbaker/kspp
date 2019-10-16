#include "kmp.h"
#include <cstdio>

int main() {
    kmp::Pool<int> pool;
    int *i = pool.calloc();
    *i = 1337;
    pool.free(i);
    i = pool.calloc();
    std::fprintf(stderr, "i: %i\n", *i);
    std::free(i);
}
