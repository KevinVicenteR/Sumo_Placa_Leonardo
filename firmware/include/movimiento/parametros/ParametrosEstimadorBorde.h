// ParametrosEstimadorBorde.h — Estimador de borde: dónde está el robot en el dohyo y cuánto puede acelerar
// Lo incluye Parametros.h: los scripts de simulacion/ cambian estos valores
// por su nombre (--param). Velocidades en PWM (0-255), tiempos en ms y
// distancias en metros, salvo que se indique otra cosa.
#ifndef PARAMETROS_ESTIMADOR_BORDE_H
#define PARAMETROS_ESTIMADOR_BORDE_H

// Calcula dónde está el robot dentro del dohyo y limita la velocidad hacia
// delante a la que aún le deja frenar antes del borde, aunque los sensores de
// piso tarden en ver la línea.
constexpr bool UsarEstimadorBorde = true;
// true = también limita la embestida si prevé el borde cerca.
constexpr bool EstimadorLimitaEmbestida = true;
// Geometría: dohyo reglamentario de 77 cm de diámetro (línea incluida) y línea de 2,5 cm.
constexpr float RadioDohyo = 0.385f;
constexpr float AnchoLineaBorde = 0.025f;
constexpr float DistanciaSensorPiso = 0.078f;     // por delante del eje de ruedas (ruedas atrás)
constexpr float SeparacionSensoresPiso = 0.04f;   // a cada lado del centro
constexpr float TrochaRuedas = 0.085f;            // distancia entre ruedas
// Modelo de las ruedas: VelocidadRuedaMaxima (m/s) a PWM 255, parada por
// debajo de ZonaMuertaPwm y constante de tiempo TauRuedaEstimador (s).
constexpr float VelocidadRuedaMaxima = 0.53f; // X3: motores de 400 rpm
constexpr int ZonaMuertaPwm = 40;
constexpr float TauRuedaEstimador = 0.06f;
// Al arrancar puede estar hasta a esta distancia del centro, con rumbo desconocido.
constexpr float RadioSalidaEstimado = 0.15f;
// Con rutina de inicio se sabe dónde empieza: rounds 1 y 2 junto al centro (a lo
// sumo a IncertSalidaCentro), round 3 a RadioSalidaRound3 del centro mirándolo
// (con IncertSalidaRound3 de error y IncertRumboRound3 rad de rumbo).
constexpr float IncertSalidaCentro = 0.06f;
constexpr float RadioSalidaRound3 = 0.33f;
constexpr float IncertSalidaRound3 = 0.06f;
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

#endif
