# Ajuste de los arranques de Rumania para X3

Medición completada el 30 de octubre de 2026. Referencia y dos combinaciones candidatas confirmadas con `--muestras 30000` cada una.

Modelo: 500 g, motores de 400 rpm, par 0,8, ruedas atrás, pala delante. Se conservan el START inmediato, la evasión de borde y los pesos de las redes.

## Valores seleccionados y resultado confirmado

Cada modo toma su resultado de la batería indicada; las colocaciones de los otros modos no intervienen en él. Se prioriza la mayor tasa de victorias entre las candidatas que no aumentan ninguna categoría de caída respecto de BASE a 30.000 muestras.

| Modo | Variante | Gana BASE % | Gana final % | Cae BASE % | Cae final % | Propias | Solo |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | I (confirmacion_a) | 63,15 | 63,29 | 19,33 | 18,96 | 0 | 0/396 |
| 2 | O (confirmacion_a) | 62,14 | 63,84 | 19,77 | 14,94 | 0 | 0/792 |
| 3 | K (confirmacion_b) | 49,27 | 52,62 | 27,26 | 22,42 | 0 | 0/792 |
| 4 | AB (confirmacion_a) | 45,21 | 46,37 | 30,37 | 30,26 | 0 | 0/792 |

Cambios respecto de las constantes originales:

| Modo | Constantes finales que cambian |
| --- | --- |
| 1 | `Modo1TiempoQuieto = 1000` |
| 2 | `Modo2TiempoMinimo = 50`, `Modo2TiempoQuieto = 1500` |
| 3 | `Modo3VelocidadGiro = 200` |
| 4 | `Modo4EstabilizarFrontal = true`, `Modo4VelocidadConfirmacion = 225` |

En modo 4 se conserva la búsqueda original y se deja de girar en cuanto el frontal detecta al rival: ambos motores avanzan al PWM de confirmación. La confirmación existente sigue siendo necesaria para terminar la rutina con ataque máximo. La nueva rama respeta `Modo4TiempoMinimo`, el máximo del modo y la prioridad externa del borde. No añade una espera al START.

Se mantienen los pesos de red, el simulador y el resto del firmware. Las cabeceras recogen las cifras por RU correspondientes a sus propios combates, incluyendo los 11 rivales.

## Método

Batería original: 7 colocaciones × 11 rivales × 12 factores = 924 casillas, combates de hasta 30 s. RU1 mide exclusivamente el modo 1; RU2, RU3 y RU4 miden respectivamente los modos 2, 3 y 4, incluidos sus dos lados. Se usan todos los rivales, también «solo» y el de 500 g. Los porcentajes no excluyen esos casos.

`--muestras 10000` produce 10.164 combates (11 semillas por casilla): 1.452 del modo 1 y 2.904 de cada otro modo. `--muestras 30000` produce 30.492 (33 semillas por casilla): 4.356 del modo 1 y 8.712 de cada otro modo. Las semillas son las mismas para cada configuración: 1–11 en exploración y 1–33 en confirmación.

Criterio de selección: más victorias y no más caídas totales, sin aumentar la tasa de `caidas_propias` ni las caídas contra «solo». «Propias» cuenta salidas sin que el rival empuje; «Solo» cuenta caídas en los casos sin rival. Se comparan tasas con igual número de combates por modo. Máximo de dos baterías simultáneas.

## Exploración (10.164 combates por batería)

Las velocidades son PWM (0–255); los tiempos están en ms. Cada fila contiene todos los parámetros de su modo, incluidos los que conservan el valor original. Las configuraciones repetidas se muestran una sola vez, con el identificador de su primera prueba; los cambios en otros modos no alteran sus casillas. En el modo 4, Estab. = 1 frena el giro durante la confirmación del frontal, con el PWM indicado. Los prototipos retirados Q–T se detallan aparte.

### Modo 1 (RU1)

Parámetros, en orden: `Modo1VelocidadGiro`, `Modo1VelocidadAvanceEspera`, `Modo1TiempoMinimo`, `Modo1TiempoQuieto`, `Modo1TiempoRecibir`, `Modo1TiempoMax`.

| Variante | Giro | Avance | Mín. | Quieto | Recibir | Máx. | Gana % | Cae % | Propias | Solo |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| BASE | 200 | 50 | 0 | 3000 | 200 | 9000 | 64,05 | 18,32 | 0 | 0 |
| A | 150 | 75 | 0 | 1000 | 100 | 9000 | 62,81 | 16,12 | 0 | 0 |
| B | 120 | 100 | 0 | 500 | 50 | 9000 | 60,12 | 17,98 | 0 | 0 |
| C | 100 | 50 | 0 | 1500 | 100 | 9000 | 60,88 | 18,80 | 0 | 0 |
| D | 150 | 100 | 0 | 500 | 50 | 9000 | 60,54 | 17,63 | 0 | 0 |
| E | 180 | 75 | 0 | 2000 | 150 | 9000 | 63,64 | 17,63 | 0 | 0 |
| F | 120 | 75 | 0 | 1000 | 200 | 9000 | 63,29 | 18,60 | 0 | 0 |
| G | 150 | 50 | 0 | 3000 | 200 | 9000 | 64,33 | 18,46 | 0 | 0 |
| H | 200 | 50 | 0 | 3000 | 100 | 9000 | 63,36 | 15,98 | 0 | 0 |
| I | 200 | 50 | 0 | 1000 | 200 | 9000 | 64,46 | 17,63 | 0 | 0 |
| J | 180 | 75 | 0 | 1500 | 100 | 9000 | 63,71 | 15,22 | 0 | 0 |
| K | 200 | 50 | 0 | 3000 | 50 | 9000 | 63,77 | 15,22 | 0 | 0 |
| L | 200 | 50 | 0 | 3000 | 150 | 9000 | 63,50 | 17,63 | 0 | 0 |
| M | 150 | 50 | 0 | 3000 | 50 | 9000 | 63,91 | 15,29 | 0 | 0 |
| N | 180 | 50 | 0 | 3000 | 50 | 9000 | 64,05 | 15,15 | 0 | 0 |
| O | 180 | 50 | 0 | 3000 | 200 | 9000 | 64,39 | 18,46 | 0 | 0 |
| P | 200 | 50 | 0 | 3000 | 300 | 9000 | 63,57 | 19,15 | 0 | 0 |

Cada fila: 1.452 combates del modo; 132 casos «solo».

### Modo 2 (RU2)

Parámetros, en orden: `Modo2VelocidadGiro`, `Modo2VelocidadAvanceEspera`, `Modo2TiempoMinimo`, `Modo2TiempoQuieto`, `Modo2TiempoMax`.

| Variante | Giro | Avance | Mín. | Quieto | Máx. | Gana % | Cae % | Propias | Solo |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| BASE | 150 | 100 | 300 | 3000 | 6000 | 62,19 | 19,21 | 0 | 0 |
| A | 120 | 100 | 0 | 1000 | 6000 | 62,88 | 14,94 | 0 | 0 |
| B | 100 | 125 | 100 | 500 | 6000 | 61,36 | 18,39 | 0 | 0 |
| C | 150 | 100 | 0 | 1500 | 6000 | 63,29 | 15,01 | 0 | 0 |
| D | 120 | 100 | 300 | 500 | 6000 | 61,50 | 20,39 | 0 | 0 |
| E | 100 | 75 | 150 | 1000 | 6000 | 61,09 | 19,83 | 0 | 0 |
| F | 180 | 100 | 0 | 1000 | 6000 | 62,88 | 15,01 | 0 | 0 |
| G | 120 | 100 | 300 | 1000 | 6000 | 62,60 | 18,53 | 0 | 0 |
| H | 150 | 100 | 0 | 1000 | 6000 | 62,95 | 14,94 | 0 | 0 |
| I | 120 | 100 | 0 | 1500 | 6000 | 62,67 | 15,43 | 0 | 0 |
| J | 120 | 100 | 100 | 1000 | 6000 | 62,88 | 15,43 | 0 | 0 |
| K | 120 | 100 | 0 | 2000 | 6000 | 62,60 | 15,56 | 0 | 0 |
| L | 120 | 100 | 0 | 500 | 6000 | 61,64 | 16,67 | 0 | 0 |
| N | 150 | 100 | 100 | 1500 | 6000 | 62,81 | 15,84 | 0 | 0 |
| O | 150 | 100 | 50 | 1500 | 6000 | 63,36 | 14,81 | 0 | 0 |
| P | 150 | 100 | 0 | 2000 | 6000 | 62,74 | 15,50 | 0 | 0 |

Cada fila: 2.904 combates del modo; 264 casos «solo».

### Modo 3 (RU3)

Parámetros, en orden: `Modo3VelocidadGiro`, `Modo3VelocidadBarrido`, `Modo3TiempoMinimo`, `Modo3TiempoGiro`, `Modo3TiempoMax`.

| Variante | Giro | Barrido | Mín. | T. giro | Máx. | Gana % | Cae % | Propias | Solo |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| BASE | 150 | 120 | 0 | 500 | 1500 | 50,52 | 26,00 | 0 | 0 |
| A | 120 | 100 | 0 | 700 | 1500 | 46,59 | 30,61 | 0 | 0 |
| B | 100 | 80 | 0 | 900 | 1500 | 40,91 | 39,74 | 0 | 0 |
| C | 150 | 100 | 0 | 800 | 2000 | 49,52 | 27,13 | 0 | 0 |
| D | 180 | 120 | 0 | 650 | 1500 | 53,31 | 22,38 | 0 | 0 |
| E | 120 | 120 | 0 | 1000 | 2000 | 45,73 | 30,75 | 0 | 0 |
| F | 150 | 100 | 0 | 1000 | 2500 | 49,45 | 27,07 | 0 | 0 |
| G | 200 | 120 | 0 | 300 | 1000 | 53,13 | 21,38 | 0 | 0 |
| H | 255 | 120 | 0 | 300 | 1500 | 52,75 | 21,25 | 0 | 0 |
| I | 180 | 120 | 0 | 500 | 1000 | 53,13 | 22,56 | 0 | 0 |
| J | 150 | 120 | 0 | 300 | 1000 | 50,45 | 25,03 | 0 | 0 |
| K | 200 | 120 | 0 | 500 | 1500 | 53,48 | 21,11 | 0 | 0 |
| L | 180 | 120 | 0 | 650 | 1000 | 53,17 | 22,52 | 0 | 0 |
| M | 180 | 100 | 0 | 650 | 1500 | 53,27 | 22,42 | 0 | 0 |
| N | 200 | 120 | 0 | 300 | 1500 | 53,13 | 21,38 | 0 | 0 |
| O | 180 | 150 | 0 | 650 | 1500 | 53,31 | 22,38 | 0 | 0 |
| P | 200 | 120 | 0 | 650 | 1500 | 53,48 | 21,07 | 0 | 0 |

Cada fila: 2.904 combates del modo; 264 casos «solo».

### Modo 4 (RU4)

Parámetros, en orden: `Modo4VelocidadAvance`, `Modo4VelocidadGiro`, `Modo4EstabilizarFrontal`, `Modo4VelocidadConfirmacion`, `Modo4TiempoMinimo`, `Modo4TiempoMax`.

| Variante | Avance | Giro | Estab. (0/1) | PWM conf. | Mín. | Máx. | Gana % | Cae % | Propias | Solo |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| BASE | 150 | 255 | 0 | 0 | 0 | 1500 | 46,21 | 28,86 | 0 | 0 |
| A | 200 | 150 | 0 | 0 | 0 | 1500 | 37,05 | 44,46 | 0 | 0 |
| B | 220 | 120 | 0 | 0 | 0 | 1500 | 33,26 | 49,69 | 0 | 0 |
| C | 180 | 100 | 0 | 0 | 0 | 1500 | 29,58 | 52,89 | 0 | 0 |
| D | 200 | 200 | 0 | 0 | 0 | 1500 | 41,60 | 35,92 | 0 | 0 |
| E | 230 | 180 | 0 | 0 | 0 | 1500 | 40,67 | 38,95 | 0 | 0 |
| F | 200 | 120 | 0 | 0 | 0 | 2000 | 32,51 | 49,90 | 0 | 0 |
| G | 100 | 255 | 0 | 0 | 0 | 1500 | 45,97 | 28,99 | 0 | 0 |
| H | 120 | 255 | 0 | 0 | 0 | 1500 | 46,04 | 28,93 | 0 | 0 |
| I | 150 | 255 | 0 | 0 | 0 | 750 | 45,80 | 28,79 | 0 | 0 |
| J | 150 | 200 | 0 | 0 | 0 | 1000 | 41,53 | 36,02 | 0 | 0 |
| K | 150 | 255 | 0 | 0 | 0 | 500 | 45,18 | 29,68 | 0 | 0 |
| L | 200 | 255 | 0 | 0 | 0 | 1500 | 46,07 | 28,96 | 0 | 0 |
| M | 200 | 255 | 0 | 0 | 0 | 250 | 45,32 | 30,75 | 0 | 0 |
| N | 150 | 255 | 0 | 0 | 0 | 250 | 45,45 | 30,65 | 0 | 0 |
| O | 150 | 255 | 0 | 0 | 0 | 100 | 44,77 | 32,23 | 0 | 0 |
| P | 150 | 255 | 0 | 0 | 0 | 50 | 43,66 | 33,40 | 0 | 0 |
| U | 150 | 255 | 0 | 0 | 0 | 3000 | 46,42 | 29,34 | 0 | 0 |
| V | 150 | 255 | 0 | 0 | 0 | 6000 | 46,35 | 29,61 | 0 | 0 |
| W | 150 | 255 | 1 | 0 | 0 | 1500 | 44,35 | 29,96 | 0 | 0 |
| X | 150 | 255 | 1 | 100 | 0 | 1500 | 49,76 | 30,10 | 0 | 0 |
| Y | 150 | 255 | 1 | 200 | 0 | 1500 | 46,90 | 29,37 | 0 | 0 |
| Z | 150 | 255 | 1 | 255 | 0 | 1500 | 47,38 | 28,58 | 0 | 0 |
| AA | 150 | 255 | 1 | 150 | 0 | 1500 | 48,42 | 29,20 | 0 | 0 |
| AB | 150 | 255 | 1 | 225 | 0 | 1500 | 47,80 | 28,51 | 0 | 0 |

Cada fila: 2.904 combates del modo; 264 casos «solo».

### Prototipos de búsqueda retirados (modo 4)

No se retiene su código: empeoran el resultado o no lo cambian. Todos usan avance 150, giro 255, mínimo 0, máximo 1500 y 20 ms antes de buscar. Q/R buscan solo si `p.visto == 0`; S/T usan `decision.rival == 0 && !decision.enemigoFrente`. Durante la búsqueda giran hacia DIP3; el frontal confirmado y el borde conservan prioridad. No son configuraciones reproducibles únicamente con los parámetros de la versión final.

| Variante | Giro de búsqueda (ms) | Gana % | Cae % | Propias | Solo |
| --- | ---: | ---: | ---: | ---: | ---: |
| Q | 250 | 46,21 | 28,86 | 0 | 0 |
| R | 400 | 46,21 | 28,86 | 0 | 0 |
| S | 250 | 39,50 | 39,29 | 0 | 0 |
| T | 400 | 44,73 | 31,37 | 0 | 0 |

## Composición de las confirmaciones

| Batería | Modo 1 | Modo 2 | Modo 3 | Modo 4 |
| --- | --- | --- | --- | --- |
| confirmacion_a | I | O | P | AB |
| confirmacion_b | N | M | K | Z |

En el modo 2, M repite exactamente los parámetros de C y aparece agrupado bajo C en la tabla. Los JSON de cada confirmación contienen los valores completos usados en sus cuatro modos.

## Confirmación (30.492 combates por batería)

| Batería | Modo | Combates | Victorias | Gana % | Caídas | Cae % | Propias | Caídas / casos solo |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| BASE_30000 | 1 | 4356 | 2751 | 63,15 | 842 | 19,33 | 0 | 0/396 |
| BASE_30000 | 2 | 8712 | 5414 | 62,14 | 1722 | 19,77 | 0 | 0/792 |
| BASE_30000 | 3 | 8712 | 4292 | 49,27 | 2375 | 27,26 | 0 | 0/792 |
| BASE_30000 | 4 | 8712 | 3939 | 45,21 | 2646 | 30,37 | 0 | 0/792 |
| CONFIRMACION_A_30000 | 1 | 4356 | 2757 | 63,29 | 826 | 18,96 | 0 | 0/396 |
| CONFIRMACION_A_30000 | 2 | 8712 | 5562 | 63,84 | 1302 | 14,94 | 0 | 0/792 |
| CONFIRMACION_A_30000 | 3 | 8712 | 4583 | 52,61 | 1952 | 22,41 | 0 | 0/792 |
| CONFIRMACION_A_30000 | 4 | 8712 | 4040 | 46,37 | 2636 | 30,26 | 0 | 0/792 |
| CONFIRMACION_B_30000 | 1 | 4356 | 2748 | 63,09 | 703 | 16,14 | 0 | 0/396 |
| CONFIRMACION_B_30000 | 2 | 8712 | 5482 | 62,92 | 1384 | 15,89 | 0 | 0/792 |
| CONFIRMACION_B_30000 | 3 | 8712 | 4584 | 52,62 | 1953 | 22,42 | 0 | 0/792 |
| CONFIRMACION_B_30000 | 4 | 8712 | 4005 | 45,97 | 2650 | 30,42 | 0 | 0/792 |

## Reproducción

La referencia se midió inicialmente sin `--param`, antes de editar las cabeceras. Para repetir BASE con las cabeceras finales, desactivar la nueva estabilización y especificar todos los valores originales:

```bash
python3 simulacion/bateria.py --rumania --muestras 30000 --json /tmp/x3_base_reproducida.json \
  --param Modo1VelocidadGiro=200 --param Modo1VelocidadAvanceEspera=50 \
  --param Modo1TiempoMinimo=0 --param Modo1TiempoQuieto=3000 \
  --param Modo1TiempoRecibir=200 --param Modo1TiempoMax=9000 \
  --param Modo2VelocidadGiro=150 --param Modo2VelocidadAvanceEspera=100 \
  --param Modo2TiempoMinimo=300 --param Modo2TiempoQuieto=3000 \
  --param Modo2TiempoMax=6000 --param Modo3VelocidadGiro=150 \
  --param Modo3VelocidadBarrido=120 --param Modo3TiempoMinimo=0 \
  --param Modo3TiempoGiro=500 --param Modo3TiempoMax=1500 \
  --param Modo4VelocidadAvance=150 --param Modo4VelocidadGiro=255 \
  --param Modo4TiempoMinimo=0 --param Modo4TiempoMax=1500 \
  --param Modo4EstabilizarFrontal=0 --param Modo4VelocidadConfirmacion=0
```

Para repetir una fila de exploración, usar `--muestras 10000` y **todos** los parámetros de esa fila. Para una batería completa, especificar los parámetros de sus cuatro modos. Los prototipos Q–T se retiraron y se describen aparte; sus parámetros de búsqueda no forman parte del firmware final.

Para repetir los ganadores ya aplicados, sin sustituciones:

```bash
python3 simulacion/bateria.py --rumania --muestras 30000 --json /tmp/x3_final_reproducido.json
```

Los JSON completos, parámetros exactos y registros de esta sesión están en `/tmp/x3_codex_resultados/`; son temporales. Las tablas de este informe conservan las cifras.

## Validación del firmware final

- `pio test -e native`: 84 correctas, 14 omitidas por configuración y 0 fallos (98 casos).
- `pio run -e leonardo -e monitoreo`: ambos entornos compilados correctamente.

| Entorno | Flash usada / disponible (bytes) | RAM usada / disponible (bytes) |
| --- | ---: | ---: |
| leonardo | 27382 / 28672 | 787 / 2560 |
| monitoreo | 28126 / 28672 | 793 / 2560 |

Las comprobaciones usaron una caché aislada de PlatformIO en `/tmp`, con los paquetes locales; se conservaron las opciones de `platformio.ini`. Se verificó que todas las constantes finales coinciden con sus sustituciones confirmadas y que solo cambian las cuatro cabeceras autorizadas, `Modo4Agresivo.cpp` y este informe.

Resultados del modelo de simulación indicado, con semillas 1–33; la confirmación amplía las semillas 1–11 de la exploración. No se atribuye significación estadística a diferencias pequeñas entre candidatas. No se hizo commit ni push.
