// Importamos módulos necesarios
#include "ns3/core-module.h"     // núcleo de ns-3 (Time, Simulator, RNG, etc.)
#include "ns3/network-module.h"  // Node, NetDevice, Packet, Address
#include "ns3/mobility-module.h" // Movilidad y distribuidores de posiciones
#include "ns3/lte-module.h"      // Módulo LTE / NB-IoT (LENA-NB)

using namespace ns3;

int main (int argc, char *argv[])
{   
  //activacion de logs necesarios para debuguear
  LogComponentEnable("LteEnbMac", LOG_LEVEL_INFO); // o LOG_LEVEL_DEBUG
  LogComponentEnable("LteUeMac", LOG_LEVEL_INFO);
  LogComponentEnable("LteEnbRrc", LOG_LEVEL_INFO);
  LogComponentEnable("LteUeRrc", LOG_LEVEL_INFO);
  LogComponentEnable("LteRlc", LOG_LEVEL_INFO);
  LogComponentEnable("LtePhy", LOG_LEVEL_INFO);



  // Parámetros iniciales (CLI en módulo 2 si quieres)
  uint32_t numUe     = 20;
  double   cellRadius= 250.0;
  uint32_t simTimeMs = 3000;

  // 1) Helper LTE: instalar pila LTE/NB-IoT y cablear SAPs
  Ptr<LteHelper> lte = CreateObject<LteHelper>();

  // 2) Nodos (contenedores facilitan instalación masiva)
  NodeContainer enbs; enbs.Create(1);
  NodeContainer ues;  ues.Create(numUe);

  // 3) Posiciones: eNB fijo, UEs uniformes en disco (celda)
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

  // eNB (altura ~30 m)
  Ptr<ListPositionAllocator> posEnb = CreateObject<ListPositionAllocator>();
  posEnb->Add(Vector(0.0, 0.0, 30.0));
  mobility.SetPositionAllocator(posEnb);
  mobility.Install(enbs);

  // UEs uniformes en disco de radio cellRadius
  Ptr<UniformDiscPositionAllocator> posUe = CreateObject<UniformDiscPositionAllocator>();
  posUe->SetX(0.0); posUe->SetY(0.0); posUe->SetRho(cellRadius);
  mobility.SetPositionAllocator(posUe);
  mobility.Install(ues);

  // 4) Instalación de dispositivos LTE/NB-IoT en los nodos, el contenedor guarda los punteros a los nodos
  NetDeviceContainer enbDevs = lte->InstallEnbDevice(enbs);
  NetDeviceContainer ueDevs  = lte->InstallUeDevice (ues);

  // 5) Attach: asocia todos los UEs al eNB 0 → dispara RA (Msg1..Msg4)
  lte->Attach(ueDevs, enbDevs.Get(0));

  // (Opcional) Activar bearer de datos radio-only para estresar RLC/MAC
  // EpsBearer bearer (EpsBearer::NGBR_VOICE_VIDEO_GAMING);
  // lte->ActivateDataRadioBearer(ueDevs, bearer);

  Simulator::Stop (MilliSeconds(simTimeMs));
  Simulator::Run();
  Simulator::Destroy();
  return 0;
}
