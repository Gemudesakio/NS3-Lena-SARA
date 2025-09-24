// src/lte/examples/mi-escenario-nbiot-2.cc
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/point-to-point-epc-helper.h"   // EPC mínimo (requisito de Attach/AttachSuspendedNb)
#include "ns3/internet-module.h"             // Pila IP para los UEs
#include "ns3/ipv4-static-routing-helper.h"  // Ruta por defecto hacia el PGW (sin backhaul real)

using namespace ns3;

// Envolturas libres para usar Simulator::Schedule sin ambigüedad
static void
DoAttachSusp (Ptr<LteHelper> lte, Ptr<NetDevice> ue, Ptr<NetDevice> enb)
{
  lte->AttachSuspendedNb (ue, enb);
}

static void
DoAttach (Ptr<LteHelper> lte, Ptr<NetDevice> ue, Ptr<NetDevice> enb)
{
  lte->Attach (ue, enb);
}

int main (int argc, char *argv[])
{
  // ===== Parámetros =====
  uint32_t numUe      = 20;
  double   cellRadius = 250.0;
  uint32_t simTimeMs  = 3000;

  // true  => NB-IoT suspend/resume (AttachSuspendedNb + CIoT/EDT)
  // false => RA inmediato (Attach)
  bool useNbSuspend = true;

  // ===== Logs útiles =====
  LogComponentEnable ("LteEnbMac", LOG_LEVEL_INFO);
  LogComponentEnable ("LteUeMac",  LOG_LEVEL_INFO);
  LogComponentEnable ("LteEnbRrc", LOG_LEVEL_INFO);
  LogComponentEnable ("LteUeRrc",  LOG_LEVEL_INFO);
  LogComponentEnable ("LteRlc",    LOG_LEVEL_INFO);
  LogComponentEnable ("LtePhy",    LOG_LEVEL_INFO);

  // ===== Helper LTE/NB-IoT + EPC mínimo =====
  Ptr<LteHelper> lte = CreateObject<LteHelper> ();
  Ptr<PointToPointEpcHelper> epc = CreateObject<PointToPointEpcHelper> ();
  lte->SetEpcHelper (epc); // Requisito del fork para Attach/AttachSuspendedNb

  // ===== Nodos =====
  NodeContainer enbs; enbs.Create (1);
  NodeContainer ues;  ues.Create  (numUe);

  // ===== Posiciones =====
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  // eNB fijo
  Ptr<ListPositionAllocator> posEnb = CreateObject<ListPositionAllocator> ();
  posEnb->Add (Vector (0.0, 0.0, 30.0));
  mobility.SetPositionAllocator (posEnb);
  mobility.Install (enbs);

  // UEs uniformes en disco de radio cellRadius
  Ptr<UniformDiscPositionAllocator> posUe = CreateObject<UniformDiscPositionAllocator> ();
  posUe->SetX (0.0);
  posUe->SetY (0.0);
  posUe->SetRho (cellRadius);
  mobility.SetPositionAllocator (posUe);
  mobility.Install (ues);

  // ===== Dispositivos radio =====
  NetDeviceContainer enbDevs = lte->InstallEnbDevice (enbs);
  NetDeviceContainer ueDevs  = lte->InstallUeDevice  (ues);

  // ===== Pila IP mínima en UEs (requisito del EPC para activar bearers) =====
  InternetStackHelper internet;
  internet.Install (ues);

  // Asigna IPv4 a UEs a través del EPC (no necesitas remote host)
  Ipv4InterfaceContainer ueIfaces = epc->AssignUeIpv4Address (ueDevs);

  // Ruta por defecto de cada UE hacia el PGW (aunque no envíes tráfico IP)
  Ipv4StaticRoutingHelper srt;
  for (uint32_t i = 0; i < ues.GetN (); ++i)
    {
      Ptr<Ipv4StaticRouting> r = srt.GetStaticRouting (ues.Get (i)->GetObject<Ipv4> ());
      r->SetDefaultRoute (epc->GetUeDefaultGatewayAddress (), 1);
    }

  // ===== Adjuntar UEs al eNB =====
  Ptr<UniformRandomVariable> jitter = CreateObject<UniformRandomVariable> ();
  jitter->SetAttribute ("Min", DoubleValue (0.0));
  jitter->SetAttribute ("Max", DoubleValue (200.0)); // ms

  for (uint32_t i = 0; i < ueDevs.GetN (); ++i)
    {
      Ptr<LteUeNetDevice> ueNd  = ueDevs.Get (i)->GetObject<LteUeNetDevice> ();
      Ptr<LteUeRrc>       ueRrc = ueNd->GetRrc ();

      Time t = MilliSeconds (jitter->GetValue ());

      if (useNbSuspend)
        {
          // NB-IoT suspend/resume
          ueRrc->SetAttribute ("CIoT-Opt", BooleanValue (true));
          ueRrc->SetAttribute ("EDT",      BooleanValue (true));

          Simulator::Schedule (t, &DoAttachSusp, lte, ueDevs.Get (i), enbDevs.Get (0));
          // Alternativa directa:
          // lte->AttachSuspendedNb(ueDevs.Get(i), enbDevs.Get(0));
        }
      else
        {
          // RA inmediato (Attach)
          Simulator::Schedule (t, &DoAttach, lte, ueDevs.Get (i), enbDevs.Get (0));
          // Alternativa directa:
          // lte->Attach(ueDevs.Get(i), enbDevs.Get(0));
        }
    }

  // ===== Simulación =====
  Simulator::Stop (MilliSeconds (simTimeMs + 500));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
