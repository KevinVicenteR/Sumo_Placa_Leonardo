#ifndef PARAMETROS_H
#define PARAMETROS_H

// Ajustados con simulacion/barrido.py para motores de 750 rpm con LiPo 2S (7,0–8,4 V),
// ruedas JSUMO de 3 cm, 300 g y sensores de piso a 3–4,5 cm por delante del eje, con
// el robot colocado en cualquier punto del dohyo. Toleran motores duros o suaves y
// sensores de piso de cualquier polaridad.
// Si cambian los motores o las ruedas, hay que volver a correr simulacion/barrido.py.

// Velocidades PWM (0-255)
constexpr int VelocidadMaxima = 255;
// Por encima de ~0,65 m/s reales el robot no alcanza a frenar entre que el
// sensor de piso ve la línea y que el centro de masa llega al borde
constexpr int VelocidadAtaque = 255;
constexpr int VelocidadCurva = 100;
constexpr int VelocidadRetroceso = 140;
constexpr int VelocidadGiroEvasion = 150;
constexpr int VelocidadAvance = 140;
constexpr int VelocidadGiroBusqueda = 140;
constexpr int VelocidadPivoteLateral = 170;
// Enemigo visto por un sensor lateral: true = la rueda de ese lado se frena y hace
// de pivote mientras la otra gira a VelocidadRuedaPivote; false = giro en el sitio
// (ruedas en sentidos opuestos a VelocidadPivoteLateral)
constexpr bool GiroLateralEnRueda = true;
constexpr int VelocidadRuedaPivote = 150;

// Sensores de piso: al terminar la espera de inicio se mide el negro del dohyo
// y se considera línea cualquier lectura que se aleje de ese valor más de este
// porcentaje (o de UmbralLineaMinimo cuentas ADC si el negro da valores bajos).
// Funciona tanto si el sensor da menos como más sobre el blanco.
constexpr int PorcentajeUmbralLinea = 30;
constexpr int UmbralLineaMinimo = 150;
// Si un sensor ve línea sin parar más que esto, se asume mal calibrado (ms)
constexpr unsigned long TiempoLineaSospechosa = 250;
constexpr int MuestrasCalibracionPiso = 64;

// Tiempos (ms)
constexpr unsigned long TiempoInicioReglamentario = 5000;
// Frenado al ver la línea o al perder al enemigo: cada rueda frena según su
// velocidad estimada (orden filtrada con la inercia TauRuedas), hasta este tiempo a
// toda velocidad; por debajo de VelocidadMinimaFreno no hace falta frenar
constexpr unsigned long TiempoFrenadoRuedas = 70;
constexpr unsigned long TauRuedas = 80;
constexpr int VelocidadMinimaFreno = 20;
constexpr unsigned long TiempoRetroceso = 130;
// Paro en seco cuando el enemigo desaparece en pleno ataque (ms)
constexpr unsigned long TiempoParoPerdida = 80;
constexpr unsigned long TiempoRetrocesoAmbos = 100;
constexpr unsigned long TiempoFrenado = 30;
constexpr unsigned long TiempoGiroEvasion = 150;
constexpr unsigned long TiempoGiroEvasionAmbos = 200;
constexpr unsigned long TiempoGiroEvasionEnSitio = 100;
constexpr unsigned long TiempoAsentarGiro = 200;
constexpr unsigned long TiempoMinimoGiroEvasion = 60;
constexpr unsigned long TiempoAvanceBusqueda = 200;
constexpr unsigned long TiempoGiroBusqueda = 600;
constexpr unsigned long TiempoRetencionEnemigo = 40;
// Giro hacia un enemigo visto de lado: sigue hasta verlo de frente o hasta este tiempo
constexpr unsigned long TiempoMaxGiroLateral = 400;

#endif
