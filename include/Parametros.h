// Parametros.h — Todos los valores ajustables del robot (sumo X3, motores de 400 rpm)
// Valores elegidos con simulacion/optimizar.py (batería de combates con todos los
// rounds, rivales y factores externos). Hay que comprobarlos en el robot real.
// Velocidades en PWM (0-255), tiempos en milisegundos y distancias en metros,
// salvo que se indique otra cosa. Los scripts de simulacion/ cambian estos
// valores por su nombre: si se renombra uno, hay que actualizar los scripts.
#ifndef PARAMETROS_H
#define PARAMETROS_H

// 1. Velocidades de movimiento

// Sentido de giro de cada motor: true invierte el cableado (velocidad positiva
// = avanzar). Ajustado en la prueba física con los motores de 400 rpm del X3.
constexpr bool InvertirMotorIzq = true;
constexpr bool InvertirMotorDer = true;
// false = para frenar pone PWM 0 en vez de dar contramando (marcha atrás).
// En el X3, el contramando tras un choque lo hacía retroceder.
constexpr bool FrenoActivo = false;

// Límite absoluto que se manda a cada motor.
constexpr int VelocidadMaxima = 255;
// Contramando al frenar: se limita para no dar golpes a PWM 255.
constexpr int VelocidadFreno = 140;

// --- Ataque ---
// Velocidad al empezar a atacar; sube poco a poco hasta VelocidadEmpuje
// durante TiempoEmbestida mientras tenga al rival de frente.
constexpr int VelocidadAtaque = 130;
constexpr int VelocidadEmpuje = 255;
// Rival en los tres sensores delanteros a la vez: es un robot cercano (un reflejo
// no ocupa tres sensores), así que ataca ya a esta velocidad (0 = sin efecto).
constexpr int VelocidadAtaqueDirecto = 0;
// Rival de frente y también en un sensor de 45°: la rueda de ese lado va a
// este porcentaje de la otra para centrarlo sin dejar de empujar.
constexpr int PorcentajeAjuste = 85;
// Rival visto solo por un sensor de 45°: true = la rueda de ese lado se para y
// hace de pivote; false = curva con las dos ruedas (la interior a VelocidadCurva).
constexpr bool Corregir45EnPivote = true;
constexpr int VelocidadCurva = 85;
// true = al llegar a la línea empujando al rival, sigue empujando en vez de
// retroceder (solo tras TiempoEmbestida empujándolo de frente).
constexpr bool ResistirEnBorde = false;

// --- Rival visto por un sensor lateral ---
// true = la rueda de ese lado se frena y la otra gira a VelocidadRuedaPivote;
// false = giro en el sitio a VelocidadPivoteLateral.
constexpr bool GiroLateralEnRueda = false;
constexpr int VelocidadRuedaPivote = 180;
constexpr int VelocidadPivoteLateral = 180;

// --- Evasión del borde ---
constexpr int VelocidadRetroceso = 255;
constexpr int VelocidadGiroEvasion = 75;

// --- Búsqueda del rival ---
constexpr int VelocidadAvance = 80;
constexpr int VelocidadGiroBusqueda = 75;

// 2. Sensores de enemigo

// Polaridad: 0 = se aprende en modo diagnóstico midiendo sin objetos delante;
// 1 = detectan en HIGH; -1 = detectan en LOW.
constexpr int PolaridadSensoresEnemigo = 0;
// Polaridad usada en combate (nunca se recalibra con el rival delante).
// Un bit por sensor, en orden: lateral izq, 45° izq, central, 45° der, lateral
// der. 31 = los cinco detectan en HIGH.
constexpr int MascaraEnemigosActivosHigh = 31;
// Una detección se mantiene estos ms tras perderla (tolera parpadeos).
constexpr unsigned long TiempoRetencionEnemigo = 40;
// Una detección nueva solo cuenta tras verse estos ms seguidos (0 = al instante).
constexpr unsigned long ConfirmacionSensorEnemigo = 0;

// 3. Sensores de piso

// El negro se mide al empezar el combate. Se considera línea blanca cuando la
// lectura se aleja del negro más de este porcentaje (y como mínimo
// UmbralLineaMinimo), sin importar si el sensor sube o baja sobre el blanco.
constexpr int PorcentajeUmbralLinea = 15;
constexpr int UmbralLineaMinimo = 60;
constexpr int MuestrasCalibracionPiso = 64;
// Referencias de negro hasta que se calibra (la calibración las sustituye).
constexpr int NegroPisoIzquierdo = 994;
constexpr int NegroPisoDerecho = 972;
// Aviso de "cerca del borde": la lectura se aleja del negro este porcentaje del
// margen de línea. Mientras dura, el avance se limita a VelocidadCercaBorde.
constexpr int PorcentajeCercaBorde = 70;
constexpr int VelocidadCercaBorde = 100;

// 4. Tiempos generales

// Espera inicial del modo diagnóstico (el de combate usa el módulo de arranque).
constexpr unsigned long TiempoInicioReglamentario = 5000;

// --- Frenado al perder al rival ---
// Cada rueda frena según su velocidad estimada (la orden filtrada con la
// inercia TauRuedas). Por debajo de VelocidadMinimaFreno no hace falta frenar.
constexpr unsigned long TiempoFrenadoRuedas = 80;
constexpr unsigned long TauRuedas = 80;
constexpr int VelocidadMinimaFreno = 20;
// Duración máxima del frenado cuando el rival desaparece en pleno ataque.
constexpr unsigned long TiempoParoPerdida = 150;

// --- Evasión del borde ---
// Solo la usan los tests y la simulación como referencia.
constexpr unsigned long TiempoRetroceso = 180;
// Retroceso mínimo con los dos sensores en blanco / con uno solo.
constexpr unsigned long TiempoRetrocesoAmbos = 200;
constexpr unsigned long TiempoRetrocesoUnSensor = 90;
constexpr int VelocidadRetrocesoUnSensor = 118;
// Tiempo viendo negro seguido antes de dejar de retroceder.
constexpr unsigned long TiempoSeparacionBorde = 90;
constexpr unsigned long TiempoSeparacionUnSensor = 50;
// Tope de toda la maniobra (no hay sensores traseros que avisen).
constexpr unsigned long TiempoMaximoRecuperacionBorde = 700;
// Pausa con motores parados entre retroceso y giro, y después del giro.
constexpr unsigned long TiempoFrenado = 80;
constexpr unsigned long TiempoAsentamientoEvasion = 100;
// Duración del giro con un sensor / con los dos sensores en blanco.
constexpr unsigned long TiempoGiroEvasion = 240;
constexpr unsigned long TiempoGiroEvasionAmbos = 300;
// Retroceder solo es seguro si el robot venía avanzando (el borde queda delante).
// Si ve la línea girando o parado (por ejemplo, pegado al borde al empezar el
// round 3), detrás también puede estar el borde: entonces gira en el sitio para
// salir, que no mueve su centro. Umbral: velocidad media estimada en PWM.
constexpr int AvanceMinimoParaRetroceder = 50;
// Retroceso extra (ms) si llega a la línea a velocidad máxima (proporcional a la
// velocidad). Con poco agarre el robot sigue deslizando hacia fuera al frenar y,
// con el morro fuera del dohyo, los sensores de piso pueden leer "negro".
constexpr unsigned long RetrocesoExtraAVelocidadMaxima = 300;
// Tras evadir con un solo sensor, sale despacio durante este tiempo.
constexpr unsigned long TiempoSalidaSuaveUnSensor = 500;
constexpr int VelocidadSalidaUnSensor = 90;

// --- Búsqueda en el sitio (PatronBusqueda = 0) ---
// Ciclo: avanza, pausa, gira, pausa.
constexpr unsigned long TiempoAvanceBusqueda = 1500;
constexpr unsigned long TiempoGiroBusqueda = 150;
constexpr unsigned long TiempoPausaBusqueda = 100;
// Avance en pulsos: TiempoPulsoAvance empujando y TiempoPausaPulso parado
// (0 = avance continuo).
constexpr unsigned long TiempoPulsoAvance = 0;
constexpr unsigned long TiempoPausaPulso = 40;
// Zigzag durante el avance: arcos alternos de TiempoArcoZigzag con la rueda
// interior a PorcentajeArcoZigzag de la exterior (100 = recto).
constexpr int PorcentajeArcoZigzag = 100;
constexpr unsigned long TiempoArcoZigzag = 0;

// --- Ataque ---
// Tiempo de la subida de VelocidadAtaque a VelocidadEmpuje. Vuelve a empezar
// si pierde al rival de frente o evade el borde.
constexpr unsigned long TiempoEmbestida = 2500;
// Giro hacia un rival visto de lado: sigue hasta verlo de frente o este tiempo.
constexpr unsigned long TiempoMaxGiroLateral = 400;

// 5. Estimador de borde
// Calcula dónde está el robot dentro del dohyo y limita la velocidad hacia
// delante a la que aún le deja frenar antes del borde, aunque los sensores de
// piso tarden en ver la línea.
constexpr bool UsarEstimadorBorde = true;
// true = también limita la embestida si prevé el borde cerca.
constexpr bool EstimadorLimitaEmbestida = true;
// Geometría: dohyo de 70 cm de diámetro (el reglamentario mide 77 cm) y línea de 2,5 cm.
constexpr float RadioDohyo = 0.35f;
constexpr float AnchoLineaBorde = 0.025f;
constexpr float DistanciaSensorPiso = 0.045f;     // por delante del eje de ruedas
constexpr float SeparacionSensoresPiso = 0.04f;   // a cada lado del centro
constexpr float TrochaRuedas = 0.085f;            // distancia entre ruedas
// Modelo de las ruedas: VelocidadRuedaMaxima (m/s) a PWM 255, parada por
// debajo de ZonaMuertaPwm y constante de tiempo TauRuedaEstimador (s).
constexpr float VelocidadRuedaMaxima = 0.53f;  // X3: 400 rpm (1,0 m/s × 400/750)
constexpr int ZonaMuertaPwm = 40;
constexpr float TauRuedaEstimador = 0.06f;
// Al arrancar puede estar hasta a esta distancia del centro, con rumbo desconocido.
constexpr float RadioSalidaEstimado = 0.15f;
// Con rutina de inicio se sabe dónde empieza: rounds 1 y 2 junto al centro (a lo
// sumo a IncertSalidaCentro), round 3 a RadioSalidaRound3 del centro mirándolo
// (con IncertSalidaRound3 de error y IncertRumboRound3 rad de rumbo).
constexpr float IncertSalidaCentro = 0.06f;
constexpr float RadioSalidaRound3 = 0.295f;
constexpr float IncertSalidaRound3 = 0.03f;
constexpr float IncertRumboRound3 = 0.3f;
// Cuánto crece la incertidumbre: fracción de lo recorrido, de lo girado y
// deriva del rumbo (rad) por metro.
constexpr float ErrorDistancia = 0.3f;
constexpr float ErrorGiro = 0.25f;
constexpr float DerivaRumbo = 0.3f;
// Desplazamiento desconocido (m/s) por empujones: atacando, empujando y de lado.
constexpr float EmpujonAtaque = 0.15f;
constexpr float EmpujonContacto = 0.4f;
constexpr float EmpujonLateral = 0.2f;
// Al ver la línea con un solo sensor: ángulo típico respecto al radio (rad) y
// su incertidumbre; incertidumbre de la posición tras la medida.
constexpr float AnguloLineaLado = 0.6f;
constexpr float IncertAnguloLinea = 0.5f;
constexpr float IncertPosicionLinea = 0.02f;
// Peor caso: desviaciones típicas que se cubren y distancia mínima al borde.
constexpr float SigmasSeguridad = 1.0f;
constexpr float MargenBorde = 0.09f;
// Frenada: desaceleración (m/s²) y retardo hasta empezar a frenar (s).
constexpr float DesaceleracionFreno = 4.0f;
constexpr float RetardoReaccion = 0.04f;
// Nunca limita por debajo de esto (si no, podría quedarse quieto).
constexpr int VelocidadMinimaEstimador = 100;
// Solo se fía de la predicción con incertidumbres menores que estas.
constexpr float IncertPosicionConfiable = 0.08f;
constexpr float IncertRumboConfiable = 0.8f;

// 6. Estrategia

// true = usa la política aprendida en el simulador (TablaPolitica.h) en vez de
// las reglas escritas a mano.
constexpr bool UsarPoliticaAprendida = false;

// 7. Suavidad de los movimientos

// Búsqueda: 0 = avance y giro en el sitio; 1 = arcos suaves que cambian de
// lado cada TiempoArcoBusqueda (rueda interior a PorcentajeArcoBusqueda).
constexpr int PatronBusqueda = 1;
constexpr int PorcentajeArcoBusqueda = 90;
constexpr unsigned long TiempoArcoBusqueda = 900;
// La búsqueda arranca acelerando poco a poco durante este tiempo.
constexpr unsigned long TiempoArranqueBusqueda = 400;
// Sin rutina de inicio, todo avance se limita durante este tiempo al empezar.
constexpr unsigned long TiempoArranqueSuave = 1000;
// Rampa: cuánto puede subir la orden de cada motor por ms (0 = sin rampa).
// Bajar la velocidad, frenar y escapar del borde son siempre inmediatos.
constexpr int RampaPwmPorMs = 1;

// 8. Rutinas de inicio (elegidas con los interruptores DIP)
//   DIP1 DIP2  rutina
//   off  off   1: espalda con espalda -> media vuelta en el sitio
//   ON   off   2: lado a lado -> pivota hacia el lado donde un sensor lateral
//              ve al rival (DIP3 si no lo ve ninguno)
//   off  ON    3: enfrentados en los bordes -> avanza hacia el centro
//   ON   ON    ninguna: estrategia normal desde el principio
//   DIP3: lado del giro (off = derecha, ON = izquierda)
// Los giros terminan al ver al rival de frente (o al agotar su tiempo) y el
// avance al verlo con cualquier sensor. Ver la línea cancela la rutina.
// true = un interruptor en ON lee LOW (contra GND, con pull-up).
constexpr bool DipActivoBajo = true;
// Rondas 1 y 2: giro inicial.
constexpr int VelocidadGiroInicio = 150;
constexpr unsigned long TiempoMaxGiroEspalda = 500;
constexpr unsigned long TiempoMaxGiroLado = 500;
// Rondas 1 y 2: antes de girar hacia el rival, avanza recto este tiempo a
// VelocidadEsquiva para apartarse de su embestida y atacarlo de lado
// (0 = gira enseguida). En la simulación, contra rivales que embisten, la
// esquiva del round 2 sube las victorias del 22 % al 95 %.
constexpr unsigned long TiempoEsquivaRound1 = 0;
constexpr unsigned long TiempoEsquivaRound2 = 150;
constexpr int VelocidadEsquiva = 160;
// Ronda 2: true = pivota sobre una rueda; false = gira en el sitio.
constexpr bool PivoteRound2 = false;
constexpr int VelocidadPivoteInicio = 180;
// Ronda 3: avance hacia el centro y, después, espera quieto al rival.
constexpr int VelocidadAvanceInicio = 130;
constexpr unsigned long TiempoAvanceInicio = 4000;
constexpr bool EsperarRound3 = true;
constexpr unsigned long TiempoEsperaRound3 = 3000;
// Rondas 1 y 2: al encontrar al rival embiste durante TiempoEmbestidaInicio,
// subiendo poco a poco de VelocidadEmbestidaInicio a VelocidadEmbestidaMaxima
// mientras lo tenga delante; después mantiene VelocidadAtaqueRound12.
constexpr int VelocidadEmbestidaInicio = 160;
constexpr int VelocidadEmbestidaMaxima = 160;
constexpr int VelocidadAtaqueRound12 = 160;
constexpr unsigned long TiempoEmbestidaInicio = 800;

// --- Protección contra detecciones falsas ---
// Una detección solo termina una acción si dura ConfirmacionDeteccion ms y
// nunca antes del tiempo mínimo de esa acción.
constexpr unsigned long ConfirmacionDeteccion = 5;
constexpr unsigned long TiempoMinimoGiroEspalda = 80;
constexpr unsigned long TiempoMinimoGiroLado = 60;
constexpr unsigned long TiempoMinimoAvanceInicio = 150;
constexpr unsigned long TiempoMinimoGiroLateral = 60;

// 9. Módulo de arranque
constexpr bool ModuloArranqueActivoAlto = true;
// RUN debe mantenerse estos ms seguidos para arrancar.
constexpr unsigned long FiltroModuloArranqueMs = 5;
// STOP debe mantenerse estos ms seguidos para parar: más largo para que el
// infrarrojo del rival no detenga (y reinicie) el combate.
constexpr unsigned long FiltroParadaModuloMs = 150;

// Una parada más corta que esto no es del árbitro (para reiniciar hay que volver a
// colocar los robots) sino una interferencia: al volver RUN, el robot sigue el
// mismo combate sin recalibrar el piso (recalibrar sobre la línea blanca le
// haría ver borde en todas partes).
constexpr unsigned long TiempoReanudarCombate = 2000;

// Rutina (1 espalda, 2 lado, 3 frente, 0 ninguna) según DIP1 (bit 0) y DIP2 (bit 1).
constexpr int rutinaSegunInterruptores(int dip) {
    return (dip & 3) == 0 ? 1 : (dip & 3) == 1 ? 2 : (dip & 3) == 2 ? 3 : 0;
}

// Si la curva fuera más rápida que el ataque, al corregir hacia un rival visto
// a 45° la rueda interior iría más rápida y el robot giraría al revés.
static_assert(VelocidadCurva <= VelocidadAtaque, "VelocidadCurva debe ser <= VelocidadAtaque");

#endif
