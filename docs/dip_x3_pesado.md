# DIP del X3 pesado para RoboChallenge Rumania

X3: ~0,5 kg (en el simulador), motores de 400 rpm, ruedas atrás, pala delante.
Reglamento ([robochallenge.ro](https://robochallenge.ro/regulation/mini-sumo)):
cada robot se coloca detrás de su línea de salida (10 x 1 cm), en su franja, y
el combate empieza al instante con START (sin espera de 5 s). El robot cabe en
10 x 10 cm, así que en la franja solo caben colocaciones rectas: de frente, de
lado o de espaldas. La de 45° no está en la batería del simulador.

## Tabla de DIP

| DIP1 | DIP2 | DIP3 | Modo | Para qué colocación |
|---|---|---|---|---|
| off | off | — | 1 defensivo | De frente, contra un rival que embiste: lo recibe con la pala |
| **ON** | off | lado del rival si lo ves desplazado | **2 contragolpe** | **De frente (recomendado)** |
| off | ON | off = rival a la derecha / ON = a la izquierda | 3 de lado | De lado al rival |
| ON | ON | off = gira a la derecha / ON = a la izquierda | 4 agresivo | De espaldas al rival |

Para la colocación a 45° (no simulada): el modo 2 gira hacia cualquier sensor
de 45° o lateral que lo vea, así que es la opción más segura.

## Cifras (100 716 combates por batería, solo contra rivales vencibles)

Vencibles = sin el combate sin rival ni el de 500 g ni los factores extremos.

| Colocación (modo) | gana antes | gana ahora | cae antes | cae ahora |
|---|---|---|---|---|
| RU1 de frente (modo 1) | 87,6 % | 88,2 % | 5,9 % | 5,7 % |
| RU2 de frente (modo 2) | 87,3 % | 87,7 % | 6,5 % | **5,4 %** |
| RU3 de lado (modo 3) | 74,6 % | **79,4 %** | 16,5 % | **11,0 %** |
| RU4 de espaldas (modo 4) | 67,1 % | 68,2 % | 22,5 % | 22,8 % |
| Total | 77,9 % | 79,9 % | 13,9 % | 12,0 % |

Caídas propias: 1 antes y 1 ahora; salidas sin rival: 1 y 1 (las dos en RU4,
sin rival y con todos los factores malos a la vez).

## Qué cambió (investigación: sumo pesado y lento)

Un robot pesado y lento gana empujando con la pala y aprovechando su masa, no
corriendo: lo importante es encarar pronto y sin pasarse de giro.

- Modo 1: quieto 1 s en vez de 3 s antes de ir a buscarlo.
- Modo 2: maniobra ciega 50 ms (antes 300) y quieto 1,5 s (antes 3 s).
- Modo 3: giro hacia el rival a 180 (antes 150); a 200 ganaba un poco más pero
  con una caída propia.
- Modo 4: al ver al rival de frente deja de girar y avanza a 150 mientras se
  confirma la detección (con la inercia, el giro a fondo lo sacaba del frontal).
  A 225 ganaba algo más pero con 4 caídas propias.
- Lo que no sirvió: avanzar más rápido en el modo 4 (200-230: cae 36-50 %),
  girar más despacio en el modo 3 (100-120: cae 30-40 %).

Exploración completa de Codex (10 164 y 30 492 combates): [dip_x3_codex.md](dip_x3_codex.md).
