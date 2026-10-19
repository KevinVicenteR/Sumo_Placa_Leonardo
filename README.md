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

## Carpetas

Cada carpeta tiene su `.h` en `firmware/include/` y su `.cpp` en `firmware/src/`.

| Carpeta | Qué hay |
|---------|---------|
| `configuracion/` | Todos los parámetros ajustables y los pines |
| `combate/` | Módulo de arranque: START, STOP/PROG, reinicio y LED |
| `modos_rumania/` | Los 4 modos (DIP) y quién los elige (`RutinaInicio`) |
| `maniobras/` | Ataque, búsqueda, giro lateral, evasión del borde, paro |
| `borde/` | Estimador de posición y limitador de velocidad junto al borde |
| `movimiento/` | Control del movimiento, mando de motores y PID |
| `estrategia/` | Qué hacer según lo que ven los sensores (reglas) |
| `redes/` | Redes neuronales (lógica y control) y sus pesos |
| `sensores/` | Sensores de piso y de rival |
| `hardware/` | Motores, placa, DIP y caja negra (EEPROM) |
| `robot/` | Ciclo percibir → decidir → mover |

## Dónde se ajusta

- `configuracion/Pines.h`: conexiones.
- `configuracion/Parametros.h`: sensores, borde, DIP y módulo de arranque.
- `configuracion/ParametrosRedes.h`: redes neuronales y tácticas.
- `configuracion/ParametrosMotores.h`, `ParametrosAtaque.h`, `ParametrosBusqueda.h`,
  `ParametrosEvasion.h`, `ParametrosFrenado.h`, `ParametrosGiroLateral.h`.
- `modos_rumania/Modo1Defensivo.h` … `Modo4Agresivo.h`: velocidades y tiempos
  de cada modo.
