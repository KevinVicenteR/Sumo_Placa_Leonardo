#ifndef PARAMETROS_H
#define PARAMETROS_H

// Ajustados con simulacion/barrido.py --todos para motores de 750 rpm con LiPo 2S
// (7,0–8,4 V), ruedas JSUMO de 3 cm, 300 g y sensores de piso a 3–4,5 cm por delante
// del eje, con el robot colocado en cualquier punto del dohyo, contra un enemigo
// quieto, uno que deambula y uno que embiste. Toleran motores duros o suaves y
// sensores de piso de cualquier polaridad.
// Si cambian los motores o las ruedas, hay que volver a correr simulacion/barrido.py.

// Velocidades PWM (0-255)
constexpr int VelocidadMaxima = 255;
// A esta velocidad el robot no alcanzaría a frenar si llegara lanzado a la línea,
// pero al atacar el enemigo está delante: si lo esquiva, el paro frena antes de
// llegar al borde. En la simulación 200 gana bastantes más combates que 155 sin
// caerse más (con motores duros, 155 apenas logra empujar).
constexpr int VelocidadAtaque = 50;
// Tras atacar sin interrupción durante TiempoEmbestida ya está encima del enemigo
// y empuja a esta velocidad. En la simulación 255 no mejora a 200 (las ruedas ya
// patinan), pero queda para probar en el robot real.
constexpr int VelocidadEmpuje = 200;
// Enemigo de frente y también en un sensor de 45°: la rueda de ese lado va a
// este porcentaje de la otra para centrarlo sin dejar de empujar
constexpr int PorcentajeAjuste = 85;
// Enemigo empujando de frente al llegar a la línea: true = sigue empujando a
// fondo en vez de frenar y retroceder (retroceder solo le ayuda a sacarlo).
// En la simulación sale peor: si el enemigo se aparta en el último momento, el
// robot sigue de largo. Desactivado.
constexpr bool ResistirEnBorde = false;
constexpr int VelocidadCurva = 100;
constexpr int VelocidadRetroceso = 140;
constexpr int VelocidadGiroEvasion = 150;
constexpr int VelocidadAvance = 60;
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
constexpr unsigned long TiempoMinimoGiroEvasion = 60;
constexpr unsigned long TiempoAvanceBusqueda = 200;
constexpr unsigned long TiempoGiroBusqueda = 600;
// Motores parados entre el avance y el giro de búsqueda, y entre el giro y el
// avance, para que el robot se asiente (0 = sin pausa)
constexpr unsigned long TiempoPausaBusqueda = 40;
// Ataque continuo necesario para pasar a VelocidadEmpuje (y para ResistirEnBorde).
// Debe ser mayor que lo que tarda en recorrer el alcance de los sensores de enemigo
// (40 cm a ~0,6 m/s): así un objeto fuera del dohyo nunca lo activa antes de que
// aparezca la línea.
constexpr unsigned long TiempoEmbestida = 700;
constexpr unsigned long TiempoRetencionEnemigo = 40;
// Giro hacia un enemigo visto de lado: sigue hasta verlo de frente o hasta este tiempo
constexpr unsigned long TiempoMaxGiroLateral = 400;

#endif
