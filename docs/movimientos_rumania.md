# Movimientos rápidos para RoboChallenge Rumania (Boldenona)

Investigación de movimientos de arranque y combate usados por minisumos rápidos, filtrados
para el reglamento de RoboChallenge y para Boldenona (Leonardo, IFX9201SG, JSUMO 700 rpm,
5 × JS40F, 2 × QTR al frente).

## Lo que impone el reglamento (RoboChallenge, mini sumo)

Fuente: [reglamento oficial de mini sumo de RoboChallenge](https://robochallenge.ro/regulation/mini-sumo).

- **Dohyo:** 77 cm, con borde blanco de 25 mm.
- **Líneas de salida (Shikiri-Sen):** de 10 × 1 cm. Todo el robot va detrás de su línea y dentro de la franja que forman las prolongaciones de los extremos de la línea. La orientación es libre, y cada operador coloca su robot a la vez que el otro.
- **Arranque:** por IR del árbitro, **sin espera**, y el robot sale en cuanto llega la señal. Moverse antes de la señal cuesta 2 puntos Yuko.
- **Combate:** 3 rounds, gana quien llega primero a 2, con 3 min en total. Entre rounds hay 1 min como máximo, que es el tiempo para cambiar el DIP.
- **Consecuencia:** como no hay espera de 5 s, **el primer contacto decide la mayoría de los rounds**. El robot que golpea primero de lado o por detrás suele ganar ([BrooksBots, estrategia](http://brooksbots.com/Strategy.html)). Además, la colocación es libre, así que **el rival elige su colocación viendo la nuestra**. Conviene tener respuestas distintas para cada ángulo de salida: de frente, a 45°, de lado y de espaldas.

## Catálogo de movimientos

Los nombres son de la comunidad. Las fuentes enlazadas describen el movimiento o una variante
muy cercana; donde no hay fuente directa lo indico como *práctica común*.

| # | Movimiento | Qué hace | Para qué salida | Riesgo a 700 rpm | Fuente |
|---|---|---|---|---|---|
| M1 | **Golpe frontal inmediato** | Al START, rampa corta y empuje a fondo de frente. Si un sensor a 45° ve al rival, corrige con el diferencial sin frenar. | RU1 de frente | Bajo. Pierde contra la pala más baja y contra el *henka* (esquive). | práctica común; `Modo4Agresivo` actual |
| M2 | **Flanqueo en arco ("banana")** | Sale en arco de radio grande hacia un lado (rueda exterior al 100 % y la interior al 40–60 %) para llegar al costado del rival; al verlo con el frontal o a 45°, cierra y ataca. | RU1/RU2: de frente o a 45° contra rivales que cargan recto | Medio. Hay que cortar el arco por tiempo o al ver el borde. | [Flank, dampedcoast/Sumo-Robot](https://github.com/dampedcoast/Sumo-Robot-), [BrooksBots: "angle 45/90"](http://brooksbots.com/Strategy.html) |
| M3 | **Paso lateral (*henka*) y contragolpe** | Al START, giro corto de 30–45° hacia un lado (o arco breve), espera de 100–250 ms y, cuando el rival pasa por delante cargando recto, golpe de costado. | RU1 contra embestidores | Medio. Si el rival no carga, se pierde la iniciativa. | rival "Esquivador" del informe Codex (Robotic Arena 2019); práctica común |
| M4 | **Giro de 180° con ataque** | Salimos de espaldas al rival, giramos en el sitio a 255/−255 hasta que lo ve el frontal y embestimos. La parte trasera aguanta el primer golpe. | RU4 de espaldas | Bajo si el giro se corta con el frontal y no por tiempo. | [BrooksBots: "hit spin"](http://brooksbots.com/Strategy.html); `Modo4`/legado 1 |
| M5 | **Giro de 90° desde lado (pivote o arco)** | Colocado de lado, gira hacia el rival sobre una rueda o en arco corto y ataca. El arco evita ofrecer el costado. | RU3 de lado | Bajo | `Modo3Lado` actual; [BrooksBots "angle 90"](http://brooksbots.com/Strategy.html) |
| M6 | **Quieto y espera (*sit and wait*)** | No se mueve hasta ver al rival; solo se orienta y golpea cuando está cerca. Gana contra rivales que buscan y sufre contra los que esperan. | Contra rivales lentos o buscadores | Bajo; pierde si el rival también espera | [Sit and Wait, dampedcoast](https://github.com/dampedcoast/Sumo-Robot-); `Modo1`/`Modo2` |
| M7 | **Finta (retroceso corto y carga)** | 60–120 ms hacia atrás para salir del primer choque del rival y luego carga a fondo. Contra quien embiste recto, el rival llega desacelerado o desalineado. | RU1 contra embestidores | **Alto:** detrás está el borde de nuestra franja, a unos 28 cm del borde, y el QTR va delante, así que no ve el borde al retroceder. | práctica común |
| M8 | **Tornado (giro en el sitio buscando)** | Gira sobre su eje hasta ver al rival y carga. Sirve de búsqueda universal tras fallar el primer golpe. | Todas, como respaldo | Bajo | [Tornado, tesis Lancelot UNIVPM](https://tesi.univpm.it/handle/20.500.12075/27301); rival "Tornado" del simulador |
| M9 | **Barrido en círculo (*circle scan*)** | Diferencial de PWM para avanzar en círculos buscando. Es la búsqueda más común según JSUMO. | Respaldo tras perder al rival | Medio, cerca del borde | [JSUMO, estrategias de ataque](https://blog.jsumo.com/sumo-robots-attack-strategies/) |
| M10 | **Empuje en zigzag (*wiggle*)** | Ya en contacto, alterna ±15–30 de diferencial cada 80–150 ms para meter la pala por debajo y que el rival no nos levante. | Durante el empuje | Bajo | práctica común; la [guía de diseño de pala](https://siegfriedt-survey.umb.edu/pti/sumobot-designs-a-deep-dive-into-the-hidden-details) explica por qué gana la pala más baja |
| M11 | **Rodeo por el borde** | Arco que sigue la línea blanca durante 600–900 ms para atacar por detrás. | Contra rivales que esperan | **Alto:** a 700 rpm el QTR delantero tiene poco margen para frenar. | [Flank (900 ms con el borde desactivado), dampedcoast](https://github.com/dampedcoast/Sumo-Robot-) |

## Recomendación por colocación

| Salida (DIP3 = lado) | Qué ya existe | Qué probar |
|---|---|---|
| RU1, de frente (DIP 00) | `Modo1Defensivo` (espera y recibe) | M1 contra el defensivo actual, y M3 (*henka* y contragolpe) |
| RU2, a 45° (DIP 10) | `Modo2Contragolpe` | **M2 flanqueo en arco** como variante: salir en arco hacia el lado DIP3 |
| RU3, de lado (DIP 01) | `Modo3Lado` (pivote) | M5 con arco corto en lugar de pivote puro, comparando los tiempos del arco |
| RU4, de espaldas (DIP 11) | `Modo4Agresivo` | M4: giro de 180° cortado por el frontal, con M10 al llegar al contacto |

## Prioridad de implementación (máx. 5)

1. **M2 flanqueo en arco**: lo que más falta hoy, porque ningún modo sale en arco.
2. **M3 *henka* y contragolpe**: la respuesta al embestidor rápido, que es el rival más común en Rumania sin espera de 5 s.
3. **M4 giro de 180° a fondo** para RU4, revisando cuánto tarda hoy `Modo4` en encarar.
4. **M10 *wiggle*** dentro del empuje (`ControlAtaque`). Es barato en Flash y sirve en todos los modos.
5. **M5 arco desde lado**, como ajuste de `Modo3Lado`.

Descartados para Boldenona: **M7 finta** y **M11 rodeo**, porque los QTR van solo delante y la
marcha atrás o el borde a 700 rpm aumentan las caídas propias. Solo se reconsideran si el
simulador muestra 0 caídas propias.

## Restricciones para implementarlos

- Flash: `monitoreo` usa 27 836 B de 28 672, así que quedan unos 830 B. Los movimientos entran como variantes o parámetros de los modos existentes (`firmware/src/movimiento/rumania/`), no como modos nuevos.
- Patrón: `ResultadoModo paso(const PasoModo&)` con sus `constexpr` en el `.h` del modo. El ataque va por `ControlAtaque::iniciarAtaqueMaximo`, el giro por `GiroLateral`, el borde por `LimitadorBorde`/`EstimadorBorde` y la búsqueda por `BusquedaRival`.
- Criterio de aceptación: `simulacion/bateria.py --rumania` debe dar más victorias y **ninguna caída propia más** en RU1–RU4 frente a la línea base.

## Fuentes

- [RoboChallenge: reglamento de mini sumo](https://robochallenge.ro/regulation/mini-sumo)
- [JSUMO: Sumo Robots — Attack Strategies](https://blog.jsumo.com/sumo-robots-attack-strategies/)
- [BrooksBots: Strategy for Autonomous Sumo Robot](http://brooksbots.com/Strategy.html)
- [dampedcoast/Sumo-Robot- (estrategias por DIP: Flank, Sit and Wait, Flags…)](https://github.com/dampedcoast/Sumo-Robot-)
- [Tesis Lancelot, UNIVPM (estrategias Tornado, Slow search y Tracking)](https://tesi.univpm.it/handle/20.500.12075/27301)
- [Cytron: Sumo Robot Game Strategies](https://www.cytron.io/tutorial/GameStrategies)
- [Video: Few examples of Mini Sumo robot competition strategies](https://www.youtube.com/watch?v=rthMiqFCiBA)
- [Guía de diseño de palas (sumobot)](https://siegfriedt-survey.umb.edu/pti/sumobot-designs-a-deep-dive-into-the-hidden-details)
