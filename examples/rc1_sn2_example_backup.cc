#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/point-to-point-epc-helper.h"
#include "ns3/internet-module.h"
#include "ns3/sara-report.h"
#include "ns3/system-path.h"

#include <ctime>
#include <list>

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
  uint32_t numUe      = 40;     // muchos UEs para provocar colisión
  double   cellRadius = 200.0;
  uint32_t stopMs     = 5000;   // sim corta para validar Msg5 rápidamente
  bool     useNbSuspend = false; // usa AttachSuspendedNb (también sirve Attach normal)
  bool     reportEnabled = true;
  std::string reportPrefix = "rc1_sn2_report";
  std::string reportDir = "";
  std::string reportRunId = "";
  bool     saraActivated = true;
  bool     dropPreambleCollision = false;

  CommandLine cmd;
  cmd.AddValue ("numUe", "Número de UEs", numUe);
  cmd.AddValue ("stopMs", "Duración de la simulación en ms", stopMs);
  cmd.AddValue ("report", "Habilitar reporte CSV", reportEnabled);
  cmd.AddValue ("reportPrefix", "Prefijo de archivos de reporte", reportPrefix);
  cmd.AddValue ("reportDir", "Directorio base para reportes (se crea subcarpeta por corrida)", reportDir);
  cmd.AddValue ("reportRunId", "Nombre de la subcarpeta de corrida (si vacío se autogenera)", reportRunId);
  cmd.AddValue ("saraActivated", "Habilitar SARA", saraActivated);
  cmd.AddValue ("dropPreambleCollision", "Descartar colisiones de preámbulo", dropPreambleCollision);
  cmd.Parse (argc, argv);

  // ---------- Reproducibilidad ----------
  GlobalValue::Bind ("RngSeed", UintegerValue (12345));
  GlobalValue::Bind ("RngRun",  UintegerValue (1));

  // ---------- SARA ON + “detección perfecta” para test ----------
  Config::SetDefault ("ns3::LteEnbMac::SaraActivated",    BooleanValue (saraActivated));
  Config::SetDefault ("ns3::LteEnbMac::SaraTpr",          DoubleValue (1.0)); // detecta siempre colisión real
  Config::SetDefault ("ns3::LteEnbMac::SaraFpr",          DoubleValue (0.0)); // sin falsos positivos (para este smoke)
  Config::SetDefault ("ns3::LteEnbMac::SaraMaxGroupSize", UintegerValue (2));

  // IMPORTANTE: no descartar colisiones (flujo legacy OFF).
  // Si NO tienes atributo expuesto, pon m_dropPreambleCollision=false en el ctor para esta prueba.
  Config::SetDefault ("ns3::LteEnbMac::DropPreambleCollision", BooleanValue (dropPreambleCollision));

  // ---------- Logs útiles ----------
  LogComponentEnable ("LteEnbMac", LOG_LEVEL_INFO);
  LogComponentEnable ("LteUeMac",  LOG_LEVEL_INFO);
  // si quieres más detalle:
  // LogComponentEnable ("LteEnbRrc", LOG_LEVEL_INFO);
  // LogComponentEnable ("LteUeRrc",  LOG_LEVEL_INFO);
  // LogComponentEnable ("LtePhy",    LOG_LEVEL_INFO);

  // ---------- Helper LTE + EPC mínimo (requisito de este fork para attach) ----------
  Ptr<LteHelper> lte = CreateObject<LteHelper> ();
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

  // ---------- Dispositivos radio ----------
  NetDeviceContainer enbDevs = lte->InstallEnbDevice (enbs);
  NetDeviceContainer ueDevs  = lte->InstallUeDevice  (ues);

  // ---------- Pila IP mínima en UEs (EPC lo pide para activar bearers) ----------
  InternetStackHelper internet;
  internet.Install (ues);
  // No necesitamos remote host para este smoke; solo RA/RAR.

  // ---------- Adjuntar UEs casi a la vez para forzar colisión ----------
  Ptr<UniformRandomVariable> jitter = CreateObject<UniformRandomVariable> ();
  jitter->SetAttribute ("Min", DoubleValue (0.0));
  jitter->SetAttribute ("Max", DoubleValue (10.0)); // ms: ventanas muy apretadas -> colisiones

  for (uint32_t i = 0; i < ueDevs.GetN (); ++i)
  {
    Time t = MilliSeconds (5.0 + jitter->GetValue ());
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
          if (reportDir.empty ())
            {
              reportDir = ".";
            }
          if (reportRunId.empty ())
            {
              std::time_t now = std::time (0);
              std::tm *lt = std::localtime (&now);
              char buf[32];
              std::strftime (buf, sizeof (buf), "run_%Y%m%d_%H%M%S", lt);
              reportRunId = std::string (buf);
            }
          std::string runDir = ns3::SystemPath::Append (reportDir, reportRunId);
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
