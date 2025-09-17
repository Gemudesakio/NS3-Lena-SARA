LENA-NB + SARA (SCMA-Applied Random Access) — WIP

Este repositorio parte de LENA-NB (extensión NB-IoT de ns-3) y añade una implementación en curso de SARA en el procedimiento de acceso aleatorio (RA) para reproducir la simulación del paper SARA con cambios mínimos fuera de eNB-MAC.

Estado del proyecto

LENA-NB: características existentes (listadas abajo) ya integradas.

SARA: aún no implementado; este README define objetivos, puntos de integración y hoja de ruta. No se debe asumir funcionalidad SARA operativa hasta que los hitos estén marcados como completados.

� LENA-NB (Narrowband)

LENA-NB es una extensión NB-IoT para ns-3 (probada con ns-3.32). Actualmente incluye:

RRC Connection Resume (3GPP Rel. 13)

Cellular IoT Optimization (3GPP Rel. 13)

Early Data Transmission (EDT) (3GPP Rel. 15)

Cross-Subframe Scheduling

Adaptive Modulation and Coding

NB-IoT Energy State Machine

Nota: No se usa por defecto un modelo de error específico NB-IoT. Se emplea una lookup table derivada de simulaciones MATLAB NB-IoT BLER para mapear SNR→config UL/DL con BLER objetivo. En trabajos futuros se integrará un modelo de error NB-IoT nativo.

Publicaciones que usan LENA-NB

P. Jörke, T. Gebauer, and C. Wietfeld, “From LENA to LENA-NB…”, WNS3 ‘22, pp. 73–80.

P. Jörke, T. Gebauer, S. Böcker, and C. Wietfeld, “Scaling Dense NB-IoT…”, VTC2022-Spring.

P. Jörke, D. Ronschka, and C. Wietfeld, “Performance Evaluation of Random Access…”, VTC2023-Spring.

M. Štůsek et al., “Exploiting NB-IoT Network Performance…”, ICUMT 2023.

M.T. Abbas et al., “Evaluating the Impact of Pre-Configured Uplink Resources…”, Sensors 2024.

P. Masek et al., “Quantifying NB-IoT Performance in 5G Use-Cases…”, IEEE IoT Journal.

Si usas este código/resultados, cita el trabajo en la sección Citation. Si falta tu publicación, envía PR.

Uso de LENA-NB (ns-3.32)

Recomendado con ns-3 release 3.32 para evitar incompatibilidades. Tras compilar ns-3, sustituye src/lte por el provisto en este repo. (Actualmente LENA-NB reemplaza LTE; en el futuro se planea coexistencia.)

Ejemplo incluido: src/lte/examples/lena-nb-5G-scenario.cc
Parámetros esperados:

--simTime       # Tiempo a simular (ms)
--randomSeed    # Semilla RNG
--numUeAppA     # Nº UEs app A
--numUeAppB     # Nº UEs app B
--numUeAppC     # Nº UEs app C
--ciot          # Habilitar Cellular IoT Optimization
--edt           # Habilitar Early Data Transmission


Topología por defecto: UEs uniformes en disco de 2500 m; cada UE transmite 1 vez/día (49 bytes por app en el ejemplo; ajustable).

Modelo de propagación adicional:
Este repo puede usar Winner+: https://github.com/tudo-cni/ns3-propagation-winner-plus
 (añádelo para builds exitosos si tu escenario lo requiere).

Nota sobre tiempo real de simulación

La simulación efectiva es 3×simTime (warm-up, ventana principal, cool-down) para estabilizar interferencia/carga y permitir completar transmisiones en curso.

Simulaciones automatizadas

Script runner_example.py para ejecutar múltiples semillas/configuraciones (cola de tareas, reintentos automáticos, num_workers para paralelismo; cuidado con RAM).

*Compilación y logs (desarrollo)
# Recomendado en desarrollo:
./waf -d debug configure --enable-modules=lte --disable-python --disable-werror
./waf build -j"$(nproc)"

# Logs útiles
export NS_LOG="LteEnbMac=level_debug|prefix_time|prefix_node;LteEnbPhy=level_info;LteEnbRrc=level_info"

# Ejecutar ejemplo
./waf --run "lena-nb-5G-scenario"

*Extensión SARA — Implementación en curso (WIP)

Objetivo: reproducir fielmente el flujo del paper SARA en RA NB-IoT:

RAR (Msg2) de colisión “estilo SARA”: un único TC-RNTI por RAPID colisionado, flag de grupo SARA, y anuncio de pools {codebooks (J), DMRS (D)} para Msg3.

Msg3 multi-UE: cada UE del grupo elige aleatoriamente (equiprobable) un par (codebook, DMRS) del pool y transmite simultáneamente.

Detección ciega + separación: éxito si difieren en codebook o DMRS; fallo solo si hay triple coincidencia (preambulo + codebook + DMRS).

Msg4 “broadcast”: un solo mensaje al TC-RNTI común con lista (Random40b → nuevo C-RNTI) para cada UE decodificado.

Alcance y puntos de integración (plan)

eNB-MAC (lte-enb-mac.cc)

Atributo ActivateSara (on/off).

Corrección RAR huérfano en colisión: encolar y agendar el RAR único con flag SARA + {J, D}.

Estado para reconocer TC-RNTIs de grupo y conteo de Msg3 por TTI (telemetría).

Modelado de detección ciega (DMRS) y aceptación de múltiples Msg3 en un mismo grant/TTI (entrega a RLC/RRC).

eNB-PHY (lte-enb-phy.cc)

Atributo SaraMaxUlTbPerGrant (N): preparar el receptor para N TB UL en el mismo TTI por grant (permitir múltiples PhyPduReceived(...) en ese TTI).

eNB-RRC (lte-enb-rrc.cc)

Acumular múltiples Random40b bajo el mismo TC-RNTI.

Construir Msg4 broadcast con pares (Random40b, nuevo C-RNTI).

UEs: inicialmente sin cambios funcionales; si el RAR trae flag SARA, se modelará (desde eNB) la elección aleatoria uniforme de (codebook, DMRS) para Msg3.

Parámetros SARA previstos
Parámetro	Ámbito	Descripción
ActivateSara (bool)	eNB-MAC	Activa/desactiva toda la lógica SARA.
SaraCodebookPoolSize (J)	eNB-MAC	Tamaño del pool de codebooks SCMA por RAPID.
SaraDmrsPoolSize (D)	eNB-MAC	Tamaño del pool de secuencias DMRS por RAPID.
SaraPreambleDetectionProb (0..1)	eNB-MAC	Probabilidad de detección de preámbulo (modelado).
SaraMaxUlTbPerGrant (N)	eNB-PHY	Nº máx. de PDUs UL esperados en el mismo TTI/grant.

Activación (cuando esté implementado):

Config::SetDefault("ns3::LteEnbMac::ActivateSara", BooleanValue(true));
Config::SetDefault("ns3::LteEnbMac::SaraCodebookPoolSize", UintegerValue(6)); // J
Config::SetDefault("ns3::LteEnbMac::SaraDmrsPoolSize",     UintegerValue(8)); // D
Config::SetDefault("ns3::LteEnbMac::SaraPreambleDetectionProb", DoubleValue(0.9));
Config::SetDefault("ns3::LteEnbPhy::SaraMaxUlTbPerGrant",  UintegerValue(2));

Hoja de ruta (milestones)

M0 — Andamiaje: atributos/flags (sin cambiar comportamiento por defecto).

M1 — RAR colisión operativo: encolar/agendar RAR único con flag SARA + {J, D}.

M2 — PHY multi-TB: SaraMaxUlTbPerGrant y telemetría UL por TTI.

M3 — Separación SARA (MAC): detección ciega por DMRS y aceptación de múltiples Msg3.

M4 — Msg4 broadcast (RRC): construcción y entrega al TC-RNTI común.

M5 — Evaluación: curvas de éxito RA, retrasos, colisiones DMRS vs D, comparación LTE vs SARA.

Validación (plan)

Prob. de éxito RA por intento vs carga.

Retransmisiones y delay medio.

Tasa de colisión de DMRS ~ función de D.

Comparativa LTE vs SARA manteniendo constantes el resto de capas.

* Estructura (sugerida)
src/
  lte/
    model/
      lte-enb-mac.cc      # (LENA-NB) + ganchos SARA (WIP)
      lte-enb-phy.cc      # (LENA-NB) + receptor multi-TB (WIP)
      lte-enb-rrc.cc      # (LENA-NB) + Msg4 broadcast (WIP)
    examples/
      lena-nb-5G-scenario.cc
      sara-nbiot-sim.cc   # (WIP) escenario mínimo para SARA
docs/
  SARA-notes.md           # supuestos, fórmulas y trazas
  results/                # scripts/figuras de evaluación

* Acknowledgement

Trabajo realizado en el marco de PuLS (BMVI 03EMF0203B), 5hine y Competence Center 5G.NRW (MWIDE 005-2108-0073 / 005-01903-0047), y con soporte del SFB 876 “Providing Information by Resource-Constrained Analysis”, proyecto A4.

# References

[1] MathWorks. 2021. NB-IoT NPDSCH Block Error Rate Simulation. Retrieved December 11, 2021 from https://www.mathworks.com/help/lte/ug/nb-iot-npdsch-block-error-rate-simulation.html
[2] MathWorks. 2021. NB-IoT NPUSCH Block Error Rate Simulation. Retrieved December 11, 2021 from https://www.mathworks.com/help/lte/ug/nb-iot-npusch-block-error-rate-simulation.html
