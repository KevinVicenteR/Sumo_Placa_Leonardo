#ifndef PARAMETROS_H
#define PARAMETROS_H

constexpr bool FrenoActivo = false;

constexpr int VelocidadMaxima = 255;
constexpr int VelocidadFreno = 140;
constexpr int VelocidadAtaque = 100;
constexpr int VelocidadEmpuje = 255;
constexpr int PorcentajeAjuste = 85;
constexpr bool ResistirEnBorde = false;
constexpr int VelocidadCurva = 85;
constexpr bool Corregir45EnPivote = true;
constexpr int VelocidadRetroceso = 195;
constexpr int VelocidadGiroEvasion = 75;
constexpr int VelocidadAvance = 32;
constexpr int VelocidadGiroBusqueda = 75;
constexpr int VelocidadPivoteLateral = 100;
constexpr bool GiroLateralEnRueda = true;
constexpr int VelocidadRuedaPivote = 100;

constexpr int PolaridadSensoresEnemigo = 0;
constexpr int MascaraEnemigosActivosHigh = 31;

constexpr int PorcentajeUmbralLinea = 15;
constexpr int UmbralLineaMinimo = 60;
constexpr int MuestrasCalibracionPiso = 64;
constexpr int NegroPisoIzquierdo = 994;
constexpr int NegroPisoDerecho = 972;
constexpr int PorcentajeCercaBorde = 70;
constexpr int VelocidadCercaBorde = 45;

constexpr unsigned long TiempoInicioReglamentario = 5000;
constexpr unsigned long TiempoFrenadoRuedas = 80;
constexpr unsigned long TauRuedas = 80;
constexpr int VelocidadMinimaFreno = 20;
constexpr unsigned long TiempoRetroceso = 180;
constexpr unsigned long TiempoParoPerdida = 250;
constexpr unsigned long TiempoRetrocesoAmbos = 200;
constexpr unsigned long TiempoSeparacionBorde = 90;
constexpr unsigned long TiempoSeparacionUnSensor = 72;
constexpr unsigned long TiempoRetrocesoUnSensor = 90;
constexpr int VelocidadRetrocesoUnSensor = 118;
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
constexpr unsigned long TiempoMinimoGiroEvasion = 60;
constexpr unsigned long TiempoAvanceBusqueda = 1500;
constexpr unsigned long TiempoGiroBusqueda = 150;
constexpr unsigned long TiempoPausaBusqueda = 100;
constexpr unsigned long TiempoPulsoAvance = 0;
constexpr unsigned long TiempoPausaPulso = 40;
constexpr int PorcentajeArcoZigzag = 100;
constexpr unsigned long TiempoArcoZigzag = 0;
constexpr unsigned long TiempoEmbestida = 2500;
constexpr unsigned long TiempoRetencionEnemigo = 40;
constexpr unsigned long TiempoMaxGiroLateral = 400;

constexpr bool UsarEstimadorBorde = true;
constexpr bool EstimadorLimitaEmbestida = true;
constexpr float RadioDohyo = 0.385f;
constexpr float AnchoLineaBorde = 0.025f;
constexpr float DistanciaSensorPiso = 0.045f;
constexpr float SeparacionSensoresPiso = 0.04f;
constexpr float TrochaRuedas = 0.085f;
constexpr float VelocidadRuedaMaxima = 1.0f;
constexpr int ZonaMuertaPwm = 40;
constexpr float TauRuedaEstimador = 0.06f;
constexpr float RadioSalidaEstimado = 0.15f;
constexpr float ErrorDistancia = 0.3f;
constexpr float ErrorGiro = 0.25f;
constexpr float DerivaRumbo = 0.3f;
constexpr float EmpujonAtaque = 0.15f;
constexpr float EmpujonContacto = 0.4f;
constexpr float EmpujonLateral = 0.2f;
constexpr float AnguloLineaLado = 0.6f;
constexpr float IncertAnguloLinea = 0.5f;
constexpr float IncertPosicionLinea = 0.02f;
constexpr float SigmasSeguridad = 1.5f;
constexpr float MargenBorde = 0.06f;
constexpr float DesaceleracionFreno = 4.0f;
constexpr float RetardoReaccion = 0.04f;
constexpr int VelocidadMinimaEstimador = 70;
constexpr float IncertPosicionConfiable = 0.08f;
constexpr float IncertRumboConfiable = 0.8f;

constexpr bool UsarPoliticaAprendida = false;

constexpr int PatronBusqueda = 1;
constexpr int PorcentajeArcoBusqueda = 75;
constexpr unsigned long TiempoArcoBusqueda = 900;
constexpr unsigned long TiempoArranqueBusqueda = 400;
constexpr unsigned long TiempoArranqueSuave = 1000;
constexpr bool EscapeEnArco = true;
constexpr int PorcentajeArcoEscape = 60;
constexpr unsigned long TiempoArcoEscape = 220;
constexpr unsigned long TiempoArcoEscapeAmbos = 250;
constexpr unsigned long TiempoMinimoArcoEscape = 150;
constexpr int RampaPwmPorMs = 1;

constexpr bool DipActivoBajo = true;
constexpr int VelocidadGiroInicio = 180;
constexpr unsigned long TiempoMaxGiroEspalda = 500;
constexpr unsigned long TiempoMaxGiroLado = 500;
constexpr bool PivoteRound2 = true;
constexpr int VelocidadPivoteInicio = 255;
constexpr int VelocidadAvanceInicio = 33;
constexpr unsigned long TiempoAvanceInicio = 4000;
constexpr bool EsperarRound3 = true;
constexpr unsigned long TiempoEsperaRound3 = 3000;
constexpr int VelocidadEmbestidaInicio = 255;
constexpr int VelocidadAtaqueRound12 = 255;
constexpr unsigned long TiempoEmbestidaInicio = 1500;

constexpr unsigned long ConfirmacionDeteccion = 5;
constexpr unsigned long ConfirmacionSensorEnemigo = 0;
constexpr unsigned long TiempoMinimoGiroEspalda = 80;
constexpr unsigned long TiempoMinimoGiroLado = 60;
constexpr unsigned long TiempoMinimoAvanceInicio = 150;
constexpr unsigned long TiempoMinimoGiroLateral = 60;

constexpr bool ModuloArranqueActivoAlto = true;
constexpr unsigned long FiltroModuloArranqueMs = 5;
constexpr unsigned long FiltroParadaModuloMs = 150;

constexpr int rutinaSegunInterruptores(int dip) {
    return (dip & 3) == 0 ? 1 : (dip & 3) == 1 ? 2 : (dip & 3) == 2 ? 3 : 0;
}

static_assert(VelocidadCurva <= VelocidadAtaque, "VelocidadCurva debe ser <= VelocidadAtaque");

#endif
