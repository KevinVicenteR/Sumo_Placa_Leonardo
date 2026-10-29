# Borde del X3 pesado (0,5 kg, 400 rpm) — barrido para Rumania

Batería `bateria.py --rumania`, un parámetro por vez, 20 328 combates por valor.
"Vencibles" = sin el combate sin rival ni el de 500 g ni los factores extremos
(`meta_vencibles.py`). Propias = caídas sin que lo empujen; solo = caídas sin rival.

| Variante | gana | cae | propias | solo |
|---|---|---|---|---|
| actual (referencia) | 78,9 % | 12,9 % | 0 | 0 |
| DesaceleracionFreno 2,5 / 3,0 / 5,0 | 78,9 % | 12,9 % | 0 | 0 |
| MargenBorde 0,07 / 0,11 / 0,13 | 78,9 % | 12,9 % | 0 | 0 |
| IncertPosicionConfiable 0,06 | 78,9 % | 12,8 % | 0 | 0 |
| IncertPosicionConfiable 0,11 | 78,9 % | 12,9 % | 0 | 0 |
| RetrocesoExtraAVelocidadMaxima 200 | 79,0 % | 12,7 % | **14** | 2 |
| RetrocesoExtraAVelocidadMaxima 450 | 78,8 % | 13,0 % | 2 | 0 |
| VelocidadCercaBorde 80 | 78,9 % | 12,8 % | 0 | 0 |
| VelocidadCercaBorde 125 | 78,9 % | 12,9 % | 0 | 0 |
| FrenoActivo = true (contramando en la evasión) | 78,9 % | 12,7 % | 0 | 0 |

Conclusión: se quedan los valores actuales.

- Con colocación libre el estimador de borde casi nunca llega a ser fiable,
  así que su margen y su deceleración no actúan: el borde lo resuelve la
  evasión (retroceso proporcional a la velocidad + giro).
- RetrocesoExtraAVelocidadMaxima = 300 es el punto bueno: con menos retroceso,
  la masa de 0,5 kg sigue deslizando y se sale solo.
- FrenoActivo = true baja 0,2 pp las caídas, que es ruido a 20 000 combates, y
  en el X3 real se apagó porque retrocedía tras los choques: se deja apagado.
- Con 100 716 combates la base da 77,9 % / 13,9 % con 1 caída propia y 1 salida
  sin rival (las demás caídas son empujones del rival, sobre todo los rápidos y
  el de 500 g; sin sensores de piso traseros no bajan a 0).
