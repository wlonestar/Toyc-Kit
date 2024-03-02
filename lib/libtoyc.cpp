#include <stdint.h>
#include <stdio.h>

extern "C" void printI64(int64_t val) { printf("%ld\n", val); }

extern "C" void printF64(double val) { printf("%lf\n", val); }
