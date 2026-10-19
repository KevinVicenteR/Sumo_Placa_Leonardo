# Minisumo Rumania — firmware de combate

Solo el firmware que se carga al robot (Arduino Leonardo / JSumo XMotion) para
Rumania (RoboChallenge). El simulador, los tests, la web, el diagnóstico y el
entrenamiento de las redes están en la rama `refactor_arquitectura`, de la que
sale esta (mismo comportamiento en combate).

## Cargar al robot

```bash
pio run -t upload
```

## Modos (interruptores DIP)

| DIP1 | DIP2 | Modo | Colocación |
|------|------|------|------------|
| off  | off  | 1 defensivo   | de frente: lo espera quieto y lo recibe |
| ON   | off  | 2 contragolpe | de frente: ataca en cuanto lo ve |
| off  | ON   | 3 de lado     | de lado: pivota hacia él (DIP3 off = rival a la derecha, ON = izquierda) |
| ON   | ON   | 4 agresivo    | de espaldas: avanza y gira hacia él |

## Control remoto

- **START**: combate (calibra el piso, lee los DIP y hace el modo).
- **STOP o PROG**: para los motores y reinicia la placa entera. Los LED RX y TX
  parpadean 3 veces y quedan encendidos = listo para el siguiente START.

## Dónde se ajusta

- `firmware/include/Parametros.h`: sensores, borde, DIP y módulo de arranque.
- `firmware/include/ParametrosModo4.h`: redes neuronales y tácticas.
- `firmware/include/movimiento/parametros/`: motores, ataque, búsqueda, evasión,
  frenado y giro lateral.
- `firmware/include/movimiento/rumania/Modo1Defensivo.h` … `Modo4Agresivo.h`:
  velocidades y tiempos de cada modo.
- `firmware/include/Pines.h`: conexiones.
