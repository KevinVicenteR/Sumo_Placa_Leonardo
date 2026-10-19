#ifndef PARAMETROS_EVASION_H
#define PARAMETROS_EVASION_H

constexpr int VelocidadRetroceso = 255;
constexpr int VelocidadGiroEvasion = 75;

constexpr bool PivoteEnEvasion = false;

constexpr unsigned long TiempoRetroceso = 180;
constexpr unsigned long TiempoRetrocesoAmbos = 200;
constexpr unsigned long TiempoRetrocesoUnSensor = 90;
constexpr int VelocidadRetrocesoUnSensor = 118;
constexpr unsigned long TiempoSeparacionBorde = 90;
constexpr unsigned long TiempoSeparacionUnSensor = 50;
constexpr unsigned long TiempoMaximoRecuperacionBorde = 700;
constexpr bool CortarEvasionAlVerRival = true;
constexpr unsigned long TiempoConfirmarRivalEnEvasion = 20;
constexpr unsigned long EsperaEntreCortesEvasion = 2000;
constexpr unsigned long TiempoMaxParadoEnLinea = 300;
constexpr unsigned long TiempoReintentoRetroceso = 150;
constexpr unsigned long TiempoFrenado = 80;
constexpr unsigned long TiempoAsentamientoEvasion = 100;
constexpr unsigned long TiempoGiroEvasion = 240;
constexpr unsigned long TiempoGiroEvasionAmbos = 300;
constexpr bool SalirGirandoSiNoAvanza = false;
constexpr int AvanceMinimoParaRetroceder = 50;
constexpr unsigned long RetrocesoExtraAVelocidadMaxima = 300;
constexpr unsigned long TiempoSalidaSuaveUnSensor = 500;
constexpr int VelocidadSalidaUnSensor = 90;

#endif
