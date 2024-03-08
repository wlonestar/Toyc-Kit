#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <ctime>

#define i64 int64_t
#define f64 double

// std
extern "C" i64 println() { return printf("\n"); }
extern "C" i64 printspace() { return printf(" "); }
extern "C" i64 printi64(i64 x) { return printf("%ld", x); }
extern "C" i64 printi64ln(i64 x) { return printf("%ld\n", x); }
extern "C" i64 printf64(f64 x) { return printf("%lf", x); }
extern "C" i64 printf64ln(f64 x) { return printf("%lf\n", x); }

// math
extern "C" i64 _absi(i64 x) { return std::abs(x); }
extern "C" f64 _absf(f64 x) { return std::abs(x); }
extern "C" f64 _acos(f64 x) { return std::acos(x); }
extern "C" f64 _asin(f64 x) { return std::asin(x); }
extern "C" f64 _atan(f64 x) { return std::atan(x); }
extern "C" f64 _atan2(f64 y, f64 x) { return std::atan2(y, x); }
extern "C" f64 _cos(f64 x) { return std::cos(x); }
extern "C" f64 _sin(f64 x) { return std::sin(x); }
extern "C" f64 _tan(f64 x) { return std::tan(x); }
extern "C" f64 _cosh(f64 x) { return std::cosh(x); }
extern "C" f64 _sinh(f64 x) { return std::sinh(x); }
extern "C" f64 _tanh(f64 x) { return std::tanh(x); }
extern "C" f64 _exp(f64 x) { return std::exp(x); }
extern "C" f64 _log(f64 x) { return std::log(x); }
extern "C" f64 _log10(f64 x) { return std::log10(x); }
extern "C" f64 _pow(f64 x, f64 y) { return std::pow(x, y); }
extern "C" f64 _sqrt(f64 x) { return std::sqrt(x); }
extern "C" f64 _ceil(f64 x) { return std::ceil(x); }
extern "C" f64 _floor(f64 x) { return std::floor(x); }
extern "C" f64 _acosh(f64 x) { return std::acosh(x); }
extern "C" f64 _asinh(f64 x) { return std::asinh(x); }
extern "C" f64 _atanh(f64 x) { return std::atanh(x); }
extern "C" f64 _cbrt(f64 x) { return std::cbrt(x); }
extern "C" f64 _copysign(f64 x, f64 y) { return std::copysign(x, y); }
extern "C" f64 _erf(f64 x) { return std::erf(x); }
extern "C" f64 _erf2(f64 x) { return std::erfc(x); }
extern "C" f64 _exp2(f64 x) { return std::exp2(x); }
extern "C" f64 _expm1(f64 x) { return std::expm1(x); }
extern "C" f64 _fdim(f64 x, f64 y) { return std::fdim(x, y); }
extern "C" f64 _fma(f64 x, f64 y, f64 z) { return std::fma(x, y, z); }
extern "C" f64 _fmax(f64 x, f64 y) { return std::fmax(x, y); }
extern "C" f64 _fmin(f64 x, f64 y) { return std::fmin(x, y); }
extern "C" f64 _hypot(f64 x, f64 y) { return std::hypot(x, y); }
extern "C" f64 _lgamma(f64 x) { return std::lgamma(x); }
extern "C" f64 _log1p(f64 x) { return std::log1p(x); }
extern "C" f64 _log2(f64 x) { return std::log2(x); }
extern "C" f64 _logb(f64 x) { return std::logb(x); }
extern "C" f64 _nearbyint(f64 x) { return std::nearbyint(x); }
extern "C" f64 _nextafter(f64 x, f64 y) { return std::nextafter(x, y); }
extern "C" f64 _remainder(f64 x, f64 y) { return std::remainder(x, y); }
extern "C" f64 _rint(f64 x) { return std::rint(x); }
extern "C" f64 _round(f64 x) { return std::round(x); }
extern "C" f64 _scalbln(f64 x, i64 y) { return std::scalbln(x, y); }
extern "C" f64 _tgamma(f64 x) { return std::tgamma(x); }
extern "C" f64 _trunc(f64 x) { return std::trunc(x); }

// time
extern "C" void seed(i64 x) { srand(x); }
extern "C" i64 random() { return rand(); }
extern "C" i64 get_time_ns() {
  auto current = std::chrono::high_resolution_clock::now();
  auto duration = current.time_since_epoch();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
}
extern "C" i64 get_time_us() {
  auto current = std::chrono::high_resolution_clock::now();
  auto duration = current.time_since_epoch();
  return std::chrono::duration_cast<std::chrono::microseconds>(duration)
      .count();
}
extern "C" i64 get_time_ms() {
  auto current = std::chrono::high_resolution_clock::now();
  auto duration = current.time_since_epoch();
  return std::chrono::duration_cast<std::chrono::milliseconds>(duration)
      .count();
}
