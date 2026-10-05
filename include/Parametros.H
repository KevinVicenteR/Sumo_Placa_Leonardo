#ifndef PARAMETROS_H
#define PARAMETROS_H

// Ajuste conservador para verificar la evasión en el robot real de 750 rpm.
// Velocidades y tiempos pendientes de validar sobre el dohyo; la simulación
// no sustituye medir la respuesta del driver y el sentido de cada rueda.

// Parada con PWM=0; sin la orden de freno a PWM máximo.
constexpr bool FrenoActivo = false;

// Velocidades PWM (0-255)
constexpr int VelocidadMaxima = 255;
// Limitar el contramando de frenado para evitar golpes a PWM 255.
constexpr int VelocidadFreno = 140;
// Acercamiento al enemigo. Con sensores de piso rápidos 200 gana algo más, pero si
// tardan ~40 ms en ver la línea y el enemigo se aparta (o era un objeto fuera del
// dohyo) el robot llega tarde al borde. 180 queda casi igual de fuerte y cae menos;
// 155 o menos apenas empuja con motores duros.
constexpr int VelocidadAtaque = 100;
// Techo del ataque: sube desde VelocidadAtaque hasta aquí durante TiempoEmbestida
constexpr int VelocidadEmpuje = 255;
// Enemigo de frente y también en un sensor de 45°: la rueda de ese lado va a
// este porcentaje de la otra para centrarlo sin dejar de empujar
constexpr int PorcentajeAjuste = 85;
// Enemigo empujando de frente al llegar a la línea: true = sigue empujando a
// fondo en vez de frenar y retroceder (retroceder solo le ayuda a sacarlo).
// En la simulación sale peor: si el enemigo se aparta en el último momento, el
// robot sigue de largo. Desactivado.
constexpr bool ResistirEnBorde = false;
constexpr int VelocidadCurva = 85;
// Enemigo visto solo por un sensor de 45°: true = la rueda de ese lado queda
// parada y hace de pivote mientras la otra empuja a la velocidad de ataque;
// false = curva con las dos ruedas (la interior a VelocidadCurva)
constexpr bool Corregir45EnPivote = true;
constexpr int VelocidadRetroceso = 210;
constexpr int VelocidadGiroEvasion = 75;
constexpr int VelocidadAvance = 32;
constexpr int VelocidadGiroBusqueda = 75;
constexpr int VelocidadPivoteLateral = 100;
// Enemigo visto por un sensor lateral: true = la rueda de ese lado se frena y hace
// de pivote mientras la otra gira a VelocidadRuedaPivote; false = giro en el sitio
// (ruedas en sentidos opuestos a VelocidadPivoteLateral)
constexpr bool GiroLateralEnRueda = true;
constexpr int VelocidadRuedaPivote = 100;

// Diagnóstico: 0 aprende cada sensor sin objetos; 1=HIGH, -1=LOW.
// Combate: usar la máscara guardada, sin recalibrar delante del rival.
constexpr int PolaridadSensoresEnemigo = 0;
// Orden: lateral izq, frontal izq, central, frontal der, lateral der.
// Las lecturas de reposo registradas eran 0: los cinco detectan en HIGH.
constexpr int MascaraEnemigosActivosHigh = 31;

// Piso: medir negro al encender, sin espera y con motores deshabilitados.
// Detectar blanco por diferencia absoluta, con margen e histéresis.
constexpr int PorcentajeUmbralLinea = 15;
constexpr int UmbralLineaMinimo = 60;
// La referencia de negro se mantiene fija durante todo el combate.
constexpr int MuestrasCalibracionPiso = 64;
// Referencias iniciales; la medición al encender las sustituye antes de moverse.
constexpr int NegroPisoIzquierdo = 994;
constexpr int NegroPisoDerecho = 972;
// Aviso de cerca del borde: la lectura de piso se aleja del negro este porcentaje
// del margen de línea. Mientras dura, el robot avanza (también al atacar) a lo sumo a
// VelocidadCercaBorde (100 % equivale a no usarlo). En la simulación apenas cambia
// nada: entre el primer gris y la línea hay muy poca distancia.
constexpr int PorcentajeCercaBorde = 70;
constexpr int VelocidadCercaBorde = 45;

// Tiempos (ms)
constexpr unsigned long TiempoInicioReglamentario = 5000;
// Contramando al perder al enemigo: cada rueda frena según su
// velocidad estimada (orden filtrada con la inercia TauRuedas), hasta este tiempo a
// toda velocidad; por debajo de VelocidadMinimaFreno no hace falta frenar
// Referencia de simulación antigua; la nueva evasión no usa este contramando.
constexpr unsigned long TiempoFrenadoRuedas = 80;
constexpr unsigned long TauRuedas = 80;
constexpr int VelocidadMinimaFreno = 20;
constexpr unsigned long TiempoRetroceso = 180;
// Paro en seco cuando el enemigo desaparece en pleno ataque (ms)
constexpr unsigned long TiempoParoPerdida = 150;
constexpr unsigned long TiempoRetrocesoAmbos = 220;
// Negro continuo antes del giro; límite de recuperación sin sensores traseros.
constexpr unsigned long TiempoSeparacionBorde = 100;
constexpr unsigned long TiempoSeparacionUnSensor = 80;
constexpr unsigned long TiempoRetrocesoUnSensor = 100;
constexpr int VelocidadRetrocesoUnSensor = 130;
constexpr unsigned long TiempoRectoBordeUnSensor = 80;
constexpr int PorcentajeRetrocesoUnSensor = 60;
constexpr unsigned long TiempoMaximoRecuperacionBorde = 700;
constexpr unsigned long TiempoFrenado = 80;
constexpr unsigned long TiempoAsentamientoEvasion = 100;
constexpr unsigned long TiempoGiroEvasion = 240;
constexpr unsigned long TiempoSalidaSuaveUnSensor = 500;
constexpr int VelocidadSalidaUnSensor = 60;
constexpr unsigned long TiempoGiroEvasionAmbos = 220;
constexpr unsigned long TiempoGiroEvasionEnSitio = 100;
// Referencia histórica del simulador; el control completa TiempoGiroEvasion.
constexpr unsigned long TiempoMinimoGiroEvasion = 60;
constexpr unsigned long TiempoAvanceBusqueda = 1500;
constexpr unsigned long TiempoGiroBusqueda = 150;
// Motores parados entre el avance y el giro de búsqueda, y entre el giro y el
// avance, para que el robot se asiente (0 = sin pausa)
constexpr unsigned long TiempoPausaBusqueda = 100;
// Avance de búsqueda en pulsos: TiempoPulsoAvance empujando y TiempoPausaPulso
// frenando, para que no coja velocidad y, aunque el sensor de piso tarde en ver la
// línea, pare a tiempo (0 = avance continuo: a 255 se sale casi siempre con un
// sensor lento). Avances largos y giros cortos recorren todo el dohyo.
constexpr unsigned long TiempoPulsoAvance = 0;
constexpr unsigned long TiempoPausaPulso = 40;
// Búsqueda en zigzag: durante el avance, arcos alternos de TiempoArcoZigzag con la
// rueda interior a PorcentajeArcoZigzag de la exterior (100 = avance recto).
// En la simulación, con pulsos sale peor que el avance recto.
constexpr int PorcentajeArcoZigzag = 100;
constexpr unsigned long TiempoArcoZigzag = 0;
// Duración de la subida gradual de VelocidadAtaque a VelocidadEmpuje (ms).
// Se reinicia si pierde la alineación antes de completar la subida o evade el borde.
constexpr unsigned long TiempoEmbestida = 1500;
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
constexpr int PatronBusqueda = 1;
constexpr int PorcentajeArcoBusqueda = 75;
constexpr unsigned long TiempoArcoBusqueda = 900;
constexpr unsigned long TiempoArranqueBusqueda = 400;
// Limitar todo avance al salir de la calibración, incluso si detecta un enemigo.
constexpr unsigned long TiempoArranqueSuave = 1000;
// Evasión: retroceso recto limitado, paro y giro lento solo con ambos sensores en negro.
// El selector se conserva por compatibilidad; todas las evasiones usan esta maniobra.
constexpr bool EscapeEnArco = true;
constexpr int PorcentajeArcoEscape = 60;
constexpr unsigned long TiempoArcoEscape = 220;
constexpr unsigned long TiempoArcoEscapeAmbos = 250;
// Reservado para compatibilidad con el simulador; se completa todo el escape.
constexpr unsigned long TiempoMinimoArcoEscape = 150;
// Rampa: cuánto puede subir la orden de cada motor por milisegundo (0 = sin
// rampa). Bajar la velocidad, frenar, escapar del borde y el paro son inmediatos.
constexpr int RampaPwmPorMs = 1;

// Rutinas de inicio según la posición de salida de cada round, elegidas con los
// interruptores DIP de la placa al encender:
//   DIP1 DIP2  rutina
//   off  off   round 1 (predeterminada): espalda con espalda -> media vuelta en el sitio
//   ON   off   round 2: lado a lado, mirando en sentidos opuestos -> pivota sobre la
//              rueda del lado donde un sensor lateral ve al rival (DIP3 si no lo ve ninguno)
//   off  ON    round 3: enfrentados en los bordes -> avance hacia el centro
//   ON   ON    ninguna (estrategia normal desde el principio)
//   DIP3: lado del giro (off = derecha, ON = izquierda). En el round 2, el lado
//   donde está el rival.
// Los giros terminan en cuanto el sensor frontal ve al rival (o al agotar su
// tiempo máximo) y el avance en cuanto lo ve cualquier sensor. Ver la línea
// cancela la rutina: el borde siempre tiene prioridad.
// DipActivoBajo: true si un interruptor en ON lee LOW (contra GND, con pull-up)
constexpr bool DipActivoBajo = true;
constexpr int VelocidadGiroInicio = 120;
constexpr unsigned long TiempoMaxGiroEspalda = 500;
constexpr unsigned long TiempoMaxGiroLado = 500;
// Round 2: true = pivota sobre la rueda del lado del rival; false = gira en el sitio
constexpr bool PivoteRound2 = true;
// Velocidad de la rueda que empuja en el pivote del round 2 (la otra queda parada)
constexpr int VelocidadPivoteInicio = 180;
constexpr int VelocidadAvanceInicio = 45;
constexpr unsigned long TiempoAvanceInicio = 6000;
// Al encontrar al rival con la rutina de los rounds 1 y 2 está pegado y el borde
// lejos: embiste a esta velocidad mientras lo tenga delante, hasta este tiempo.
// Con una rutina elegida no se aplica el arranque suave (TiempoArranqueSuave).
constexpr int VelocidadEmbestidaInicio = 200;
// Tras esa embestida, mientras siga el mismo ataque, los rounds 1 y 2 conservan
// su velocidad de siempre (no suben hasta VelocidadEmpuje); al perder al rival
// vuelve el ataque normal
constexpr int VelocidadAtaqueRound12 = 130;
constexpr unsigned long TiempoEmbestidaInicio = 1500;

// Protección contra detecciones falsas (reflejos, manos, parpadeos del sensor):
// una detección solo termina una acción si se mantiene ConfirmacionDeteccion ms
// seguidos, y nunca antes del tiempo mínimo de esa acción. El mínimo del giro
// debe quedar por debajo de lo que tarda en girar hasta donde puede estar el
// rival (en el round 1, unos 2/3 de lo que tarda en dar media vuelta).
constexpr unsigned long ConfirmacionDeteccion = 5;
// Lo mismo en cada sensor de enemigo: una detección nueva cuenta solo tras verse
// estos ms seguidos (antes de la retención TiempoRetencionEnemigo, que alargaría
// también las lecturas falsas)
constexpr unsigned long ConfirmacionSensorEnemigo = 0;
constexpr unsigned long TiempoMinimoGiroEspalda = 80;
constexpr unsigned long TiempoMinimoGiroLado = 60;
constexpr unsigned long TiempoMinimoAvanceInicio = 150;
constexpr unsigned long TiempoMinimoGiroLateral = 60;

// Rutina (1 espalda, 2 lado, 3 frente, 0 ninguna) según DIP1 (bit 0) y DIP2 (bit 1)
constexpr int rutinaSegunInterruptores(int dip) {
    return (dip & 3) == 0 ? 1 : (dip & 3) == 1 ? 2 : (dip & 3) == 2 ? 3 : 0;
}

// Si la curva fuera más rápida que el ataque, al corregir hacia un enemigo
// visto a 45° la rueda interior iría más rápida y el robot giraría al revés
static_assert(VelocidadCurva <= VelocidadAtaque, "VelocidadCurva debe ser <= VelocidadAtaque");

#endif
