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
  uint32_t numUe      = 70;     // muchos UEs para provocar colisión
  double   cellRadius = 200.0;
  uint32_t stopMs     = 6000;   // parar pronto: ver RAR y cortar antes de líos de Msg3
  bool     useNbSuspend = false; // usa AttachSuspendedNb (también sirve Attach normal)

  // ---------- Reproducibilidad ----------
  GlobalValue::Bind ("RngSeed", UintegerValue (12345));
  GlobalValue::Bind ("RngRun",  UintegerValue (1));

  // ---------- SARA ON + “detección perfecta” para test ----------
  Config::SetDefault ("ns3::LteEnbMac::SaraActivated",    BooleanValue (true));
  Config::SetDefault ("ns3::LteEnbMac::SaraTpr",          DoubleValue (1.0)); // detecta siempre colisión real
  Config::SetDefault ("ns3::LteEnbMac::SaraFpr",          DoubleValue (0.0)); // sin falsos positivos (para este smoke)
  Config::SetDefault ("ns3::LteEnbMac::SaraMaxGroupSize", UintegerValue (2));

  // IMPORTANTE: no descartar colisiones (flujo legacy OFF).
  // Si NO tienes atributo expuesto, pon m_dropPreambleCollision=false en el ctor para esta prueba.
  Config::SetDefault ("ns3::LteEnbMac::DropPreambleCollision", BooleanValue (false));

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
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
