#include <stdint.h>
#include <stdio.h>

extern "C" void println() { printf("\n"); }
extern "C" void printSpace() { printf(" "); }

extern "C" void printI64(int64_t val) { printf("%ld", val); }
extern "C" void printI64ln(int64_t val) { printf("%ld\n", val); }

extern "C" void printF64(double val) { printf("%lf", val); }
extern "C" void printF64ln(double val) { printf("%lf\n", val); }
