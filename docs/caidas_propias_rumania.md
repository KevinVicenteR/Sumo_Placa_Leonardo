# Diagnóstico de caídas propias en Rumania

Fecha: 27 de octubre de 2026. Rama `boldenona`, HEAD `0b1daddff6e354f7508d6916299b627df86e8a49`, firmware actual del workspace (incluidos los cambios locales previos), motores de 700 rpm. Solo diagnóstico: no se modificaron fuentes de firmware, scripts Python existentes ni resultados de candidatos. Se respetaron los cambios locales que ya existían al empezar.

## Base, reproducción y alcance

`simulacion/resultados/candidatos/base.json`: 924 casillas (7 salidas × 11 rivales × 12 factores), 55 semillas por casilla (1–55), **50 820 combates, 25 005 victorias, 16 699 caídas totales y 24 caídas propias**. Tasa propia: 0,04723 %; victorias: 49,203 %.

Se reprodujeron las 19 casillas afectadas completas: 1 045 combates, mediante `compilar({})` y `ejecutar()` de `simular.py`. Coinciden tanto las caídas propias como las victorias de las 19 casillas con el JSON original. Se localizaron y repitieron individualmente las **24 caídas**, con igualdad exacta del resultado CSV al repetir la semilla, y se guardaron sus trayectorias con `--tray`. No se ha vuelto a ejecutar toda la base de 50 820 para este diagnóstico.

Script: [`diagnostico_rumania.py`](../simulacion/build/diagnostico_rumania.py). Datos: [`resumen.json`](../simulacion/build/diagnostico_base/resumen.json). Los archivos de `build/` son artefactos locales y pueden quedar ignorados por Git.

```sh
python3 simulacion/build/diagnostico_rumania.py
```

## Casillas con caídas propias
| Round | Rival | Factor | Propias / 55 | Semillas |
| --- | --- | --- | --- | --- |
| RU3 de lado, rival der | embiste rápido | LiPo recién cargada (8,4 V) | 1 | 2 |
| RU3 de lado, rival izq | embiste rápido | LiPo recién cargada (8,4 V) | 1 | 2 |
| RU4 de espaldas, gira der | solo | Llantas gastadas (las dos) | 1 | 34 |
| RU4 de espaldas, gira izq | solo | Llantas gastadas (las dos) | 1 | 34 |
| RU3 de lado, rival der | solo | Sensor de piso lento (40 ms) | 4 | 12, 14, 34, 49 |
| RU4 de espaldas, gira der | solo | Sensor de piso lento (40 ms) | 1 | 55 |
| RU4 de espaldas, gira izq | solo | Sensor de piso lento (40 ms) | 1 | 55 |
| RU1 de frente | solo | Reflejos (detecciones falsas) | 1 | 14 |
| RU4 de espaldas, gira der | deambula | Reflejos (detecciones falsas) | 1 | 17 |
| RU4 de espaldas, gira izq | deambula | Reflejos (detecciones falsas) | 1 | 17 |
| RU1 de frente | solo | IR del rival (sensores cegados) | 1 | 36 |
| RU2 de frente, DIP3 der | solo | IR del rival (sensores cegados) | 2 | 13, 49 |
| RU2 de frente, DIP3 izq | solo | IR del rival (sensores cegados) | 2 | 13, 49 |
| RU4 de espaldas, gira der | solo | IR del rival (sensores cegados) | 1 | 52 |
| RU4 de espaldas, gira izq | solo | IR del rival (sensores cegados) | 1 | 52 |
| RU2 de frente, DIP3 der | woodpecker | Todo junto (peor caso) | 1 | 52 |
| RU2 de frente, DIP3 izq | woodpecker | Todo junto (peor caso) | 1 | 52 |
| RU4 de espaldas, gira der | deambula | Todo junto (peor caso) | 1 | 42 |
| RU4 de espaldas, gira izq | deambula | Todo junto (peor caso) | 1 | 42 |

### Por round

| Por round | Caídas propias |
| --- | --- |
| RU1 de frente | 2 |
| RU2 de frente, DIP3 der | 3 |
| RU2 de frente, DIP3 izq | 3 |
| RU3 de lado, rival der | 5 |
| RU3 de lado, rival izq | 1 |
| RU4 de espaldas, gira der | 5 |
| RU4 de espaldas, gira izq | 5 |

### Por rival

| Por rival | Caídas propias |
| --- | --- |
| deambula | 4 |
| embiste rápido | 2 |
| solo | 16 |
| woodpecker | 2 |

### Por factor

| Por factor | Caídas propias |
| --- | --- |
| IR del rival (sensores cegados) | 7 |
| LiPo recién cargada (8,4 V) | 2 |
| Llantas gastadas (las dos) | 2 |
| Reflejos (detecciones falsas) | 3 |
| Sensor de piso lento (40 ms) | 6 |
| Todo junto (peor caso) | 4 |

Los grupos omitidos tienen cero caídas propias. Las salidas derecha/izquierda producen varias trayectorias equivalentes (en algunos casos solo cambia el ángulo en 2π); no son 24 mecanismos independientes.

## Qué significa la evidencia

El simulador marca `empujado = (t − ultimoContacto < 0.3)`, usando proximidad de los cuerpos (radio conjunto más 5 mm). Por tanto, «propia» significa **sin contacto durante los últimos 0,3 s**, no ausencia de choque en todo el combate. Veinte caídas nunca tuvieron contacto; las cuatro restantes son las dos contra embiste rápido y las dos contra woodpecker.

`causa` clasifica la **orden de motores al caer**, no la causa física. De las 24: 16 ordenan retroceso, 4 están en 0/0, 2 giran y 2 avanzan en curva. En dos de las órdenes de retroceso el robot todavía se mueve hacia delante.

Las trayectorias se muestrean cada 10 ms, sin cabecera: `t,x,y,theta,enemigo_x,enemigo_y,motor_izq,motor_der,linea,v_longitudinal,v_lateral,f_ext`. Las posiciones y velocidades están en metros y segundos; los motores, en PWM con signo. `linea` es **blanco geométrico bajo al menos un QTR**, no la lectura ADC retrasada/filtrada ni la decisión del firmware. Por eso pueden aparecer órdenes nuevas cuando ese indicador ya ha cambiado. No se instrumentó la fase interna; las fases que siguen se deducen de las órdenes y de la máquina de estados.

En `fisica.h`, tanto `fantasma(i)` como `saturado(i)` hacen que `sensorEnemigo()` devuelva verdadero: el factor llamado «IR del rival (sensores cegados)» genera detecciones activas, incluso sin rival físico. Puede cortar la evasión por falsas detecciones; no debe interpretarse como pérdida pura de visión.

El último contacto indicado abajo se reconstruye con la geometría de la trayectoria (centro del cuerpo 3,3 cm delante del eje, umbral 11 cm), con precisión aproximada de 10 ms y posiciones redondeadas. El indicador `empujado` del resultado se calcula a la resolución interna y es la referencia para clasificar. `t_contacto` es duración **acumulada**, no tiempo desde el último contacto.

La caída simulada ocurre cuando el centro de masa (2 cm delante del eje) supera el radio 38,5 cm; `--vuelco-pala` está desactivado. No basta con que una esquina sobresalga. Fuera del dohyo los QTR vuelven a leer oscuro: perder el blanco no demuestra que se haya vuelto al negro interior.

### A. Retroceso que termina apuntando hacia fuera: velocidad, reentradas y falta de sensores traseros

En `ManiobraEvasion.cpp`, una nueva línea durante el giro o las pausas devuelve a `Retrocediendo` si no agotó 700 ms. Al entrar el segundo sensor se pasa del retroceso de un sensor (118 PWM) al completo (255 PWM); según el modo, también se prolonga su mínimo. `sinRetroceso` se conserva desde el comienzo de la evasión: con `SalirGirandoSiNoAvanza=false` siempre se retrocede, aunque el giro haya cambiado la relación entre la trasera y el borde. El negro continuo y los temporizadores son la condición de salida; no hay comprobación de margen trasero.

- **RU3 derecha / embiste rápido / LiPo 8,4 V / semilla 2**: cae a 11,908 s. Retroceso −255/−255 desde 11,30; pausa 11,50; giro −75/+75 a 11,58; nueva línea y −255/−255 a 11,73. A 11,84 pierde el blanco, pero sigue retrocediendo; a 11,90 lleva −1,268 m/s y cae con −1,28 m/s. Último contacto muestreado 11,14: unos 0,768 s antes. La salida final sucede durante una evasión reactivada, sin empuje reciente. [Trayectoria](../simulacion/build/diagnostico_base/casilla_00_semilla_2.csv).

- **RU4 derecha / solo / llantas gastadas / semilla 34**: −255/−255 a 2,89; pausa a 3,20; giro a 3,28; a 3,30 se interrumpe el giro con avance sobre una rueda, coherente con corte por detección del rival (falsa, pues está solo); vuelve a −118/−118 a 3,40 y cae a 3,532 s, −0,42 m/s. Sigue habiendo blanco geométrico en la última muestra. Ausencia total de contacto. [Trayectoria](../simulacion/build/diagnostico_base/casilla_02_semilla_34.csv). El caso izquierdo es equivalente.

- **RU4 derecha / deambula / reflejos / semilla 17**: retroceso −118/−118; a 3,00 el QTR entra en blanco mientras la trasera sale; a 3,25 vuelve a perder el blanco y a 3,268 cae, −0,45 m/s. Nunca hubo contacto. Ver la línea no garantiza que retroceder sea la dirección segura. [Trayectoria](../simulacion/build/diagnostico_base/casilla_08_semilla_17.csv).

El cálculo actual `TiempoRetrocesoAmbos + RetrocesoExtraAVelocidadMaxima * velocidad / 255` puede exigir hasta 500 ms, y se permite seguir separándose hasta 700 ms. Con la velocidad libre nominal de 700 rpm y rueda de 3 cm (~1,10 m/s antes del factor de batería), estos tiempos permiten recorrer una fracción grande del dohyo sin ninguna observación trasera.

### B. Ordenar 0/0 durante 80 ms no detiene la inercia

- **RU3 derecha / solo / piso lento / semilla 14**: ve blanco físico a 2,37; vuelve a −255/−255 a 2,40; deja de ver blanco a 2,54; manda 0/0 a 2,59 y cae a 2,609, aún a −1,03 m/s. A 2,60 el eje ya está a 39,0 cm del centro. No hay rival. [Trayectoria](../simulacion/build/diagnostico_base/casilla_04_semilla_14.csv).

La fase `Frenando` de `ManiobraEvasion.cpp` llama a `mando.mover(...,0,0,true)`. En el modelo actual, `estadoPuente()` interpreta PWM cero como **libre**, sin fuerza eléctrica. La pausa de 80 ms es un tiempo de espera, no una garantía de frenada; al girar después aún puede quedar velocidad de traslación. Este resultado depende del modelo de puente y debe contrastarse con el IFX9201SG real, sin asumir encoders inexistentes.

### C. Giro con el eje al borde: el centro de masa sí se desplaza

- **RU3 derecha / solo / piso lento / semilla 49**: retroceso desde 2,43, pausa a 2,58; el blanco desaparece a 2,80 y comienza giro +75/−75 a 2,85. A 2,90 el eje está a ~38,5 cm y la velocidad media casi es cero; al girar, el centro de masa pasa de radio ~37,6 cm a 38,5 cm y cae a 3,025 s. No hay rival. En la última muestra un QTR está a 43,0 cm y el otro a 35,1 cm: ambos informan oscuro aunque uno esté fuera. [Trayectoria](../simulacion/build/diagnostico_base/casilla_04_semilla_49.csv).

El comentario de `separarse()` que identifica giro en sitio con centro inmóvil solo es cierto para el eje. Con ruedas traseras, cuerpo y centro de masa tienen un barrido distinto; girar sobre el eje no garantiza mantener el apoyo.

### D. Corte prematuro de la evasión y avance con los QTR ya fuera

- **RU4 derecha / deambula / todo junto / semilla 42**: blanco físico a 11,33; retroceso −118 a 11,35 y −255 a 11,39; negro a 11,41; pausa a 11,51; giro −75/+75 a 11,72. A 11,74 vuelve al control normal (+4/+79) tras unos 20 ms de giro, coherente con `TiempoConfirmarRivalEnEvasion=20`. Avanza hasta +98/+100 y cae a 11,966 s, +0,28 m/s. Nunca tuvo contacto; la última línea física queda 0,566 s antes. [Trayectoria](../simulacion/build/diagnostico_base/casilla_17_semilla_42.csv).

El corte temprano borra `salidaSuave` y restituye `limiteSalida=255`; además, la salida suave normal solo se aplica a evasiones que terminan como `unSensor`. Es una vía concreta para volver a avanzar antes de recuperar una orientación segura. El estimador solo limita avance y vuelve a 255 cuando deja de ser confiable (`LimitadorBorde.cpp`); no limita los retrocesos anteriores ni resuelve el negro ambiguo exterior.

### E. Frenada frontal tardía tras contacto antiguo

- **RU2 derecha / woodpecker / todo junto / semilla 52**: último contacto ~9,24 s (8,43 s acumulados). A 9,60 avanza a ~1,003 m/s con +255/+255; a 9,65 ya manda −118/−118 y un QTR está fuera (39,6 cm) mientras el otro pisa blanco (37,9 cm). A 9,70 ambos están fuera; manda −255/−255 pero sigue a +0,744 m/s. Cae a 9,722 s, aún +0,65 m/s: unos 0,482 s desde el contacto. No es una salida trasera aunque `causa=retroceso`; es frenada frontal insuficiente con sensor retrasado y menor adherencia. [Trayectoria](../simulacion/build/diagnostico_base/casilla_15_semilla_52.csv). El caso izquierdo coincide.

## Inventario de las 24 reproducciones

| Round | Rival | Factor | Semilla | Caída (s) | PWM última muestra | v al caer (m/s) | Blanco última muestra | Desde contacto | Trayectoria |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| RU3 de lado, rival der | embiste rápido | LiPo recién cargada (8,4 V) | 2 | 11.908 | -255/-255 | -1.28 | 0 | ~0.768 s | [CSV](../simulacion/build/diagnostico_base/casilla_00_semilla_2.csv) |
| RU3 de lado, rival izq | embiste rápido | LiPo recién cargada (8,4 V) | 2 | 4.274 | -255/-255 | -0.82 | 1 | ~0.634 s | [CSV](../simulacion/build/diagnostico_base/casilla_01_semilla_2.csv) |
| RU4 de espaldas, gira der | solo | Llantas gastadas (las dos) | 34 | 3.532 | -118/-118 | -0.42 | 1 | sin contacto | [CSV](../simulacion/build/diagnostico_base/casilla_02_semilla_34.csv) |
| RU4 de espaldas, gira izq | solo | Llantas gastadas (las dos) | 34 | 3.532 | -118/-118 | -0.42 | 1 | sin contacto | [CSV](../simulacion/build/diagnostico_base/casilla_03_semilla_34.csv) |
| RU3 de lado, rival der | solo | Sensor de piso lento (40 ms) | 12 | 2.292 | -255/-255 | -0.87 | 1 | sin contacto | [CSV](../simulacion/build/diagnostico_base/casilla_04_semilla_12.csv) |
| RU3 de lado, rival der | solo | Sensor de piso lento (40 ms) | 14 | 2.609 | 0/0 | -1.03 | 0 | sin contacto | [CSV](../simulacion/build/diagnostico_base/casilla_04_semilla_14.csv) |
| RU3 de lado, rival der | solo | Sensor de piso lento (40 ms) | 34 | 3.578 | -255/-255 | -0.87 | 1 | sin contacto | [CSV](../simulacion/build/diagnostico_base/casilla_04_semilla_34.csv) |
| RU3 de lado, rival der | solo | Sensor de piso lento (40 ms) | 49 | 3.025 | 75/-75 | -0.00 | 0 | sin contacto | [CSV](../simulacion/build/diagnostico_base/casilla_04_semilla_49.csv) |
| RU4 de espaldas, gira der | solo | Sensor de piso lento (40 ms) | 55 | 5.636 | -255/-255 | -0.97 | 1 | sin contacto | [CSV](../simulacion/build/diagnostico_base/casilla_05_semilla_55.csv) |
| RU4 de espaldas, gira izq | solo | Sensor de piso lento (40 ms) | 55 | 5.636 | -255/-255 | -0.97 | 1 | sin contacto | [CSV](../simulacion/build/diagnostico_base/casilla_06_semilla_55.csv) |
| RU1 de frente | solo | Reflejos (detecciones falsas) | 14 | 17.205 | 0/0 | -0.34 | 0 | sin contacto | [CSV](../simulacion/build/diagnostico_base/casilla_07_semilla_14.csv) |
| RU4 de espaldas, gira der | deambula | Reflejos (detecciones falsas) | 17 | 3.268 | -118/-118 | -0.45 | 0 | sin contacto | [CSV](../simulacion/build/diagnostico_base/casilla_08_semilla_17.csv) |
| RU4 de espaldas, gira izq | deambula | Reflejos (detecciones falsas) | 17 | 3.268 | -118/-118 | -0.45 | 0 | sin contacto | [CSV](../simulacion/build/diagnostico_base/casilla_09_semilla_17.csv) |
| RU1 de frente | solo | IR del rival (sensores cegados) | 36 | 12.185 | 75/-75 | -0.51 | 0 | sin contacto | [CSV](../simulacion/build/diagnostico_base/casilla_10_semilla_36.csv) |
| RU2 de frente, DIP3 der | solo | IR del rival (sensores cegados) | 13 | 9.200 | -255/-255 | -0.91 | 1 | sin contacto | [CSV](../simulacion/build/diagnostico_base/casilla_11_semilla_13.csv) |
| RU2 de frente, DIP3 der | solo | IR del rival (sensores cegados) | 49 | 25.101 | 0/0 | -0.36 | 0 | sin contacto | [CSV](../simulacion/build/diagnostico_base/casilla_11_semilla_49.csv) |
| RU2 de frente, DIP3 izq | solo | IR del rival (sensores cegados) | 13 | 9.200 | -255/-255 | -0.91 | 1 | sin contacto | [CSV](../simulacion/build/diagnostico_base/casilla_12_semilla_13.csv) |
| RU2 de frente, DIP3 izq | solo | IR del rival (sensores cegados) | 49 | 25.101 | 0/0 | -0.36 | 0 | sin contacto | [CSV](../simulacion/build/diagnostico_base/casilla_12_semilla_49.csv) |
| RU4 de espaldas, gira der | solo | IR del rival (sensores cegados) | 52 | 24.459 | -255/-255 | -1.10 | 0 | sin contacto | [CSV](../simulacion/build/diagnostico_base/casilla_13_semilla_52.csv) |
| RU4 de espaldas, gira izq | solo | IR del rival (sensores cegados) | 52 | 24.459 | -255/-255 | -1.10 | 0 | sin contacto | [CSV](../simulacion/build/diagnostico_base/casilla_14_semilla_52.csv) |
| RU2 de frente, DIP3 der | woodpecker | Todo junto (peor caso) | 52 | 9.722 | -255/-255 | 0.65 | 0 | ~0.482 s | [CSV](../simulacion/build/diagnostico_base/casilla_15_semilla_52.csv) |
| RU2 de frente, DIP3 izq | woodpecker | Todo junto (peor caso) | 52 | 9.722 | -255/-255 | 0.65 | 0 | ~0.482 s | [CSV](../simulacion/build/diagnostico_base/casilla_16_semilla_52.csv) |
| RU4 de espaldas, gira der | deambula | Todo junto (peor caso) | 42 | 11.966 | 98/100 | 0.28 | 0 | sin contacto | [CSV](../simulacion/build/diagnostico_base/casilla_17_semilla_42.csv) |
| RU4 de espaldas, gira izq | deambula | Todo junto (peor caso) | 42 | 11.966 | 98/100 | 0.28 | 0 | sin contacto | [CSV](../simulacion/build/diagnostico_base/casilla_18_semilla_42.csv) |

La orden de la última muestra puede preceder hasta 10 ms al instante de caída; `causa` y `vel_caida` corresponden al instante interno exacto.

## Cambios de parámetros ensayados

Se repitió un control del firmware actual y cada variante con `--rumania --muestras 10000`. Son **11 semillas (1–11) en cada una de las mismas 924 casillas: 10 164 combates**. Es el prefijo de semillas de la base, no una muestra aleatoria nueva. Comparar el número bruto de caídas con 24/50 820 sería engañoso: abajo se compara principalmente contra ese control de igual tamaño y se conserva la base completa como referencia.

La paralelización original usa `ThreadPoolExecutor(os.cpu_count())` y no tiene opción `--workers`. Se limitó sin editarlo mediante `sitecustomize.py` local y `PYTHONPATH`. El control y la primera variante usaron un trabajador cada uno. Al terminar el control, las siguientes variantes usaron dos trabajadores y esperaron a que acabara la revalidación de semillas de la variante anterior. Las variantes se ejecutaron secuencialmente; cada revalidación usó un trabajador. Como máximo hubo dos simulaciones activas simultáneas, sin contar sus procesos padre que esperan al hijo. El mismo `sitecustomize.py` añade progreso cada 77 casillas sin alterar resultados.

| Ensayo | Combates | Propias | Tasa propias | Victorias | Tasa victorias | Δ victorias vs control | Caídas totales |
| --- | --- | --- | --- | --- | --- | --- | --- |
| base_50k | 50820 | 24 | 0.04723 % | 25005 | 49.203 % | — | 16699 |
| control_10k | 10164 | 2 | 0.01968 % | 5065 | 49.833 % | +0.000 pp | 3224 |
| sin_corte | 10164 | 20 | 0.19677 % | 4948 | 48.682 % | -1.151 pp | 3386 |
| retroceso_moderado | 10164 | 1 | 0.00984 % | 5039 | 49.577 % | -0.256 pp | 3190 |
| giro_si_no_avanza | 10164 | 2 | 0.01968 % | 5067 | 49.852 % | +0.020 pp | 3221 |

### `control_10k`

Parámetros: sin cambios.

```sh
DIAGNOSTICO_WORKERS=1 PYTHONPATH="$PWD/simulacion/build/limite_procesos" python3 simulacion/bateria.py --rumania --muestras 10000 --json simulacion/build/control_10k.json
```

[JSON](../simulacion/build/control_10k.json) · [salida completa](../simulacion/build/control_10k.log).

| Round | Rival | Factor | Propias / 11 |
| --- | --- | --- | --- |
| RU3 de lado, rival der | embiste rápido | LiPo recién cargada (8,4 V) | 1 |
| RU3 de lado, rival izq | embiste rápido | LiPo recién cargada (8,4 V) | 1 |

### `sin_corte`

Parámetros: `CortarEvasionAlVerRival=false`.

```sh
DIAGNOSTICO_WORKERS=1 PYTHONPATH="$PWD/simulacion/build/limite_procesos" python3 simulacion/bateria.py --rumania --muestras 10000 --param CortarEvasionAlVerRival=false --json simulacion/build/sin_corte.json
```

[JSON](../simulacion/build/sin_corte.json) · [salida completa](../simulacion/build/sin_corte.log).

| Round | Rival | Factor | Propias / 11 |
| --- | --- | --- | --- |
| RU1 de frente | solo | Dohyo con polvo (poco agarre) | 1 |
| RU4 de espaldas, gira der | solo | Sensor de piso lento (40 ms) | 1 |
| RU4 de espaldas, gira izq | solo | Sensor de piso lento (40 ms) | 1 |
| RU1 de frente | solo | IR del rival (sensores cegados) | 1 |
| RU2 de frente, DIP3 der | solo | IR del rival (sensores cegados) | 1 |
| RU2 de frente, DIP3 der | deambula | IR del rival (sensores cegados) | 1 |
| RU2 de frente, DIP3 izq | solo | IR del rival (sensores cegados) | 1 |
| RU2 de frente, DIP3 izq | deambula | IR del rival (sensores cegados) | 1 |
| RU4 de espaldas, gira der | solo | IR del rival (sensores cegados) | 2 |
| RU4 de espaldas, gira izq | solo | IR del rival (sensores cegados) | 2 |
| RU4 de espaldas, gira der | solo | Todo junto (peor caso) | 1 |
| RU4 de espaldas, gira der | quieto | Todo junto (peor caso) | 1 |
| RU4 de espaldas, gira der | contragolpe | Todo junto (peor caso) | 1 |
| RU4 de espaldas, gira der | esquivador | Todo junto (peor caso) | 1 |
| RU4 de espaldas, gira izq | solo | Todo junto (peor caso) | 1 |
| RU4 de espaldas, gira izq | quieto | Todo junto (peor caso) | 1 |
| RU4 de espaldas, gira izq | contragolpe | Todo junto (peor caso) | 1 |
| RU4 de espaldas, gira izq | esquivador | Todo junto (peor caso) | 1 |

Cambios agregados entre casillas: +41 victorias y −158 victorias respecto al control (mismas semillas). No equivale a una tabla de transiciones de combates individuales, que estos JSON no contienen.

### `retroceso_moderado`

Parámetros: `VelocidadRetroceso=140`; `VelocidadRetrocesoUnSensor=90`; `RetrocesoExtraAVelocidadMaxima=100`; `TiempoMaximoRecuperacionBorde=450`.

```sh
DIAGNOSTICO_WORKERS=1 PYTHONPATH="$PWD/simulacion/build/limite_procesos" python3 simulacion/bateria.py --rumania --muestras 10000 --param VelocidadRetroceso=140 --param VelocidadRetrocesoUnSensor=90 --param RetrocesoExtraAVelocidadMaxima=100 --param TiempoMaximoRecuperacionBorde=450 --json simulacion/build/retroceso_moderado.json
```

[JSON](../simulacion/build/retroceso_moderado.json) · [salida completa](../simulacion/build/retroceso_moderado.log).

| Round | Rival | Factor | Propias / 11 |
| --- | --- | --- | --- |
| RU1 de frente | deambula | Todo junto (peor caso) | 1 |

Cambios agregados entre casillas: +83 victorias y −109 victorias respecto al control (mismas semillas). No equivale a una tabla de transiciones de combates individuales, que estos JSON no contienen.

### `giro_si_no_avanza`

Parámetros: `SalirGirandoSiNoAvanza=true`.

```sh
DIAGNOSTICO_WORKERS=1 PYTHONPATH="$PWD/simulacion/build/limite_procesos" python3 simulacion/bateria.py --rumania --muestras 10000 --param SalirGirandoSiNoAvanza=true --json simulacion/build/giro_si_no_avanza.json
```

[JSON](../simulacion/build/giro_si_no_avanza.json) · [salida completa](../simulacion/build/giro_si_no_avanza.log).

| Round | Rival | Factor | Propias / 11 |
| --- | --- | --- | --- |
| RU3 de lado, rival der | embiste rápido | LiPo recién cargada (8,4 V) | 1 |
| RU3 de lado, rival izq | embiste rápido | LiPo recién cargada (8,4 V) | 1 |

Cambios agregados entre casillas: +2 victorias y −0 victorias respecto al control (mismas semillas). No equivale a una tabla de transiciones de combates individuales, que estos JSON no contienen.

## Revalidación de las 24 semillas originalmente fallidas

La batería de 10 164 incluye solo dos de las 24 caídas de la base. Como complemento, cada variante se volvió a ejecutar en los 24 combates originales (round, rival, factor y semilla exactos), guardando también trayectorias de los resultados alternativos. Este conjunto está seleccionado por fallo previo y **no estima una tasa global** ni detecta por sí solo regresiones nuevas.

| Variante | Combates | Propias | Caídas totales | Victorias | Datos |
| --- | --- | --- | --- | --- | --- |
| Firmware base | 24 | 24 | 24 | 0 | Reproducción de base.json |
| sin_corte | 24 | 3 | 5 | 2 | [JSON](../simulacion/build/semillas_sin_corte.json) |
| retroceso_moderado | 24 | 2 | 4 | 2 | [JSON](../simulacion/build/semillas_retroceso_moderado.json) |
| giro_si_no_avanza | 24 | 19 | 19 | 2 | [JSON](../simulacion/build/semillas_giro_si_no_avanza.json) |

```sh
python3 simulacion/build/revalidar_semillas_rumania.py
```

Propias que persisten con `sin_corte`:

- RU3 de lado, rival der / solo / Sensor de piso lento (40 ms), semilla 12; cae a 3.252 s, orden quieto, velocidad -1.09 m/s. [Trayectoria](../simulacion/build/semillas_sin_corte/original_04.csv).
- RU2 de frente, DIP3 der / woodpecker / Todo junto (peor caso), semilla 52; cae a 9.722 s, orden retroceso, velocidad 0.65 m/s. [Trayectoria](../simulacion/build/semillas_sin_corte/original_20.csv).
- RU2 de frente, DIP3 izq / woodpecker / Todo junto (peor caso), semilla 52; cae a 9.722 s, orden retroceso, velocidad 0.65 m/s. [Trayectoria](../simulacion/build/semillas_sin_corte/original_21.csv).


Propias que persisten con `retroceso_moderado`:

- RU2 de frente, DIP3 der / woodpecker / Todo junto (peor caso), semilla 52; cae a 9.722 s, orden retroceso, velocidad 0.65 m/s. [Trayectoria](../simulacion/build/semillas_retroceso_moderado/original_20.csv).
- RU2 de frente, DIP3 izq / woodpecker / Todo junto (peor caso), semilla 52; cae a 9.722 s, orden retroceso, velocidad 0.65 m/s. [Trayectoria](../simulacion/build/semillas_retroceso_moderado/original_21.csv).


Propias que persisten con `giro_si_no_avanza`:

- RU3 de lado, rival der / embiste rápido / LiPo recién cargada (8,4 V), semilla 2; cae a 11.908 s, orden retroceso, velocidad -1.28 m/s. [Trayectoria](../simulacion/build/semillas_giro_si_no_avanza/original_00.csv).
- RU3 de lado, rival izq / embiste rápido / LiPo recién cargada (8,4 V), semilla 2; cae a 4.274 s, orden retroceso, velocidad -0.82 m/s. [Trayectoria](../simulacion/build/semillas_giro_si_no_avanza/original_01.csv).
- RU3 de lado, rival der / solo / Sensor de piso lento (40 ms), semilla 12; cae a 2.292 s, orden retroceso, velocidad -0.87 m/s. [Trayectoria](../simulacion/build/semillas_giro_si_no_avanza/original_04.csv).
- RU3 de lado, rival der / solo / Sensor de piso lento (40 ms), semilla 14; cae a 2.609 s, orden quieto, velocidad -1.03 m/s. [Trayectoria](../simulacion/build/semillas_giro_si_no_avanza/original_05.csv).
- RU3 de lado, rival der / solo / Sensor de piso lento (40 ms), semilla 34; cae a 3.578 s, orden retroceso, velocidad -0.87 m/s. [Trayectoria](../simulacion/build/semillas_giro_si_no_avanza/original_06.csv).
- RU3 de lado, rival der / solo / Sensor de piso lento (40 ms), semilla 49; cae a 3.025 s, orden giro, velocidad -0.00 m/s. [Trayectoria](../simulacion/build/semillas_giro_si_no_avanza/original_07.csv).
- RU4 de espaldas, gira der / solo / Sensor de piso lento (40 ms), semilla 55; cae a 5.636 s, orden retroceso, velocidad -0.97 m/s. [Trayectoria](../simulacion/build/semillas_giro_si_no_avanza/original_08.csv).
- RU4 de espaldas, gira izq / solo / Sensor de piso lento (40 ms), semilla 55; cae a 5.636 s, orden retroceso, velocidad -0.97 m/s. [Trayectoria](../simulacion/build/semillas_giro_si_no_avanza/original_09.csv).
- RU1 de frente / solo / IR del rival (sensores cegados), semilla 36; cae a 12.185 s, orden giro, velocidad -0.51 m/s. [Trayectoria](../simulacion/build/semillas_giro_si_no_avanza/original_13.csv).
- RU2 de frente, DIP3 der / solo / IR del rival (sensores cegados), semilla 13; cae a 9.200 s, orden retroceso, velocidad -0.91 m/s. [Trayectoria](../simulacion/build/semillas_giro_si_no_avanza/original_14.csv).
- RU2 de frente, DIP3 der / solo / IR del rival (sensores cegados), semilla 49; cae a 25.101 s, orden quieto, velocidad -0.36 m/s. [Trayectoria](../simulacion/build/semillas_giro_si_no_avanza/original_15.csv).
- RU2 de frente, DIP3 izq / solo / IR del rival (sensores cegados), semilla 13; cae a 9.200 s, orden retroceso, velocidad -0.91 m/s. [Trayectoria](../simulacion/build/semillas_giro_si_no_avanza/original_16.csv).
- RU2 de frente, DIP3 izq / solo / IR del rival (sensores cegados), semilla 49; cae a 25.101 s, orden quieto, velocidad -0.36 m/s. [Trayectoria](../simulacion/build/semillas_giro_si_no_avanza/original_17.csv).
- RU4 de espaldas, gira der / solo / IR del rival (sensores cegados), semilla 52; cae a 24.459 s, orden retroceso, velocidad -1.10 m/s. [Trayectoria](../simulacion/build/semillas_giro_si_no_avanza/original_18.csv).
- RU4 de espaldas, gira izq / solo / IR del rival (sensores cegados), semilla 52; cae a 24.459 s, orden retroceso, velocidad -1.10 m/s. [Trayectoria](../simulacion/build/semillas_giro_si_no_avanza/original_19.csv).
- RU2 de frente, DIP3 der / woodpecker / Todo junto (peor caso), semilla 52; cae a 9.722 s, orden retroceso, velocidad 0.65 m/s. [Trayectoria](../simulacion/build/semillas_giro_si_no_avanza/original_20.csv).
- RU2 de frente, DIP3 izq / woodpecker / Todo junto (peor caso), semilla 52; cae a 9.722 s, orden retroceso, velocidad 0.65 m/s. [Trayectoria](../simulacion/build/semillas_giro_si_no_avanza/original_21.csv).
- RU4 de espaldas, gira der / deambula / Todo junto (peor caso), semilla 42; cae a 11.966 s, orden curva, velocidad 0.28 m/s. [Trayectoria](../simulacion/build/semillas_giro_si_no_avanza/original_22.csv).
- RU4 de espaldas, gira izq / deambula / Todo junto (peor caso), semilla 42; cae a 11.966 s, orden curva, velocidad 0.28 m/s. [Trayectoria](../simulacion/build/semillas_giro_si_no_avanza/original_23.csv).

## Interpretación de los ensayos

- **Desactivar el corte de evasión: descartar.** En las mismas 10 164 semillas/casillas pasa de 2 a 20 caídas propias (+18), de 5 065 a 4 948 victorias (−117; −1,151 puntos porcentuales) y de 3 224 a 3 386 caídas totales. En las 24 semillas originalmente fallidas quedan solo 3 propias, pero 5 caídas totales: dos pasan a clasificarse como empujadas. Este ejemplo muestra por qué resolver únicamente el conjunto de fallos conocidos puede ocultar regresiones.
- **Retroceso moderado: mejor candidato de los tres para seguir investigando, todavía insuficiente.** Pasa de 2 a 1 caída propia, de 5 065 a 5 039 victorias (−26; −0,256 puntos) y de 3 224 a 3 190 caídas totales. En los 24 fallos conocidos quedan 2 propias y 4 caídas totales; persisten los dos woodpecker/52, que siguen saliendo hacia delante a +0,65 m/s. Reducir de −255 a −140 no cambia en esos casos la desaceleración simulada porque ambos contramandos alcanzan el límite de adherencia mientras el robot avanza; las trayectorias muestran la misma velocidad de frenada.
- **Girar si no avanza: descartar como corrección del borde.** En la batería corta conserva las mismas 2 caídas propias; suma solo 2 victorias (5 067; +0,020 puntos) y reduce las caídas totales a 3 221. En las 24 semillas originalmente fallidas siguen cayéndose por sí solas 19: evita solo 5 de esos fallos. No resuelve las reentradas de una evasión ya activa ni certifica que el giro sea seguro.

La propia que aparece con retroceso moderado es **nueva respecto al control**: RU1 de frente / deambula / todo junto, semilla 2; caída a 2,543 s, curva +98/+100, velocidad +0,28 m/s, sin contacto en los últimos 0,3 s. Se reprodujo la casilla completa de 11 semillas y la semilla individual con `--tray`. Ve línea física a 1,94; retrocede a 1,96 (−90) y 2,01 (−140); pausa a 2,08; gira +75/−75 a 2,29; a 2,31 vuelve al avance, tras unos 20 ms de giro. Pierde el blanco a 2,05 y no vuelve a verlo antes de caer. Es otra instancia del corte prematuro y del negro exterior, ahora con retroceso reducido. [Datos](../simulacion/build/regresion_moderado.json) · [trayectoria](../simulacion/build/regresion_moderado_semilla_2.csv) · [script](../simulacion/build/reproducir_regresion_moderado.py).

Los resultados de la batería corta y de las 24 semillas no deben sumarse directamente: los dos combates de embiste rápido / semilla 2 están en ambos conjuntos. Tampoco se puede extrapolar una reducción de 2 a 1 a una reducción demostrada del 50 % en la tasa real: son eventos escasos y hay salidas simétricas.

## Cambios concretos de lógica propuestos (no implementados ni ensayados)

Los ensayos anteriores cambian únicamente copias de las cabeceras en `build/`. Los siguientes cambios requieren otra revisión de código y una batería nueva; no se presentan como soluciones validadas.

1. **Separar frenada de llegada y retroceso de separación en `ManiobraEvasion.cpp`.** Añadir una fase de frenada con contramando antes de decidir cuánto retroceder. En la pausa posterior al retroceso, usar el mecanismo existente `mando.calcularFreno(TiempoFrenadoRuedas, freno)` (máximo de referencia 80 ms, PWM limitado por `VelocidadFreno=140`), enviando el contramando durante su duración y después 0/0. No continuar al giro simplemente por haber esperado `TiempoFrenado=80`. Son estimaciones basadas en órdenes, sin encoders; se deben acotar para que el contramando no se convierta en otro movimiento sostenido. Comprobar el frenado real del IFX9201SG: el modelo actual deja el motor libre con PWM cero. Evidencia: semillas RU3/14 y RU3/49 con piso lento; el contramando inicial fuerte sigue siendo necesario en woodpecker/52.

2. **Presupuesto único de retroceso por evasión, incluido cada reintento.** Contabilizar el tiempo acumulado de traslación hacia atrás en `separarse()` y `reintentarSiAtascado()`, separado del tiempo de contramando que todavía está frenando avance. Como primer ensayo de lógica, usar `TiempoReintentoRetroceso` (150 ms) como tope acumulado de separación trasera, con la velocidad de un sensor, y pasar a frenada cuando se agote. No renovar ese presupuesto al volver a ver la línea ni al activar el segundo sensor; el temporizador global de 700 ms no sustituye este límite. Al reaparecer blanco durante giro/pausa, no saltar directamente a −255/−255: frenar primero y volver a evaluar la dirección. La duración concreta necesita validación: el límite de 150 ms es una hipótesis de implementación, no un parámetro probado aquí. Evidencia: reentradas de RU3/2, RU3/12 y retroceso hacia fuera de RU4/17.

3. **No convertir automáticamente velocidad estimada baja en giro junto al borde.** La condición `SalirGirandoSiNoAvanza` solo se decide al iniciar la evasión y no certifica posición ni orientación segura; el caso RU3/49 cae girando casi sin velocidad longitudinal. Antes de una reentrada, contrastar el signo de la velocidad estimada con el historial de línea y el rumbo estimado. Si no se puede certificar margen para retroceder o barrer el centro de masa, ejecutar la frenada acotada y no iniciar otro tramo largo a ciegas. Un detector de «negro» no puede autorizar por sí solo esa maniobra: puede ser el exterior.

4. **Conservar una protección tras la línea en `LimitadorBorde.cpp`.** Añadir una bandera de línea observada y conservar el último límite. Después de la primera línea, cuando `confiable()` pase a falso, usar `min(ultimoLimite, VelocidadMinimaEstimador)` en vez de devolver 255. Se propone reutilizar el valor existente de 100 PWM, sin cambiar el parámetro: es un cambio de lógica aún no ensayado. No falsear la confianza elevando sus umbrales ni suponiendo encoders. Este límite solo protege avance; la protección trasera del punto 2 es independiente. Evidencia: RU4/42 reanuda avance con ambos QTR ya fuera; woodpecker/52 llega demasiado rápido con retardo y poco agarre.

5. **Conservar la salida suave al cortar una evasión, también tras ambos sensores.** Al cortar por rival, no ejecutar `salidaSuave=false; limiteSalida=255`. Activar el mismo mecanismo existente de salida suave (`VelocidadSalidaUnSensor=90`, `TiempoSalidaSuaveUnSensor=500`) para cualquier evasión, tanto al cortarla como al terminarla. Conservar la posibilidad de corte y condicionarla al margen estimado: desactivarla globalmente produjo regresiones importantes. Este cambio de lógica reutiliza los parámetros actuales y necesita ensayo propio. Evidencia: corte RU4/42 tras ~20 ms de giro y nuevas caídas al poner `CortarEvasionAlVerRival=false`.

No basta con aplicar solo el punto 4: `MandoMotores::mover()` limita el avance cuando ambas órdenes son no negativas, y deja sin limitar el retroceso de evasión. Tampoco basta con los temporizadores: sin sensores traseros ni encoders, las estimaciones de velocidad, giro y posición no miden el deslizamiento real. El diseño debe tratar explícitamente esa incertidumbre, en lugar de identificar negro con interior o giro en sitio con apoyo seguro.

## Recomendación final

**No adoptar ninguno de estos ajustes como solución a la meta de cero caídas propias.** Ninguno alcanza cero simultáneamente en la batería corta y en las semillas originalmente fallidas. Conservar el firmware actual hasta ensayar la corrección de lógica; el conjunto de retroceso moderado es una referencia útil para la siguiente comparación, no una configuración aprobada para competición.

La siguiente revisión debe priorizar el presupuesto acumulado de retroceso y la frenada efectiva, junto con la protección del avance tras evasión. Después, revalidar tanto los 24 fallos originales como la regresión RU1/deambula/todo junto/semilla 2 y repetir las 924 casillas × 55 semillas, escribiendo los nuevos resultados en `simulacion/build/`. Registrar **cero propias y las victorias en la misma población**; si aparece una caída nueva, estudiar también su trayectoria antes de aceptar la revisión. Una batería adicional con semillas fuera de 1–55 permitirá comprobar si solo se ajustó el conjunto conocido.

Para el robot real, comprobar especialmente frenado a 8,4 V, piso lento y desgaste, y las transiciones tras giro con ruedas traseras. Sin QTR traseros ni encoders, el negro exterior y el deslizamiento no se pueden descartar solo con órdenes y temporizadores. Cero casos en una muestra finita del simulador es un criterio de aceptación experimental, no una garantía física universal.

Se completaron cuatro baterías de 10 164 combates (control más tres variantes), las 19 casillas afectadas de la base, las 24 repeticiones individuales con trayectoria, la revalidación de esas 24 semillas para cada variante y la reproducción de la casilla de regresión del retroceso moderado. No se editaron fuentes protegidas ni se hicieron commits. Los JSON de candidatos originales permanecieron fuera de las rutas de escritura de estos scripts.
