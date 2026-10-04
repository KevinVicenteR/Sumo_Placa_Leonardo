#ifndef PARAMETROS_H
#define PARAMETROS_H

// Ajustados con simulacion/barrido.py --todos para motores de 750 rpm con LiPo 2S
// (7,0–8,4 V), ruedas JSUMO de 3 cm, 300 g y sensores de piso a 3–4,5 cm por delante
// del eje, con el robot colocado en cualquier punto del dohyo, contra un enemigo
// quieto, uno que deambula y uno que embiste. Toleran motores duros o suaves,
// sensores de piso de cualquier polaridad y sensores de piso lentos (probado con
// simulacion/simular.py --retardo-piso 40).
// Si cambian los motores o las ruedas, hay que volver a correr simulacion/barrido.py.

// XMotion se detiene con PWM=0, según StopMotors de la biblioteca oficial.
constexpr bool FrenoActivo = false;

// Velocidades PWM (0-255)
constexpr int VelocidadMaxima = 255;
// Acercamiento al enemigo. Con sensores de piso rápidos 200 gana algo más, pero si
// tardan ~40 ms en ver la línea y el enemigo se aparta (o era un objeto fuera del
// dohyo) el robot llega tarde al borde. 180 queda casi igual de fuerte y cae menos;
// 155 o menos apenas empuja con motores duros.
constexpr int VelocidadAtaque = 120;
// Con el enemigo alineado de frente durante TiempoEmbestida, va con todo
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
constexpr int VelocidadAvance = 80;
constexpr int VelocidadGiroBusqueda = 140;
constexpr int VelocidadPivoteLateral = 170;
// Enemigo visto por un sensor lateral: true = la rueda de ese lado se frena y hace
// de pivote mientras la otra gira a VelocidadRuedaPivote; false = giro en el sitio
// (ruedas en sentidos opuestos a VelocidadPivoteLateral)
constexpr bool GiroLateralEnRueda = true;
constexpr int VelocidadRuedaPivote = 150;

// Sensores de enemigo: 1 = dan HIGH al detectar, -1 = dan LOW al detectar,
// 0 = se averigua al terminar la espera de inicio mirando los dos laterales (el
// enemigo no puede estar a ambos lados a la vez: si los dos «detectan», es que
// los sensores dan LOW al detectar). Con la polaridad equivocada el robot cree
// ver al enemigo en todas partes, ataca a fondo y se sale solo.
constexpr int PolaridadSensoresEnemigo = 1;

// Sensores de piso: al terminar la espera de inicio se mide el negro del dohyo
// y se considera línea cualquier lectura que se aleje de ese valor más de este
// porcentaje (o de UmbralLineaMinimo cuentas ADC si el negro da valores bajos).
// Funciona tanto si el sensor da menos como más sobre el blanco.
constexpr int PorcentajeUmbralLinea = 15;
constexpr int UmbralLineaMinimo = 60;
// La referencia de negro se mantiene fija durante todo el combate.
constexpr int MuestrasCalibracionPiso = 64;
// Aviso de cerca del borde: la lectura de piso se aleja del negro este porcentaje
// del margen de línea. Mientras dura, el robot avanza (también al atacar) a lo sumo a
// VelocidadCercaBorde (100 % equivale a no usarlo). En la simulación apenas cambia
// nada: entre el primer gris y la línea hay muy poca distancia.
constexpr int PorcentajeCercaBorde = 70;
constexpr int VelocidadCercaBorde = 80;

// Tiempos (ms)
constexpr unsigned long TiempoInicioReglamentario = 5000;
// Frenado al ver la línea o al perder al enemigo: cada rueda frena según su
// velocidad estimada (orden filtrada con la inercia TauRuedas), hasta este tiempo a
// toda velocidad; por debajo de VelocidadMinimaFreno no hace falta frenar
constexpr unsigned long TiempoFrenadoRuedas = 300;
constexpr unsigned long TauRuedas = 80;
constexpr int VelocidadMinimaFreno = 20;
constexpr unsigned long TiempoRetroceso = 130;
// Paro en seco cuando el enemigo desaparece en pleno ataque (ms)
constexpr unsigned long TiempoParoPerdida = 150;
constexpr unsigned long TiempoRetrocesoAmbos = 100;
constexpr unsigned long TiempoFrenado = 30;
constexpr unsigned long TiempoGiroEvasion = 150;
constexpr unsigned long TiempoGiroEvasionAmbos = 200;
constexpr unsigned long TiempoGiroEvasionEnSitio = 100;
// Referencia histórica del simulador; el control completa TiempoGiroEvasion.
constexpr unsigned long TiempoMinimoGiroEvasion = 60;
constexpr unsigned long TiempoAvanceBusqueda = 1500;
constexpr unsigned long TiempoGiroBusqueda = 150;
// Motores parados entre el avance y el giro de búsqueda, y entre el giro y el
// avance, para que el robot se asiente (0 = sin pausa)
constexpr unsigned long TiempoPausaBusqueda = 40;
// Avance de búsqueda en pulsos: TiempoPulsoAvance empujando y TiempoPausaPulso
// frenando, para que no coja velocidad y, aunque el sensor de piso tarde en ver la
// línea, pare a tiempo (0 = avance continuo: a 255 se sale casi siempre con un
// sensor lento). Avances largos y giros cortos recorren todo el dohyo.
constexpr unsigned long TiempoPulsoAvance = 20;
constexpr unsigned long TiempoPausaPulso = 40;
// Búsqueda en zigzag: durante el avance, arcos alternos de TiempoArcoZigzag con la
// rueda interior a PorcentajeArcoZigzag de la exterior (100 = avance recto).
// En la simulación, con pulsos sale peor que el avance recto.
constexpr int PorcentajeArcoZigzag = 100;
constexpr unsigned long TiempoArcoZigzag = 0;
// Tiempo con el enemigo alineado en el sensor frontal antes de ir con todo
// (VelocidadEmpuje; también activa ResistirEnBorde). Más corto embiste antes,
// pero si el «enemigo» es un objeto fuera del dohyo o se aparta en el borde,
// llega lanzado.
constexpr unsigned long TiempoEmbestida = 2000;
constexpr unsigned long TiempoRetencionEnemigo = 40;
// Giro hacia un enemigo visto de lado: sigue hasta verlo de frente o hasta este tiempo
constexpr unsigned long TiempoMaxGiroLateral = 400;

// Estimador de borde (EstimadorBorde): predice dónde está el robot dentro del
// dohyo y limita la velocidad hacia delante a la que aún le deja frenar antes
// del borde, aunque los sensores de piso tarden en ver la línea. No limita
// mientras empuja al enemigo (lo tiene delante).
constexpr bool UsarEstimadorBorde = true;
// true: el estimador también limita la embestida si prevé el borde cerca
constexpr bool EstimadorLimitaEmbestida = true;
// Geometría (minisumo reglamentario: 77 cm de diámetro, línea de 2,5 cm)
constexpr float RadioDohyo = 0.385f;
constexpr float AnchoLineaBorde = 0.025f;
constexpr float DistanciaSensorPiso = 0.045f;     // m por delante del eje de ruedas
constexpr float SeparacionSensoresPiso = 0.04f;   // m a cada lado del centro
constexpr float TrochaRuedas = 0.085f;            // m entre ruedas
// Modelo de movimiento identificado en la simulación (750 rpm, ruedas de 3 cm,
// 300 g): cada rueda va a VelocidadRuedaMaxima a PWM 255, no se mueve por debajo
// de ZonaMuertaPwm, entre medias en proporción, y llega con constante de tiempo
// TauRuedaEstimador. En el robot real: medir cuánto avanza en 1 s a PWM 255.
constexpr float VelocidadRuedaMaxima = 1.0f;
constexpr int ZonaMuertaPwm = 40;
constexpr float TauRuedaEstimador = 0.06f;
// Al arrancar: a qué distancia del centro puede estar (el rumbo es desconocido)
constexpr float RadioSalidaEstimado = 0.15f;
// Crecimiento de la incertidumbre: fracción de lo recorrido y de lo girado, y
// rad de deriva del rumbo por metro
constexpr float ErrorDistancia = 0.3f;
constexpr float ErrorGiro = 0.25f;
constexpr float DerivaRumbo = 0.3f;
// m/s de desplazamiento desconocido con el enemigo encima: atacando, empujando
// (TiempoEmbestida) y visto de lado
constexpr float EmpujonAtaque = 0.15f;
constexpr float EmpujonContacto = 0.4f;
constexpr float EmpujonLateral = 0.2f;
// Medida al ver la línea con un solo sensor: rumbo típico respecto al radio
// (rad, ~35°) y su incertidumbre; incertidumbre de la posición tras la medida
constexpr float AnguloLineaLado = 0.6f;
constexpr float IncertAnguloLinea = 0.5f;
constexpr float IncertPosicionLinea = 0.02f;
// Peor caso: cuántas desviaciones típicas se cubren, y a qué distancia del borde
// debe quedar el centro del robot
constexpr float SigmasSeguridad = 1.5f;
constexpr float MargenBorde = 0.06f;
// Frenada: desaceleración (m/s²) y retardo hasta empezar a frenar (s)
constexpr float DesaceleracionFreno = 4.0f;
constexpr float RetardoReaccion = 0.04f;
// Nunca limita por debajo de esto (si no, podría quedarse sin moverse)
constexpr int VelocidadMinimaEstimador = 70;
// Solo se usa la predicción con incertidumbres por debajo de estas (m y rad); si
// no, el robot se comporta como sin estimador (búsqueda en pulsos, sin límite)
constexpr float IncertPosicionConfiable = 0.08f;
constexpr float IncertRumboConfiable = 0.8f;

// Política de combate aprendida por refuerzo en el simulador (TablaPolitica.H,
// generada con simulacion/entrenar_politica.py) en vez de la escrita a mano
constexpr bool UsarPoliticaAprendida = false;

// Movimientos suaves
// Búsqueda: 0 = avance en pulsos y giro en el sitio; 1 = arcos suaves: avanza a
// VelocidadAvance sin pararse, con la rueda interior a PorcentajeArcoBusqueda de
// la exterior, y cambia el lado de la curva cada TiempoArcoBusqueda
constexpr int PatronBusqueda = 0;
constexpr int PorcentajeArcoBusqueda = 50;
constexpr unsigned long TiempoArcoBusqueda = 2000;
// Escape del borde: true = tras frenar, retrocede en arco alejando el frente del
// borde (rueda interior a PorcentajeArcoEscape de la exterior) en un solo
// movimiento; false = retrocede recto, para y gira en el sitio
constexpr bool EscapeEnArco = false;
constexpr int PorcentajeArcoEscape = 0;
constexpr unsigned long TiempoArcoEscape = 400;
constexpr unsigned long TiempoArcoEscapeAmbos = 500;
// Reservado para compatibilidad con el simulador; se completa todo el escape.
constexpr unsigned long TiempoMinimoArcoEscape = 150;
// Rampa: cuánto puede subir la orden de cada motor por milisegundo (0 = sin
// rampa). Bajar la velocidad, frenar, escapar del borde y el paro son inmediatos.
constexpr int RampaPwmPorMs = 2;

// Si la curva fuera más rápida que el ataque, al corregir hacia un enemigo
// visto a 45° la rueda interior iría más rápida y el robot giraría al revés
static_assert(VelocidadCurva <= VelocidadAtaque, "VelocidadCurva debe ser <= VelocidadAtaque");

#endif
