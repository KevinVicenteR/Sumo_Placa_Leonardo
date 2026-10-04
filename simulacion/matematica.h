// Funciones matemáticas de la simulación. En consola son las de <cmath>; en la
// página web (WebAssembly sin biblioteca estándar) las aporta JavaScript (Math).

#ifndef SIMULACION_MATEMATICA_H
#define SIMULACION_MATEMATICA_H

#ifdef __wasm__

#define IMPORTAR(nombre) __attribute__((import_module("env"), import_name(nombre)))

extern "C" {
IMPORTAR("sin") double js_sin(double);
IMPORTAR("cos") double js_cos(double);
IMPORTAR("atan2") double js_atan2(double, double);
IMPORTAR("log") double js_log(double);
}

namespace mat {
inline double sin(double x) { return js_sin(x); }
inline double cos(double x) { return js_cos(x); }
inline double atan2(double y, double x) { return js_atan2(y, x); }
inline double log(double x) { return js_log(x); }
inline double sqrt(double x) { return __builtin_sqrt(x); }
inline double fabs(double x) { return __builtin_fabs(x); }
inline double hypot(double x, double y) { return __builtin_sqrt(x * x + y * y); }
inline double remainder(double x, double y) {
    const double n = __builtin_rint(x / y);
    return x - n * y;
}
}  // namespace mat

#else

#include <cmath>

namespace mat {
using std::atan2;
using std::cos;
using std::fabs;
using std::log;
using std::remainder;
using std::sin;
using std::sqrt;
inline double hypot(double x, double y) { return std::sqrt(x * x + y * y); }
}  // namespace mat

#endif

#endif
