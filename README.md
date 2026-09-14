# Kick Bass Space -- Etapa 1: Análisis

Primera etapa del plan: detectar la energía de kick y bass en el tiempo, y
verla en pantalla, **sin procesar el audio todavía**. El objetivo es validar
que la detección funciona antes de construir el motor de decisión (Etapa 2)
y el DSP real (Etapa 3).

## Qué hace esta versión

- Tiene una entrada de **Sidechain** (para el bajo) además de la entrada
  principal (para el kick).
- Sigue la envolvente (energía en el tiempo) de ambas señales con un
  detector de ataque/release clásico.
- Dibuja las dos curvas superpuestas en pantalla, en tiempo real.
- Avisa visualmente si no detecta señal en el sidechain (para que sepas si
  falta enrutar el bajo).
- El audio de salida es **idéntico** al de entrada -- todavía no hay
  procesamiento real, solo análisis.

## Cómo configurar el Sidechain en Studio One

Esto es importante y a veces confunde:

1. Inserta **Kick Bass Space** como plugin en el canal del **kick**.
2. En Studio One, en la cabecera del inserto del plugin (o en el panel de
   sidechain del canal), busca la opción de **Sidechain Input** / entrada
   externa.
3. Selecciona como fuente de sidechain la salida de la pista del **bajo**
   (puede requerir crear un bus auxiliar que reciba el bajo, dependiendo de
   tu versión de Studio One).
4. Reproduce ambas pistas a la vez -- deberías ver la curva violeta (Bass)
   reaccionar cuando suena el bajo, y la curva turquesa (Kick) reaccionar
   con cada golpe.
5. Si la curva violeta se queda plana en 0 y ves el aviso amarillo en
   pantalla, el sidechain no está bien enrutado -- revisa el paso 3.

## Qué validar antes de pasar a la Etapa 2

- [ ] La curva del Kick sube claramente con cada golpe y baja entre golpes.
- [ ] La curva del Bass sube y baja seg��n la dinámica real del bajo.
- [ ] Ambas curvas se ven temporalmente alineadas con lo que escuchas
      (sin retrasos raros).
- [ ] Ajustando Attack/Release, las curvas se sienten más "nerviosas" o
      más "suavizadas" según corresponda.

Si todo esto se cumple, la base de análisis es sólida y pasamos a la
Etapa 2: detectar matemáticamente el solapamiento entre ambas curvas.

## Compilar

Mismo proceso que los otros proyectos:

```bash
cd KickBassSpace
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

O sube el proyecto a GitHub (ya incluye `.github/workflows/build.yml`
configurado) y descarga el `.vst3` compilado desde la pestaña Actions.

## Instalación en Studio One

Copia `Kick Bass Space.vst3` a `C:\Program Files\Common Files\VST3\`,
reinicia Studio One (o Options → Locations → VST Plug-Ins → Reset), y
búscalo en la categoría **Dynamics** del navegador de efectos.

## Nota importante

Este es un proyecto **nuevo e independiente** del EQ de kick -- no modifica
nada de `HybridKickEQ`. La idea es probar y validar el análisis Kick/Bass
por separado, con una base más simple (sin EQ de por medio), antes de
integrar todo en un producto final.

Como con los proyectos anteriores, escribí este código sin poder
compilarlo/probarlo con audio real en este entorno. La lógica de detección
de envolvente es simple y muy estándar (attack/release de un polo), pero si
algo no se comporta como esperas (curvas que no reaccionan, valores
extraños), cuéntamelo con detalle para ajustarlo.
