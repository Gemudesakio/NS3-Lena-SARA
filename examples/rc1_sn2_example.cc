#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/point-to-point-epc-helper.h"
#include "ns3/internet-module.h"
#include "ns3/nb-iot-traffic-helper.h"
#include "ns3/sara-report.h"
#include "ns3/system-path.h"
#include "ns3/winner-plus-propagation-loss-model.h"

#include <ctime>
#include <list>
#include <vector>

using namespace ns3;

// envolturas para usar Schedule sin ambigüedad
static void DoAttachSusp (Ptr<LteHelper> lte, Ptr<NetDevice> ue, Ptr<NetDevice> enb)
{
  lte->AttachSuspendedNb (ue, enb);
}
static void DoAttach (Ptr<LteHelper> lte, Ptr<NetDevice> ue, Ptr<NetDevice> enb)
{
  lte->Attach (ue, enb);
}
int main (int argc, char *argv[])
{
  // ---------- Parámetros ----------
  uint32_t numUe      = 10;     // muchos UEs para provocar colisión
  double   cellRadius = 9000.0;
  uint32_t stopMs     = 2000;
  bool     useNbSuspend = false; // usa AttachSuspendedNb (también sirve Attach normal)
  std::string populationPreset = "avg"; // avg|high|ultra|custom
  std::string trafficProfile = "2h"; // 2h|10m|custom
  uint32_t customPopulation = 0;
  double customPeriodSeconds = 0.0;
  uint32_t maxArrivals = 0; // 0 = ilimitado
  uint32_t phase2StartOffsetMs = 3000; // warm-up para garantizar SIB2 antes de Attach masivo
  double   enbTxPowerDbm = 43.0;
  std::string raMode = "sara";   // legacy|sara|new
  uint32_t rngSeed = 12345;
  uint32_t rngRun = 1;
  bool enableVerboseLogs = false;
  bool reportEnabled = true;
  std::string reportPrefix = ""; // si está vacío, usa "<modo>_report"
  std::string reportDir = "../reportes"; // ruta base por defecto (ns-allinone-3.32/reportes)
  std::string reportRunId = "";
  double collisionTpr = 0.975;
  double collisionFpr = 0.001;
  bool nbRaBackoffEnabled = true;
  uint32_t nbRaBackoffMinMs = 0;
  uint32_t nbRaBackoffMaxMs = 256;
  uint32_t connReqTimeoutMs = 50000;
  uint32_t connSetupTimeoutMs = 240000;

  CommandLine cmd (__FILE__);
  cmd.AddValue ("numUe", "Numero de UEs en la rafaga de acceso", numUe);
  cmd.AddValue ("cellRadius", "Radio de la celda (m) para distribuir UEs", cellRadius);
  cmd.AddValue ("stopMs", "Tiempo total de simulacion en ms", stopMs);
  cmd.AddValue ("raMode", "Modo de acceso aleatorio: legacy|sara|new", raMode);
  cmd.AddValue ("rngSeed", "Semilla de aleatoriedad", rngSeed);
  cmd.AddValue ("rngRun", "RngRun para reproducibilidad/variacion", rngRun);
  cmd.AddValue ("enableVerboseLogs", "Activa logs INFO detallados de RRC/MAC para debug", enableVerboseLogs);
  cmd.AddValue ("report", "Habilitar reporte CSV", reportEnabled);
  cmd.AddValue ("reportPrefix", "Prefijo de archivos de reporte", reportPrefix);
  cmd.AddValue ("reportDir", "Directorio base para reportes (se crea subcarpeta por corrida)", reportDir);
  cmd.AddValue ("reportRunId", "Nombre de subcarpeta de corrida (si vacío se autogenera)", reportRunId);
  cmd.AddValue ("collisionTpr", "TPR del detector de colisiones (new/sara)", collisionTpr);
  cmd.AddValue ("collisionFpr", "FPR del detector de colisiones (new/sara)", collisionFpr);
  cmd.AddValue ("nbRaBackoffEnabled", "Activa backoff aleatorio antes del reintento de Msg1 en NB-IoT", nbRaBackoffEnabled);
  cmd.AddValue ("nbRaBackoffMinMs", "Backoff minimo (ms) para reintentos de Msg1 NB-IoT", nbRaBackoffMinMs);
  cmd.AddValue ("nbRaBackoffMaxMs", "Backoff maximo (ms) para reintentos de Msg1 NB-IoT", nbRaBackoffMaxMs);
  cmd.AddValue ("connReqTimeoutMs", "Timeout eNB para esperar RRCConnectionRequest (ms, maximo 50000)", connReqTimeoutMs);
  cmd.AddValue ("connSetupTimeoutMs", "Timeout eNB para esperar RRCConnectionSetupComplete (ms)", connSetupTimeoutMs);
  cmd.AddValue ("populationPreset", "Poblacion virtual: avg|high|ultra|custom", populationPreset);
  cmd.AddValue ("trafficProfile", "Perfil por UE: 2h|10m|custom", trafficProfile);
  cmd.AddValue ("customPopulation", "Poblacion virtual cuando populationPreset=custom", customPopulation);
  cmd.AddValue ("customPeriodSeconds", "Periodo medio (s) cuando trafficProfile=custom", customPeriodSeconds);
  cmd.AddValue ("maxArrivals", "Limite de llegadas generadas en phase2 (0 = ilimitado)", maxArrivals);
  cmd.AddValue ("phase2StartOffsetMs", "Offset de inicio (ms) para Attach en phase2", phase2StartOffsetMs);
  cmd.AddValue ("enbTxPowerDbm", "Potencia TX DL del eNB (dBm) para forzar CE", enbTxPowerDbm);
  cmd.Parse (argc, argv);

  // ---------- Reproducibilidad ----------
  GlobalValue::Bind ("RngSeed", UintegerValue (rngSeed));
  GlobalValue::Bind ("RngRun",  UintegerValue (rngRun));

  const bool modeSara = (raMode == "sara");
  const bool modeNew = (raMode == "new");
  if (nbRaBackoffMaxMs < nbRaBackoffMinMs)
    {
      NS_FATAL_ERROR ("nbRaBackoffMaxMs (" << nbRaBackoffMaxMs
                      << ") no puede ser menor que nbRaBackoffMinMs (" << nbRaBackoffMinMs << ")");
    }
  if (!(raMode == "legacy" || modeSara || modeNew))
    {
      NS_FATAL_ERROR ("raMode invalido='" << raMode
                      << "'. Valores permitidos: legacy|sara|new");
    }
  if (phase2StartOffsetMs >= stopMs)
    {
      NS_FATAL_ERROR ("phase2StartOffsetMs debe ser menor que stopMs");
    }
  const std::string modeName = modeNew ? "new" : (modeSara ? "sara" : "legacy");
  if (reportPrefix.empty ())
    {
      reportPrefix = modeName + "_report";
    }

  std::vector<double> attachTimesMs;
  {
    NbIotTrafficHelper::Config trafficCfg;
    trafficCfg.populationPreset = populationPreset;
    trafficCfg.trafficProfile = trafficProfile;
    trafficCfg.customPopulation = customPopulation;
    trafficCfg.customPeriodSeconds = customPeriodSeconds;
    trafficCfg.maxArrivals = maxArrivals;
    NbIotTrafficHelper::Result traffic =
        NbIotTrafficHelper::BuildSchedule (
            trafficCfg, static_cast<double> (stopMs - phase2StartOffsetMs) / 1000.0);
    attachTimesMs.reserve (traffic.arrivalTimesSeconds.size ());
    for (std::vector<double>::const_iterator it = traffic.arrivalTimesSeconds.begin ();
         it != traffic.arrivalTimesSeconds.end (); ++it)
      {
        attachTimesMs.push_back (static_cast<double> (phase2StartOffsetMs) + (*it) * 1000.0);
      }
    numUe = static_cast<uint32_t> (attachTimesMs.size ());
    NS_LOG_UNCOND ("[TRAFFIC][PHASE2] preset=" << populationPreset
                   << " profile=" << trafficProfile
                   << " population=" << traffic.population
                   << " period_s=" << traffic.periodSeconds
                   << " lambda_per_s=" << traffic.lambdaPerSecond
                   << " horizon_s=" << (static_cast<double> (stopMs - phase2StartOffsetMs) / 1000.0)
                   << " start_offset_ms=" << phase2StartOffsetMs
                   << " arrivals=" << numUe);
  }

  // ---------- Configuración por modo ----------
  Config::SetDefault ("ns3::LteEnbMac::NewSchemaActivated", BooleanValue (modeNew));
  Config::SetDefault ("ns3::LteUeMac::NewSchemaActivated", BooleanValue (modeNew));
  Config::SetDefault ("ns3::LteUePhy::NewSchemaActivated", BooleanValue (modeNew));
  Config::SetDefault ("ns3::LteEnbMac::SaraActivated", BooleanValue (modeSara));
  Config::SetDefault ("ns3::LteEnbMac::SaraMaxGroupSize", UintegerValue (2));

  // detector de colisión común para SARA y new
  Config::SetDefault ("ns3::LteEnbMac::CollisionDetectorTpr", DoubleValue (collisionTpr));
  Config::SetDefault ("ns3::LteEnbMac::CollisionDetectorFpr", DoubleValue (collisionFpr));
  // Alias heredados (misma variable interna): se fijan igual para evitar sobrescritura por orden de atributos.
  Config::SetDefault ("ns3::LteEnbMac::ScmaTpr", DoubleValue (collisionTpr));
  Config::SetDefault ("ns3::LteEnbMac::ScmaFpr", DoubleValue (collisionFpr));
  Config::SetDefault ("ns3::LteEnbMac::SaraTpr", DoubleValue (collisionTpr));
  Config::SetDefault ("ns3::LteEnbMac::SaraFpr", DoubleValue (collisionFpr));
  Config::SetDefault ("ns3::LteEnbMac::ScmaMaxGroupSize",   UintegerValue (2));
  // Keep UE/eNB ToA quantization aligned to avoid asymmetry bias.
  const uint16_t toaNumBins = modeNew ? 2048 : 64;
  const uint16_t toaToleranceBins = modeNew ? 0 : 1;
  Config::SetDefault ("ns3::LteUeMac::ToaNumBins",          UintegerValue (toaNumBins));
  Config::SetDefault ("ns3::LteUeMac::ToaToleranceBins",    UintegerValue (toaToleranceBins));
  Config::SetDefault ("ns3::LteEnbMac::ToaNumBins",         UintegerValue (toaNumBins));
  Config::SetDefault ("ns3::LteEnbMac::ToaToleranceBins",   UintegerValue (toaToleranceBins));
  Config::SetDefault ("ns3::LteUeMac::NbRaBackoffEnabled",  BooleanValue (nbRaBackoffEnabled));
  Config::SetDefault ("ns3::LteUeMac::NbRaBackoffMinMs",    UintegerValue (nbRaBackoffMinMs));
  Config::SetDefault ("ns3::LteUeMac::NbRaBackoffMaxMs",    UintegerValue (nbRaBackoffMaxMs));
  // Hardening for long high-load runs:
  // keep UE T300 within standard max (<= 60s) and let eNB request-timeout be slightly lower.
  Config::SetDefault ("ns3::LteUeRrc::T300", TimeValue (MilliSeconds (60000)));
  Config::SetDefault ("ns3::LteEnbRrc::ConnectionRequestTimeoutDuration", TimeValue (MilliSeconds (connReqTimeoutMs)));
  Config::SetDefault ("ns3::LteEnbRrc::ConnectionSetupTimeoutDuration", TimeValue (MilliSeconds (connSetupTimeoutMs)));
  // NPRACH Msg1 layout now defaults at simulator level (LteEnbRrc TypeId).
  Config::SetDefault ("ns3::LteSpectrumPhy::ExtendedExpectedTbTracking", BooleanValue (true));
  Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (enbTxPowerDbm));

  // ---------- Logs útiles ----------
  if (enableVerboseLogs)
    {
      LogComponentEnable ("LteEnbMac", LOG_LEVEL_INFO);
      LogComponentEnable ("LteUeMac",  LOG_LEVEL_INFO);
      LogComponentEnable ("LteEnbRrc", LOG_LEVEL_INFO);
      LogComponentEnable ("LteUeRrc",  LOG_LEVEL_INFO);
    }
  // si quieres más detalle:
  // LogComponentEnable ("LteEnbRrc", LOG_LEVEL_INFO);
  // LogComponentEnable ("LteUeRrc",  LOG_LEVEL_INFO);
  // LogComponentEnable ("LtePhy",    LOG_LEVEL_INFO);

  // ---------- Helper LTE + EPC mínimo (requisito de este fork para attach) ----------
  Ptr<LteHelper> lte = CreateObject<LteHelper> ();
  lte->SetAttribute ("PathlossModel", StringValue ("ns3::WinnerPlusPropagationLossModel"));
  lte->SetPathlossModelAttribute ("Environment", EnumValue (UMaEnvironment));
  lte->SetPathlossModelAttribute ("LineOfSight", BooleanValue (false));
  lte->SetPathlossModelAttribute ("HeightBasestation", DoubleValue (50.0));
  Ptr<PointToPointEpcHelper> epc = CreateObject<PointToPointEpcHelper> ();
  lte->SetEpcHelper (epc);

  // ---------- Nodos ----------
  NodeContainer enbs; enbs.Create (1);
  NodeContainer ues;  ues.Create  (numUe);

  // ---------- Posiciones ----------
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  Ptr<ListPositionAllocator> posEnb = CreateObject<ListPositionAllocator> ();
  posEnb->Add (Vector (0.0, 0.0, 30.0));
  mobility.SetPositionAllocator (posEnb);
  mobility.Install (enbs);

  Ptr<UniformDiscPositionAllocator> posUe = CreateObject<UniformDiscPositionAllocator> ();
  posUe->SetX (0.0); posUe->SetY (0.0); posUe->SetRho (cellRadius);
  mobility.SetPositionAllocator (posUe);
  mobility.Install (ues);
  // Force UE height to 1.5 m (outdoor) to avoid the extra indoor pathloss applied at z=0.
  for (uint32_t i = 0; i < ues.GetN (); ++i)
    {
      Ptr<MobilityModel> mm = ues.Get (i)->GetObject<MobilityModel> ();
      Vector p = mm->GetPosition ();
      mm->SetPosition (Vector (p.x, p.y, 1.5));
    }

  // ---------- Dispositivos radio ----------
  NetDeviceContainer enbDevs = lte->InstallEnbDevice (enbs);
  NetDeviceContainer ueDevs  = lte->InstallUeDevice  (ues);

  // ---------- Pila IP mínima en UEs (EPC lo pide para activar bearers) ----------
  InternetStackHelper internet;
  internet.Install (ues);
  // No necesitamos remote host para este smoke; solo RA/RAR.

  // ---------- Adjuntar UEs ----------
  for (uint32_t i = 0; i < ueDevs.GetN (); ++i)
    {
      Time t = MilliSeconds (attachTimesMs[i]);
      if (useNbSuspend)
        {
          Simulator::Schedule (t, &DoAttachSusp, lte, ueDevs.Get (i), enbDevs.Get (0));
        }
      else
        {
          Simulator::Schedule (t, &DoAttach, lte, ueDevs.Get (i), enbDevs.Get (0));
        }
    }

  // ---------- Simulación corta: suficiente para NPRACH + RAR ----------
  Simulator::Stop (MilliSeconds (stopMs));
  if (reportEnabled)
    {
      std::string finalPrefix = reportPrefix;
      if (!reportDir.empty () || !reportRunId.empty ())
        {
          const std::string baseReportDir = reportDir.empty () ? "." : reportDir;
          if (reportRunId.empty ())
            {
              std::time_t now = std::time (0);
              std::tm *lt = std::localtime (&now);
              char buf[48];
              std::strftime (buf, sizeof (buf), "%Y%m%d_%H%M%S", lt);
              reportRunId = modeName + "_run_" + std::string (buf);
            }
          std::string runDir = ns3::SystemPath::Append (baseReportDir, reportRunId);
          ns3::SystemPath::MakeDirectories (runDir);
          std::string baseName = reportPrefix;
          std::list<std::string> parts = ns3::SystemPath::Split (reportPrefix);
          if (!parts.empty ())
            {
              baseName = parts.back ();
            }
          finalPrefix = ns3::SystemPath::Append (runDir, baseName);
        }
      SaraReport::Enable (finalPrefix);
    }
  Simulator::Run ();
  if (reportEnabled)
    {
      SaraReport::Finalize ();
    }
  Simulator::Destroy ();
  return 0;
}
