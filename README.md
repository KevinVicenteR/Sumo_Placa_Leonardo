# Sumo Placa Leonardo

Firmware de un robot de minisumo con placa JSumo XMotion (ATmega32U4, compatible con Arduino Leonardo), sensores de enemigo digitales JSumo y sensores de piso analógicos JSumo.

## Puesta en marcha rápida

1. Instala [PlatformIO](https://platformio.org/) (extensión de VS Code o `pip install platformio`).
2. Conecta la placa por USB.
3. Sube el firmware de combate:

   ```
   pio run -e leonardo -t upload
   ```

4. Elige la rutina del round con los interruptores DIP (ver más abajo) y enciende el robot sobre el dohyo.

## Modos de firmware

| Entorno | Para qué sirve | Comando |
|---|---|---|
| `leonardo` | Combate. Es el que se sube por defecto | `pio run -e leonardo -t upload` |
| `monitoreo` | Combate enviando por USB lo que lee y hace el robot | `pio run -e monitoreo -t upload` |
| `diagnostico` | Motores apagados; muestra por USB sensores de piso, sensores de enemigo e interruptores DIP. Pulsa `c` en el monitor para recalibrar el piso | `pio run -e diagnostico -t upload` |
| `prueba_motores` | Con el robot levantado, repite una secuencia de movimientos y la anuncia por USB para comprobar cada rueda | `pio run -e prueba_motores -t upload` |

Para ver lo que envía la placa: `pio device monitor` (115200 baudios).

## Antes de cada competición

1. **Motores.** Sube `prueba_motores`, levanta el robot y comprueba que cada rueda gira hacia donde anuncia el monitor.
2. **Sensores.** Sube `diagnostico` y mira el monitor:
   - Sensores de piso: anota el valor sobre negro, sobre la línea blanca y con el sensor fuera del borde. El blanco debe separarse claramente del negro.
   - Sensores de enemigo (`VE=`): con nada alrededor deben estar todos en 0, y en 1 al poner un objeto delante de cada uno.
   - Interruptores (`DIP=`): deben coincidir con cómo los pusiste (orden DIP1, DIP2, DIP3).
3. **Combate.** Vuelve a subir `leonardo`.

## Interruptores DIP: rutina de inicio

| DIP1 | DIP2 | Rutina |
|---|---|---|
| off | off | **Round 1, espalda con espalda** (predeterminada) |
| ON | off | **Round 2, lado a lado** |
| off | ON | **Round 3, enfrentados lejos** |
| ON | ON | Ninguna: estrategia normal desde el principio |

**DIP3** elige el lado: off = derecha, ON = izquierda.

Si al comprobarlos en el modo diagnóstico ves los valores invertidos, cambia `DipActivoBajo` en `include/Parametros.H`.

### Qué hace cada rutina

- **Round 1 (espalda con espalda).** Gira en el sitio media vuelta hacia el lado del DIP3. Al ver al rival de frente frena el giro para no pasarse y embiste.
- **Round 2 (lado a lado).** Pivota sobre una rueda hacia el lado donde un sensor lateral ve al rival (si ninguno lo ve, hacia el lado del DIP3). Al tenerlo de frente frena el giro y embiste.
- **Round 3 (enfrentados lejos).** Avanza recto y muy lento hacia el centro. Se detiene a esperar al rival mirando al frente y, si no llega, empieza a buscar. Durante el avance ignora los sensores laterales, porque apuntan fuera del dohyo.

En todas las rutinas, ver la línea del borde las cancela y el robot escapa del borde. Una detección suelta (un reflejo, una mano) no corta una acción a medias: tiene que confirmarse y respetar un tiempo mínimo.

## Comportamiento en combate

| Situación | Qué hace |
|---|---|
| No ve al rival | Busca despacio, avanzando en arcos suaves |
| Rival en el sensor frontal | Ataca: empieza a `VelocidadAtaque` y sube hasta `VelocidadEmpuje` mientras lo mantiene de frente |
| Rival en el frontal y en un sensor de 45° | Ataca corrigiendo un poco hacia ese lado |
| Rival solo en un sensor de 45° o en un lateral | Pivota sobre la rueda de ese lado hasta tenerlo de frente |
| Pierde al rival mientras ataca | Frena en seco |
| Ve la línea blanca | Retrocede, se para y gira hacia dentro. El borde tiene prioridad sobre todo |
| Cerca del borde | Limita la velocidad, incluso atacando |

El robot también estima su posición dentro del dohyo para limitar la velocidad cuando prevé el borde cerca (`UsarEstimadorBorde`).

## Módulo de arranque (START/STOP del árbitro)

Está preparado pero sin pin. Mientras `PIN_MODULO_ARRANQUE` valga `-1` en `include/Pines.H`, el robot arranca al encenderse.

Para usarlo:

1. Pon en `PIN_MODULO_ARRANQUE` el pin donde conectes la señal. Pines libres: 3, A0, A3. **No uses el 4**: es el sensor frontal central.
2. En `include/Parametros.H`, `ModuloArranqueActivoAlto` debe ser `true` si el módulo da HIGH en RUN, o `false` si da LOW.

Con el módulo activo:

- **Al encender y con STOP:** motores parados.
- **Con START:** calibra el piso, lee los DIP y empieza el combate desde cero.

## Pines

| Señal | Pin |
|---|---|
| Piso izquierdo / derecho | A1 / A2 |
| Enemigo lateral izquierdo / 45° izquierdo / frontal / 45° derecho / lateral derecho | 1 / 2 / 4 / A5 / A4 |
| Motor izquierdo: PWM, IN1, IN2 | 10, 9, 13 |
| Motor derecho: PWM, IN1, IN2 | 11, 8, 12 |
| Interruptores DIP 1 / 2 / 3 | 5 / 6 / 7 |
| Módulo de arranque | sin asignar (`PIN_MODULO_ARRANQUE`) |

Todos están en `include/Pines.H`.

## Parámetros principales

Todos están en `include/Parametros.H`. Las velocidades son PWM de 0 a 255 y los tiempos, milisegundos.

### Velocidades

| Parámetro | Valor | Qué controla |
|---|---|---|
| `VelocidadAvance` | 32 | Avance de búsqueda |
| `VelocidadGiroBusqueda` | 75 | Giro de búsqueda (patrón clásico) |
| `VelocidadAtaque` | 100 | Velocidad con la que empieza a atacar |
| `VelocidadEmpuje` | 255 | Velocidad máxima de ataque |
| `TiempoEmbestida` | 2500 | Tiempo para subir de `VelocidadAtaque` a `VelocidadEmpuje` con el rival de frente |
| `VelocidadRuedaPivote` | 100 | Rueda que empuja al pivotar hacia un rival visto de lado |
| `VelocidadRetroceso` / `VelocidadRetrocesoUnSensor` | 195 / 118 | Retroceso al ver la línea con los dos sensores / con uno |
| `VelocidadGiroEvasion` | 75 | Giro tras retroceder del borde |
| `VelocidadCercaBorde` | 45 | Límite de velocidad cerca del borde |

### Rutinas de inicio

| Parámetro | Valor | Qué controla |
|---|---|---|
| `VelocidadGiroInicio` | 180 | Giro de media vuelta del round 1 |
| `VelocidadPivoteInicio` | 255 | Rueda que empuja en el pivote del round 2 |
| `VelocidadEmbestidaInicio` / `TiempoEmbestidaInicio` | 255 / 1500 | Embestida al encontrar al rival en los rounds 1 y 2 |
| `VelocidadAtaqueRound12` | 255 | Velocidad tras esa embestida mientras siga el mismo ataque |
| `TiempoMaxGiroEspalda` / `TiempoMaxGiroLado` | 500 / 500 | Tope del giro si no encuentra al rival |
| `VelocidadAvanceInicio` / `TiempoAvanceInicio` | 33 / 4000 | Avance del round 3 |
| `EsperarRound3` / `TiempoEsperaRound3` | true / 3000 | Espera quieto tras el avance del round 3 |

### Sensores

| Parámetro | Valor | Qué controla |
|---|---|---|
| `PorcentajeUmbralLinea` | 15 | Cuánto debe separarse la lectura del negro para considerarla línea |
| `PorcentajeCercaBorde` | 70 | Parte de ese umbral a partir de la cual el robot «se siente cerca del borde» |
| `MascaraEnemigosActivosHigh` | 31 | Sensores de enemigo que dan HIGH al detectar (bit 0 = lateral izquierdo … bit 4 = lateral derecho; 31 = todos) |
| `ConfirmacionDeteccion` | 5 | Milisegundos seguidos que debe durar una detección para terminar una acción |
| `ConfirmacionSensorEnemigo` | 0 | Lo mismo en cada sensor; súbelo a 3–5 si el robot reacciona a reflejos muy cortos |
| `TiempoMinimoGiroEspalda` | 80 | Giro mínimo del round 1 antes de aceptar una detección. Debe ser unos 2/3 de lo que tarda en dar media vuelta |

## Simulador

La carpeta `simulacion/` compila el firmware real contra un robot y un dohyo simulados, para probar cambios antes de llevarlos a la pista. Necesita Python 3 y un compilador de C++.

```
python3 simulacion/simular.py                          # resumen de 4 escenarios (sin rival, quieto, errante, que embiste)
python3 simulacion/simular.py --param VelocidadAtaque=120   # probar un cambio de Parametros.H
python3 simulacion/evaluar.py                          # evaluación completa en 11 entornos, con caídas por causa
python3 simulacion/barrido.py VelocidadAtaque=100,150 --todos   # comparar combinaciones de parámetros
```

Opciones útiles del simulador (se pasan tal cual a `simular.py` y en `--extra` a `evaluar.py`):

| Opción | Qué simula |
|---|---|
| `--salida espalda\|lado\|frente --dip N` | Posición de salida de cada round y valor de los DIP (bit 0 = DIP1) |
| `--rival-lado der\|izq` | Lado del rival en el round 2 |
| `--retardo-piso 40` | Sensores de piso lentos (ms) |
| `--pared 0.45` | Gente u objetos alrededor del dohyo |
| `--fantasmas 2` | Detecciones falsas por segundo y sensor |
| `--bateria 1.4 --friccion-caja 0.03` | Robot más rápido a PWM bajo |
| `--driver l298n` | Driver que deja el motor libre con PWM 0 |

**Ver un combate concreto:**

```
python3 simulacion/simular.py --n 1 --modos agresivo
B=$(ls -t simulacion/build/sim_* | head -1)
$B --modo agresivo --n 1 --semilla 3 --rpm 750 --diam 0.03 --masa 0.3 --tray tray.csv
python3 simulacion/ver_traza.py tray.csv 2
```

**Página interactiva:** `simulacion/web/generar_pagina.sh` genera `simulacion/web/dohyo.html`. Necesita LLVM, lld y binaryen (`brew install llvm lld binaryen`).

El simulador es una aproximación: los motores y sensores del robot real no se comportan exactamente igual. Úsalo para comparar opciones y confirma siempre en la pista.

## Tests

```
pio test -e native
```

## Estructura

| Carpeta / archivo | Contenido |
|---|---|
| `include/Pines.H` | Pines |
| `include/Parametros.H` | Todos los parámetros ajustables |
| `src/main.cpp` | Arranque y modos de firmware |
| `src/Percepcion.cpp` | Lectura y filtrado de sensores, calibración del piso |
| `src/EstrategiaCombate.cpp` | Qué hacer según lo que ven los sensores |
| `src/ControlMovimiento.cpp` | Cómo moverse: búsqueda, ataque, escape del borde, rutinas de inicio |
| `src/EstimadorBorde.cpp` | Estimación de la posición en el dohyo |
| `src/Motor.cpp` | Control de los motores |
| `test/` | Tests unitarios |
| `simulacion/` | Simulador y herramientas de análisis |
