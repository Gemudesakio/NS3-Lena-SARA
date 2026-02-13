#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/point-to-point-epc-helper.h"
#include "ns3/internet-module.h"

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
  double   cellRadius = 200.0;
  uint32_t stopMs     = 2000;   // parar pronto: ver RAR y cortar antes de líos de Msg3
  bool     useNbSuspend = false; // usa AttachSuspendedNb (también sirve Attach normal)
  bool     newSchema = true;
  bool     dropCollision = false;
  uint32_t rngRun = 1;

  CommandLine cmd (__FILE__);
  cmd.AddValue ("numUe", "Numero de UEs en la rafaga de acceso", numUe);
  cmd.AddValue ("stopMs", "Tiempo total de simulacion en ms", stopMs);
  cmd.AddValue ("newSchema", "Activa/desactiva el nuevo esquema SCMA", newSchema);
  cmd.AddValue ("dropCollision", "Descartar colisiones (legacy estricto)", dropCollision);
  cmd.AddValue ("rngRun", "RngRun para reproducibilidad/variacion", rngRun);
  cmd.Parse (argc, argv);

  // ---------- Reproducibilidad ----------
  GlobalValue::Bind ("RngSeed", UintegerValue (12345));
  GlobalValue::Bind ("RngRun",  UintegerValue (rngRun));

  // ---------- Nuevo esquema ON + “detección perfecta” para test ----------
  Config::SetDefault ("ns3::LteEnbMac::NewSchemaActivated", BooleanValue (newSchema));
  Config::SetDefault ("ns3::LteEnbMac::ScmaTpr",            DoubleValue (1.0)); // detecta siempre colisión real
  Config::SetDefault ("ns3::LteEnbMac::ScmaFpr",            DoubleValue (0.0)); // sin falsos positivos (para este smoke)
  Config::SetDefault ("ns3::LteEnbMac::ScmaMaxGroupSize",   UintegerValue (2));
  Config::SetDefault ("ns3::LteUeMac::NewSchemaActivated",  BooleanValue (newSchema));
  Config::SetDefault ("ns3::LteUeMac::ToaNumBins",          UintegerValue (64));
  Config::SetDefault ("ns3::LteUeMac::ToaToleranceBins",    UintegerValue (1));

  // IMPORTANTE: no descartar colisiones.
  // Si NO tienes atributo expuesto, pon m_dropPreambleCollision=false en el ctor para esta prueba.
  Config::SetDefault ("ns3::LteEnbMac::DropPreambleCollision", BooleanValue (dropCollision));

  // ---------- Logs útiles ----------
  LogComponentEnable ("LteEnbMac", LOG_LEVEL_INFO);
  LogComponentEnable ("LteUeMac",  LOG_LEVEL_INFO);
  LogComponentEnable ("LteEnbRrc", LOG_LEVEL_INFO);
  LogComponentEnable ("LteUeRrc",  LOG_LEVEL_INFO);
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
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
