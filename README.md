# LENA-NB RA Lab (legacy / SARA / NEW)

Guia operativa exacta del estado actual del repositorio.

## 1) Alcance real del repositorio

Este repo Git tiene su raiz en `ns-3.32/src/lte`.
No incluye el resto de `ns-3.32`.

Implicacion practica:

- Para ejecutar simulaciones, debes integrarlo dentro de un arbol `ns-3.32` funcional.
- Si solo clonas este repo en una carpeta vacia, no compila por si solo.

Fuente en codigo:

- repo raiz detectada en `src/lte/.git`
- modulo declarado en [wscript](wscript)

## 2) Dependencia Winner+

El ejemplo principal `rc1_sn2_example.cc` usa Winner+ de forma obligatoria.

Pruebas en codigo:

- include directo: [examples/rc1_sn2_example.cc](examples/rc1_sn2_example.cc)
- set de pathloss fijo a Winner+: [examples/rc1_sn2_example.cc](examples/rc1_sn2_example.cc)

### 2.1 Esta dependencia viene dentro de este repo?

No. Este repo `src/lte` no contiene `winner-plus-propagation-loss-model.*`.

### 2.2 Donde debe existir Winner+

Debe existir en el arbol `ns-3.32/src/propagation`:

- `src/propagation/model/winner-plus-propagation-loss-model.h`
- `src/propagation/model/winner-plus-propagation-loss-model.cc`
- entrada en `src/propagation/wscript`

En el entorno actual SI existe:

- `src/propagation/model/winner-plus-propagation-loss-model.*`
- `src/propagation/wscript` incluye esos archivos.

### 2.3 Como verificar rapidamente

Desde `ns-3.32`:

```bash
test -f src/propagation/model/winner-plus-propagation-loss-model.h && echo "winner header: OK"
test -f src/propagation/model/winner-plus-propagation-loss-model.cc && echo "winner source: OK"
rg -n "winner-plus-propagation-loss-model" src/propagation/wscript
```

Si falta, integra el modulo Winner+ en `src/propagation` antes de compilar.

## 3) Clonado e integracion

### 3.1 Paso 1: preparar base ns-3.32

Debes tener un arbol `ns-3.32` operativo.

### 3.2 Paso 2: reemplazar `src/lte`

Desde `ns-3.32/src`:

```bash
rm -rf lte
git clone <TU_REPO_GITHUB> lte
```

### 3.3 Paso 3: validar Winner+

Ejecuta las validaciones de la seccion 2.3.

### 3.4 Paso 4: compilar

Desde `ns-3.32`:

```bash
./waf -d optimized configure --enable-examples --enable-modules=lte --disable-python
./waf build -j"$(nproc)"
```

Este flujo fue validado en este entorno.

### 3.5 Paso 5: smoke test

```bash
./waf --run "rc1_sn2_example --PrintHelp"
./waf --run "rc1_sn2_example --stopMs=6000 --populationPreset=avg --trafficProfile=10m --maxArrivals=20 --raMode=new --report=false"
```

## 4) Escenario principal y comandos

Ejemplo principal:

- [examples/rc1_sn2_example.cc](examples/rc1_sn2_example.cc)

Comando base:

```bash
./waf --run "rc1_sn2_example"
```

Comando con parametros:

```bash
./waf --run "rc1_sn2_example --raMode=sara --stopMs=25000 --populationPreset=high --trafficProfile=2h --maxArrivals=500 --report=true"
```

## 5) Parametros CLI de `rc1_sn2_example`

Fuente de verdad:

- definicion y defaults: [examples/rc1_sn2_example.cc](examples/rc1_sn2_example.cc)
- ayuda runtime: `--PrintHelp`

| Parametro | Default | Para que sirve | Valores / notas |
|---|---:|---|---|
| `--numUe` | `10` | Valor inicial de UEs creados | En este ejemplo luego se sobrescribe por llegadas de `phase2` |
| `--cellRadius` | `9000` | Radio de celda (m) para distribuir UEs en disco | `double > 0` recomendado |
| `--stopMs` | `2000` | Tiempo total de simulacion (ms) | Debe ser `> phase2StartOffsetMs` |
| `--raMode` | `sara` | Esquema RA | `legacy`, `sara`, `new` |
| `--rngSeed` | `12345` | Semilla global RNG | Entero |
| `--rngRun` | `1` | Run global RNG | Entero |
| `--enableVerboseLogs` | `false` | Activa logs INFO de RRC/MAC | `true/false` |
| `--report` | `true` | Activa CSVs de `SaraReport` | `true/false` |
| `--reportPrefix` | `""` | Prefijo de archivos de reporte | Si vacio usa `<modo>_report` |
| `--reportDir` | `../reportes` | Carpeta base para reportes | Relativa al cwd de ejecucion |
| `--reportRunId` | `""` | Nombre subcarpeta de corrida | Si vacio se autogenera con fecha |
| `--collisionTpr` | `0.975` | TPR detector de colisiones (new/sara) | Rango `[0,1]` |
| `--collisionFpr` | `0.001` | FPR detector de colisiones (new/sara) | Rango `[0,1]` |
| `--nbRaBackoffEnabled` | `true` | Habilita backoff en reintentos Msg1 | `true/false` |
| `--nbRaBackoffMinMs` | `0` | Backoff minimo Msg1 (ms) | Debe ser `<= nbRaBackoffMaxMs` |
| `--nbRaBackoffMaxMs` | `256` | Backoff maximo Msg1 (ms) | Debe ser `>= nbRaBackoffMinMs` |
| `--connReqTimeoutMs` | `50000` | Timeout eNB para esperar `RRCConnectionRequest` | maximo 50000 ms por checker del atributo |
| `--connSetupTimeoutMs` | `240000` | Timeout eNB para esperar `RRCConnectionSetupComplete` | sin tope en CLI del ejemplo |
| `--populationPreset` | `avg` | Poblacion virtual del trafico phase2 | `avg`, `high`, `ultra`, `custom` |
| `--trafficProfile` | `2h` | Perfil temporal por UE | `2h`, `10m`, `custom` |
| `--customPopulation` | `0` | Poblacion si `populationPreset=custom` | Debe ser `>0` en custom |
| `--customPeriodSeconds` | `0` | Periodo medio (s) si `trafficProfile=custom` | Debe ser `>0` en custom |
| `--maxArrivals` | `0` | Tope de llegadas en phase2 | `0` significa ilimitado por horizonte |
| `--phase2StartOffsetMs` | `3000` | Offset antes de empezar Attach masivo | Debe ser `< stopMs` |
| `--enbTxPowerDbm` | `43` | Potencia TX DL del eNB (dBm) | afecta RSRP y CE |

## 6) Modelo de trafico phase2 (Poisson agregado)

Fuente:

- [helper/nb-iot-traffic-helper.cc](helper/nb-iot-traffic-helper.cc)
- uso en [examples/rc1_sn2_example.cc](examples/rc1_sn2_example.cc)

Logica exacta:

1. Se resuelve poblacion virtual:
- `avg = 52500`
- `high = 200000`
- `ultra = 500000`
- `custom = customPopulation`

2. Se resuelve periodo por UE:
- `2h = 7200 s`
- `10m = 600 s`
- `custom = customPeriodSeconds`

3. Tasa agregada:
- `lambda = population / periodSeconds`

4. Llegadas:
- inter-arrivals ~ Exponencial con `mean = 1/lambda`
- se generan eventos hasta `horizonSeconds`
- `horizonSeconds = (stopMs - phase2StartOffsetMs)/1000`
- si `maxArrivals > 0`, aplica tope duro de llegadas

5. Numero final de UEs creados/attach:
- `numUe = arrivalTimesSeconds.size()`
- por eso `--numUe` no manda cuando phase2 esta activo (que es el caso actual)

## 7) Configuracion fija del ejemplo (no expuesta por CLI)

Fuente:

- [examples/rc1_sn2_example.cc](examples/rc1_sn2_example.cc)

Valores fijos hoy:

- Pathloss: `ns3::WinnerPlusPropagationLossModel`
- Winner+ Environment: `UMaEnvironment`
- Winner+ LOS: `false` (NLOS)
- Winner+ `HeightBasestation`: `50.0`
- posicion eNB: `(0,0,30)`
- UEs distribuidos uniforme en disco de radio `cellRadius`
- altura UE forzada a `z=1.5`
- `LteEnbMac::SaraMaxGroupSize = 2`
- `LteEnbMac::ScmaMaxGroupSize = 2`
- `LteUeRrc::T300 = 60000 ms`
- `LteSpectrumPhy::ExtendedExpectedTbTracking = true`

Dependiente de `raMode`:

- si `new`: `ToaNumBins=2048`, `ToaToleranceBins=0`
- si `legacy` o `sara`: `ToaNumBins=64`, `ToaToleranceBins=1`

## 8) Configuracion CE/NPRACH que usa hoy el simulador

Fuente:

- [model/lte-enb-rrc.cc](model/lte-enb-rrc.cc)

Umbrales CE por RSRP:

- `CE0` si `RSRP > -115.5 dBm`
- `CE1` si `-127.5 < RSRP <= -115.5 dBm`
- `CE2` si `RSRP <= -127.5 dBm`

Parametros RA por CE (SIB2 generado en eNB):

- `nprachPeriodicity`: CE0 `320 ms`, CE1 `640 ms`, CE2 `2560 ms`
- `nprachStartTime`: `256 ms` para CE0/CE1/CE2
- `numRepetitionsPerPreambleAttempt`: CE0 `1`, CE1 `8`, CE2 `32`
- `npdcchNumRepetitionsRA`: CE0 `8`, CE1 `64`, CE2 `512`
- `npdcchStartSfCssRa`: CE0 `2`, CE1 `1.5`, CE2 `4`
- `npdcchOffsetRa`: `0` para CE0/CE1/CE2
- `maxNumPreambleAttemptCE`: `10` para CE0/CE1/CE2
- `RaResponseWindowSize`: CE0 `10`, CE1 `8`, CE2 `8`
- `macContentionResolutionTimer`: `32` para CE0/CE1/CE2

Layout NPRACH por defecto (Msg1):

- CE0: `numSubcarriers=12`, `offset=36`
- CE1: `numSubcarriers=12`, `offset=24`
- CE2: `numSubcarriers=24`, `offset=0`
- `NprachStrictNoOverlap = true`

## 9) Reportes CSV

Fuente:

- [model/sara-report.cc](model/sara-report.cc)

Si `--report=true`, se generan CSV con prefijo `<prefix>`:

- `<prefix>_msg1.csv`
- `<prefix>_msg2.csv`
- `<prefix>_msg3_ue.csv`
- `<prefix>_msg3_enb.csv`
- `<prefix>_msg3_sep.csv`
- `<prefix>_msg3_rrc.csv`
- `<prefix>_msg3_fwd.csv`
- `<prefix>_ctx_enb.csv`
- `<prefix>_msg4_enb.csv`
- `<prefix>_msg4_ue.csv`
- `<prefix>_msg4_air_enb.csv`
- `<prefix>_msg4_rx_ue.csv`
- `<prefix>_msg4_phy_ue.csv`
- `<prefix>_dci_nb_ue.csv`
- `<prefix>_expected_tb_ue.csv`
- `<prefix>_msg5_tx.csv`
- `<prefix>_msg5_enb.csv`
- `<prefix>_transition.csv`
- `<prefix>_summary.csv`

## 10) Errores de configuracion comunes

- `raMode invalido`: usa solo `legacy|sara|new`.
- `phase2StartOffsetMs debe ser menor que stopMs`.
- `nbRaBackoffMaxMs < nbRaBackoffMinMs`.
- `populationPreset=custom` sin `customPopulation>0`.
- `trafficProfile=custom` sin `customPeriodSeconds>0`.
- falta Winner+ en `src/propagation`.

## 11) Comandos utiles de referencia

Imprimir ayuda completa:

```bash
./waf --run "rc1_sn2_example --PrintHelp"
```

Legacy:

```bash
./waf --run "rc1_sn2_example --raMode=legacy --stopMs=25000 --populationPreset=avg --trafficProfile=2h --maxArrivals=200"
```

SARA:

```bash
./waf --run "rc1_sn2_example --raMode=sara --stopMs=25000 --populationPreset=high --trafficProfile=10m --maxArrivals=500"
```

NEW:

```bash
./waf --run "rc1_sn2_example --raMode=new --stopMs=25000 --populationPreset=high --trafficProfile=2h --maxArrivals=500"
```
