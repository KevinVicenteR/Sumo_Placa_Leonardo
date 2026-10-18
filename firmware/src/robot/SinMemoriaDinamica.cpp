// SinMemoriaDinamica.cpp — El robot nunca destruye objetos: todas las piezas se
// crean una vez en main.cpp y viven hasta apagarlo. Las interfaces tienen
// destructor virtual y eso arrastra operator delete -> free -> malloc (unos 600
// bytes de Flash). Un operator delete vacío los elimina sin cambiar nada.
#if defined(ARDUINO_ARCH_AVR)
#include <stddef.h>

void operator delete(void*) noexcept {}
void operator delete(void*, size_t) noexcept {}
#endif
