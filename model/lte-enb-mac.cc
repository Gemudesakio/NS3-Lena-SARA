/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 * Copyright (c) 2022 Communication Networks Institute at TU Dortmund University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Marco Miozzo <marco.miozzo@cttc.es>
 *         Nicola Baldo  <nbaldo@cttc.es>
 * Modified by:
 *          Danilo Abrignani <danilo.abrignani@unibo.it> (Carrier Aggregation - GSoC 2015)
 *          Biljana Bojovic <biljana.bojovic@cttc.es> (Carrier Aggregation)
 *          Tim Gebauer <tim.gebauer@tu-dortmund.de> (NB-IoT Extension)
 *          Pascal Jörke <pascal.joerke@tu-dortmund.de> (NB-IoT Extension)
 */

#include <ns3/log.h>
#include <ns3/pointer.h>
#include <ns3/packet.h>
#include <ns3/simulator.h>
#include <ns3/build-profile.h>
#include "lte-amc.h"
#include "lte-control-messages.h"
#include "lte-enb-net-device.h"
#include "lte-ue-net-device.h"
#include "lte-rrc-header.h"

#include "ns3/lte-enb-mac.h"
#include <ns3/lte-radio-bearer-tag.h>
#include <ns3/lte-ue-phy.h>

#include "ns3/lte-mac-sap.h"
#include "ns3/lte-enb-cmac-sap.h"
#include <ns3/lte-common.h>

#include "nb-iot-data-volume-and-power-headroom-tag.h"
#include "nb-iot-buffer-status-report-tag.h"
#include "nb-iot-msg3-imsi-tag.h"
#include "nb-iot-scma-msg3-tag.h"
#include "nb-iot-toa-utils.h"
#include "sara-msg3-group-tag.h"
#include "sara-report.h"
#include "sara-ul-id-tag.h"
#include "lte-rlc-am-header.h"
#include "ns3/random-variable-stream.h"   // << NECESARIO para UniformRandomVariable

#include <algorithm>
#include <fstream>
#include <sstream>
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LteEnbMac");

NS_OBJECT_ENSURE_REGISTERED (LteEnbMac);

// //////////////////////////////////////
// member SAP forwarders (30 funciones)
// //////////////////////////////////////

/// EnbMacMemberLteEnbCmacSapProvider class
class EnbMacMemberLteEnbCmacSapProvider : public LteEnbCmacSapProvider
{
public:
  /**
   * Constructor
   *
   * \param mac the MAC
   */
  EnbMacMemberLteEnbCmacSapProvider (LteEnbMac *mac);

  // inherited from LteEnbCmacSapProvider
  virtual void ConfigureMac (uint16_t ulBandwidth, uint16_t dlBandwidth);
  virtual void AddUe (uint16_t rnti);
  virtual void RemoveUe (uint16_t rnti);
  virtual void MoveUeToResume(uint16_t rnti,uint64_t resumeId);
  virtual void ResumeUe(uint16_t rnti,uint64_t resumeId);
  virtual void RemoveUeFromScheduler(uint16_t rnti);
  virtual void AddLc (LcInfo lcinfo, LteMacSapUser *msu);
  virtual void ReconfigureLc (LcInfo lcinfo);
  virtual void ReleaseLc (uint16_t rnti, uint8_t lcid);
  virtual void UeUpdateConfigurationReq (UeConfig params);
  virtual RachConfig GetRachConfig ();
  virtual RachConfigNb GetRachConfigNb ();
  virtual void NotifyConnectionSuccessful(uint16_t rnti);
  virtual void MapTempRntiToDefRnti (uint16_t tempRnti, uint16_t assignedRnti);
  virtual void RegisterMsg4ValidityContext (uint16_t rnti,
                                            const Msg4ValidityContext &ctx);
  virtual void InvalidateMsg4ValidityContext (uint16_t rnti);
  virtual AllocateNcRaPreambleReturnValue AllocateNcRaPreamble (uint16_t rnti);
  virtual void SetLogDir(std::string logdir);

private:
  LteEnbMac *m_mac; ///< the MAC
};

EnbMacMemberLteEnbCmacSapProvider::EnbMacMemberLteEnbCmacSapProvider (LteEnbMac *mac) : m_mac (mac)
{
}

void
EnbMacMemberLteEnbCmacSapProvider::ConfigureMac (uint16_t ulBandwidth, uint16_t dlBandwidth)
{
  m_mac->DoConfigureMac (ulBandwidth, dlBandwidth);
}

void
EnbMacMemberLteEnbCmacSapProvider::AddUe (uint16_t rnti)
{
  m_mac->DoAddUe (rnti);
}

void
EnbMacMemberLteEnbCmacSapProvider::RemoveUe (uint16_t rnti)
{
  m_mac->DoRemoveUe (rnti);
}
void
EnbMacMemberLteEnbCmacSapProvider::MoveUeToResume(uint16_t rnti, uint64_t resumeId)
{
  m_mac->DoMoveUeToResume(rnti,resumeId);
}
void
EnbMacMemberLteEnbCmacSapProvider::RemoveUeFromScheduler(uint16_t rnti)
{
  m_mac->DoRemoveUeFromScheduler(rnti);
}
void
EnbMacMemberLteEnbCmacSapProvider::ResumeUe(uint16_t rnti, uint64_t resumeId)
{
  m_mac->DoResumeUe(rnti,resumeId);
}
void
EnbMacMemberLteEnbCmacSapProvider::AddLc (LcInfo lcinfo, LteMacSapUser *msu)
{
  m_mac->DoAddLc (lcinfo, msu);
}

void
EnbMacMemberLteEnbCmacSapProvider::ReconfigureLc (LcInfo lcinfo)
{
  m_mac->DoReconfigureLc (lcinfo);
}

void
EnbMacMemberLteEnbCmacSapProvider::ReleaseLc (uint16_t rnti, uint8_t lcid)
{
  m_mac->DoReleaseLc (rnti, lcid);
}

void
EnbMacMemberLteEnbCmacSapProvider::UeUpdateConfigurationReq (UeConfig params)
{
  m_mac->DoUeUpdateConfigurationReq (params);
}

LteEnbCmacSapProvider::RachConfig
EnbMacMemberLteEnbCmacSapProvider::GetRachConfig ()
{
  return m_mac->DoGetRachConfig ();
}

LteEnbCmacSapProvider::RachConfigNb
EnbMacMemberLteEnbCmacSapProvider::GetRachConfigNb ()
{
  return m_mac->DoGetRachConfigNb ();
}

void 
EnbMacMemberLteEnbCmacSapProvider::NotifyConnectionSuccessful(uint16_t rnti){
  m_mac->DoNotifyConnectionSuccessful(rnti);
}

void
EnbMacMemberLteEnbCmacSapProvider::MapTempRntiToDefRnti (uint16_t tempRnti,
                                                         uint16_t assignedRnti)
{
  m_mac->DoMapTempRntiToDefRnti (tempRnti, assignedRnti);
}

void
EnbMacMemberLteEnbCmacSapProvider::RegisterMsg4ValidityContext (
    uint16_t rnti, const Msg4ValidityContext &ctx)
{
  m_mac->DoRegisterMsg4ValidityContext (rnti, ctx);
}

void
EnbMacMemberLteEnbCmacSapProvider::InvalidateMsg4ValidityContext (uint16_t rnti)
{
  m_mac->DoInvalidateMsg4ValidityContext (rnti);
}

LteEnbCmacSapProvider::AllocateNcRaPreambleReturnValue
EnbMacMemberLteEnbCmacSapProvider::AllocateNcRaPreamble (uint16_t rnti)
{
  return m_mac->DoAllocateNcRaPreamble (rnti);
}

void
EnbMacMemberLteEnbCmacSapProvider::SetLogDir(std::string logdir)
{
  return m_mac->DoSetLogDir(logdir);
}

/// EnbMacMemberFfMacSchedSapUser class
class EnbMacMemberFfMacSchedSapUser : public FfMacSchedSapUser
{
public:
  /**
   * Constructor
   * 
   * \param mac the MAC
   */
  EnbMacMemberFfMacSchedSapUser (LteEnbMac *mac);

  virtual void SchedDlConfigInd (const struct SchedDlConfigIndParameters &params);
  virtual void SchedUlConfigInd (const struct SchedUlConfigIndParameters &params);

private:
  LteEnbMac *m_mac; ///< the MAC
};

EnbMacMemberFfMacSchedSapUser::EnbMacMemberFfMacSchedSapUser (LteEnbMac *mac) : m_mac (mac)
{
}

void
EnbMacMemberFfMacSchedSapUser::SchedDlConfigInd (const struct SchedDlConfigIndParameters &params)
{
  m_mac->DoSchedDlConfigInd (params);
}

void
EnbMacMemberFfMacSchedSapUser::SchedUlConfigInd (const struct SchedUlConfigIndParameters &params)
{
  m_mac->DoSchedUlConfigInd (params);
}

/// EnbMacMemberFfMacCschedSapUser class
class EnbMacMemberFfMacCschedSapUser : public FfMacCschedSapUser
{
public:
  /**
   * Constructor
   *
   * \param mac the MAC
   */
  EnbMacMemberFfMacCschedSapUser (LteEnbMac *mac);

  virtual void CschedCellConfigCnf (const struct CschedCellConfigCnfParameters &params);
  virtual void CschedUeConfigCnf (const struct CschedUeConfigCnfParameters &params);
  virtual void CschedLcConfigCnf (const struct CschedLcConfigCnfParameters &params);
  virtual void CschedLcReleaseCnf (const struct CschedLcReleaseCnfParameters &params);
  virtual void CschedUeReleaseCnf (const struct CschedUeReleaseCnfParameters &params);
  virtual void CschedUeConfigUpdateInd (const struct CschedUeConfigUpdateIndParameters &params);
  virtual void CschedCellConfigUpdateInd (const struct CschedCellConfigUpdateIndParameters &params);

private:
  LteEnbMac *m_mac; ///< the MAC
};

EnbMacMemberFfMacCschedSapUser::EnbMacMemberFfMacCschedSapUser (LteEnbMac *mac) : m_mac (mac)
{
}

void
EnbMacMemberFfMacCschedSapUser::CschedCellConfigCnf (
    const struct CschedCellConfigCnfParameters &params)
{
  m_mac->DoCschedCellConfigCnf (params);
}

void
EnbMacMemberFfMacCschedSapUser::CschedUeConfigCnf (const struct CschedUeConfigCnfParameters &params)
{
  m_mac->DoCschedUeConfigCnf (params);
}

void
EnbMacMemberFfMacCschedSapUser::CschedLcConfigCnf (const struct CschedLcConfigCnfParameters &params)
{
  m_mac->DoCschedLcConfigCnf (params);
}

void
EnbMacMemberFfMacCschedSapUser::CschedLcReleaseCnf (
    const struct CschedLcReleaseCnfParameters &params)
{
  m_mac->DoCschedLcReleaseCnf (params);
}

void
EnbMacMemberFfMacCschedSapUser::CschedUeReleaseCnf (
    const struct CschedUeReleaseCnfParameters &params)
{
  m_mac->DoCschedUeReleaseCnf (params);
}

void
EnbMacMemberFfMacCschedSapUser::CschedUeConfigUpdateInd (
    const struct CschedUeConfigUpdateIndParameters &params)
{
  m_mac->DoCschedUeConfigUpdateInd (params);
}

void
EnbMacMemberFfMacCschedSapUser::CschedCellConfigUpdateInd (
    const struct CschedCellConfigUpdateIndParameters &params)
{
  m_mac->DoCschedCellConfigUpdateInd (params);
}

// //////////////////////////////////////
// PHY SAP (11 funciones) 
// //////////////////////////////////////
class EnbMacMemberLteEnbPhySapUser : public LteEnbPhySapUser
{
public:
  /**
   * Constructor
   *
   * \param mac the MAC
   */
  EnbMacMemberLteEnbPhySapUser (LteEnbMac *mac);

  // inherited from LteEnbPhySapUser
  virtual void ReceivePhyPdu (Ptr<Packet> p);
  virtual void SubframeIndication (uint32_t frameNo, uint32_t subframeNo);
  virtual void ReceiveLteControlMessage (Ptr<LteControlMessage> msg);
  virtual void ReceiveRachPreamble (uint32_t prachId);
  virtual void ReceiveNprachPreamble (uint32_t prachId, uint8_t subcarrierOffset, uint32_t ranti,
                                      uint32_t senderMetaId);
  virtual void UlCqiReport (FfMacSchedSapProvider::SchedUlCqiInfoReqParameters ulcqi);
  virtual void UlCqiReportNb (std::vector<double> ulcqi);
  virtual void UlInfoListElementHarqFeeback (UlInfoListElement_s params);
  virtual void DlInfoListElementHarqFeeback (DlInfoListElement_s params);

private:
  LteEnbMac *m_mac; ///< the MAC
};

EnbMacMemberLteEnbPhySapUser::EnbMacMemberLteEnbPhySapUser (LteEnbMac *mac) : m_mac (mac)
{
}

void
EnbMacMemberLteEnbPhySapUser::ReceivePhyPdu (Ptr<Packet> p)
{
  m_mac->DoReceivePhyPdu (p);
}

void
// ====== Paso 1 =====
//====== Se registra el tiempo, se inicializa el scheduler NB-IoT ======
EnbMacMemberLteEnbPhySapUser::SubframeIndication (uint32_t frameNo, uint32_t subframeNo)
{
  m_mac->DoSubframeIndicationNb (frameNo, subframeNo);
}

void
EnbMacMemberLteEnbPhySapUser::ReceiveLteControlMessage (Ptr<LteControlMessage> msg)
{
  m_mac->DoReceiveLteControlMessage (msg);
}

void
EnbMacMemberLteEnbPhySapUser::ReceiveRachPreamble (uint32_t prachId)
{
  m_mac->DoReceiveRachPreamble (prachId);
}

void
//====== funcion que recibe el preambulo desde la capa physical ======
EnbMacMemberLteEnbPhySapUser::ReceiveNprachPreamble (uint32_t prachId,
                                                     uint8_t subcarrierOffset,
                                                     uint32_t ranti,
                                                     uint32_t senderMetaId)
{
  m_mac->DoReceiveNprachPreamble (prachId, subcarrierOffset, ranti, senderMetaId);
}
void
EnbMacMemberLteEnbPhySapUser::UlCqiReport (FfMacSchedSapProvider::SchedUlCqiInfoReqParameters ulcqi)
{
  m_mac->DoUlCqiReport (ulcqi);
}
void
EnbMacMemberLteEnbPhySapUser::UlCqiReportNb (std::vector<double> ulcqi)
{
  m_mac->DoUlCqiReportNb (ulcqi);
}
void
EnbMacMemberLteEnbPhySapUser::UlInfoListElementHarqFeeback (UlInfoListElement_s params)
{
  m_mac->DoUlInfoListElementHarqFeeback (params);
}

void
EnbMacMemberLteEnbPhySapUser::DlInfoListElementHarqFeeback (DlInfoListElement_s params)
{
  m_mac->DoDlInfoListElementHarqFeeback (params);
}

// //////////////////////////////////////
// generic LteEnbMac methods 31 funciones
// //////////////////////////////////////

TypeId
LteEnbMac::GetTypeId (void)
{
  static TypeId tid =
      TypeId ("ns3::LteEnbMac")
          .SetParent<Object> ()
          .SetGroupName ("Lte")
          .AddConstructor<LteEnbMac> ()
          .AddAttribute ("NumberOfRaPreambles",
                         "how many random access preambles are available for the contention based "
                         "RACH process",
                         UintegerValue (52),
                         MakeUintegerAccessor (&LteEnbMac::m_numberOfRaPreambles),
                         MakeUintegerChecker<uint8_t> (4, 64))
          .AddAttribute ("PreambleTransMax",
                         "Maximum number of random access preamble transmissions",
                         UintegerValue (50), MakeUintegerAccessor (&LteEnbMac::m_preambleTransMax),
                         MakeUintegerChecker<uint8_t> (3, 200))
          .AddAttribute ("RaResponseWindowSize",
                         "length of the window (in TTIs) for the reception of the random access "
                         "response (RAR); the resulting RAR timeout is this value + 3 ms",
                         UintegerValue (3),
                         MakeUintegerAccessor (&LteEnbMac::m_raResponseWindowSize),
                         MakeUintegerChecker<uint8_t> (2, 10))
          .AddAttribute ("ConnEstFailCount", "how many time T300 timer can expire on the same cell",
                         UintegerValue (1), MakeUintegerAccessor (&LteEnbMac::m_connEstFailCount),
                         MakeUintegerChecker<uint8_t> (1, 4))
          .AddTraceSource ("DlScheduling", "Information regarding DL scheduling.",
                           MakeTraceSourceAccessor (&LteEnbMac::m_dlScheduling),
                           "ns3::LteEnbMac::DlSchedulingTracedCallback")
          .AddTraceSource ("UlScheduling", "Information regarding UL scheduling.",
                           MakeTraceSourceAccessor (&LteEnbMac::m_ulScheduling),
                           "ns3::LteEnbMac::UlSchedulingTracedCallback")
          .AddAttribute ("ComponentCarrierId",
                         "ComponentCarrier Id, needed to reply on the appropriate sap.",
                         UintegerValue (0), MakeUintegerAccessor (&LteEnbMac::m_componentCarrierId),
                         MakeUintegerChecker<uint8_t> (0, 4))
          .AddAttribute ("SaraActivated",
            "Enable SARA collision detector and group RARs.",
            BooleanValue (false),
            MakeBooleanAccessor (&LteEnbMac::m_saraActivated),
            MakeBooleanChecker ())

          .AddAttribute ("SaraTpr",
            "Deprecated alias of CollisionDetectorTpr for backward compatibility.",
            DoubleValue (0.975),
            MakeDoubleAccessor (&LteEnbMac::m_collisionDetectorTpr),
            MakeDoubleChecker<double> (0.0, 1.0))

          .AddAttribute ("SaraFpr",
            "Deprecated alias of CollisionDetectorFpr for backward compatibility.",
            DoubleValue (0.001),
            MakeDoubleAccessor (&LteEnbMac::m_collisionDetectorFpr),
            MakeDoubleChecker<double> (0.0, 1.0))

          .AddAttribute ("SaraMaxGroupSize",
            "Max UEs grouped per RAPID when SARA detects a collision.",
            UintegerValue (2),
            MakeUintegerAccessor (&LteEnbMac::m_saraMaxGroupSize),
            MakeUintegerChecker<uint8_t> (1, 8))
  //=========================== Nuevo esquema + detector de colisiones ===========================
          .AddAttribute ("NewSchemaActivated",
            "Enable the new SCMA-based random access scheme. If false, legacy flow is used.",
            BooleanValue (false),
            MakeBooleanAccessor (&LteEnbMac::m_newSchemaActivated),
            MakeBooleanChecker ())

          .AddAttribute ("NumScmaCodebooks",
            "Number of logical SCMA codebooks per physical subcarrier.",
            UintegerValue (6),
            MakeUintegerAccessor (&LteEnbMac::m_numScmaCodebooks),
            MakeUintegerChecker<uint8_t> (1, 16))

          .AddAttribute ("ToaNumBins",
            "Number of quantization bins used for local ToA matching.",
            UintegerValue (64),
            MakeUintegerAccessor (&LteEnbMac::m_toaNumBins),
            MakeUintegerChecker<uint16_t> (2, 4096))

          .AddAttribute ("ToaToleranceBins",
            "Tolerance (in bins) for local ToA matching.",
            UintegerValue (1),
            MakeUintegerAccessor (&LteEnbMac::m_toaToleranceBins),
            MakeUintegerChecker<uint16_t> (0, 64))

          .AddAttribute ("CollisionDetectorTpr",
            "Common collision-detector true-positive probability [0..1] shared by new and SARA.",
            DoubleValue (0.975),
            MakeDoubleAccessor (&LteEnbMac::m_collisionDetectorTpr),
            MakeDoubleChecker<double> (0.0, 1.0))

          .AddAttribute ("CollisionDetectorFpr",
            "Common collision-detector false-positive probability [0..1] shared by new and SARA.",
            DoubleValue (0.001),
            MakeDoubleAccessor (&LteEnbMac::m_collisionDetectorFpr),
            MakeDoubleChecker<double> (0.0, 1.0))

          .AddAttribute ("ScmaTpr",
            "Deprecated alias of CollisionDetectorTpr for backward compatibility.",
            DoubleValue (0.975),
            MakeDoubleAccessor (&LteEnbMac::m_collisionDetectorTpr),
            MakeDoubleChecker<double> (0.0, 1.0))

          .AddAttribute ("ScmaFpr",
            "Deprecated alias of CollisionDetectorFpr for backward compatibility.",
            DoubleValue (0.001),
            MakeDoubleAccessor (&LteEnbMac::m_collisionDetectorFpr),
            MakeDoubleChecker<double> (0.0, 1.0))

          .AddAttribute ("ScmaMaxGroupSize",
            "Max UEs grouped per RAPID when the detector marks collision.",
            UintegerValue (2),
            MakeUintegerAccessor (&LteEnbMac::m_scmaMaxGroupSize),
            MakeUintegerChecker<uint8_t> (1, 8))

          .AddAttribute ("DropPreambleCollision",
                "Deprecated no-op. Collision handling now always follows mode logic "
                "(legacy/sara/new with scheduled fallback).",
                BooleanValue (false),
                MakeBooleanAccessor (&LteEnbMac::m_dropPreambleCollision),
                MakeBooleanChecker ());
            return tid;
}

LteEnbMac::LteEnbMac () : m_ccmMacSapUser (0)
{
  NS_LOG_FUNCTION (this);
  m_macSapProvider = new EnbMacMemberLteMacSapProvider<LteEnbMac> (this);
  m_cmacSapProvider = new EnbMacMemberLteEnbCmacSapProvider (this);
  m_schedSapUser = new EnbMacMemberFfMacSchedSapUser (this);
  m_cschedSapUser = new EnbMacMemberFfMacCschedSapUser (this);
  m_enbPhySapUser = new EnbMacMemberLteEnbPhySapUser (this);
  m_ccmMacSapProvider = new MemberLteCcmMacSapProvider<LteEnbMac> (this);
  m_dropPreambleCollision = false;

  // --- Nuevo esquema: defaults coherentes con atributos ---
  m_newSchemaActivated = false;
  m_numScmaCodebooks = 6;
  m_toaNumBins = 64;
  m_toaToleranceBins = 1;
  m_msg1RxCount = 0;
  m_msg2TxCount = 0;
  m_msg3RxCount = 0;
  m_msg3AcceptedCount = 0;
  m_msg3DropCount = 0;
  m_collisionRapidCount = 0;
  m_collisionUeCount = 0;

  // --- SARA ---
  m_saraActivated = false;
  m_saraMaxGroupSize = 2;

  // --- Detector de colisiones (matriz de confusión) ---
  m_collisionDetectorTpr = 0.975;
  m_collisionDetectorFpr = 0.001;
  m_scmaMaxGroupSize = 2;
  if (m_collisionDetectorRng == 0)
    {
      m_collisionDetectorRng = CreateObject<UniformRandomVariable> ();
      m_collisionDetectorRng->SetAttribute ("Min", DoubleValue (0.0));
      m_collisionDetectorRng->SetAttribute ("Max", DoubleValue (1.0));
    }
}

LteEnbMac::~LteEnbMac ()
{
  NS_LOG_FUNCTION (this);
  m_mac_logging = false;
}

void
LteEnbMac::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("[ENB][SUMMARY] MSG1_RX=" << m_msg1RxCount
               << " RAPIDS_COLISIONADOS=" << m_collisionRapidCount
               << " UES_EN_COLISION=" << m_collisionUeCount);
  NS_LOG_INFO ("[ENB][SUMMARY] MSG2_TX=" << m_msg2TxCount);
  NS_LOG_INFO ("[ENB][SUMMARY] MSG3_RX_OK=" << m_msg3RxCount
               << " MSG3_TX_TO_RLC=" << m_msg3AcceptedCount
               << " MSG3_DROP=" << m_msg3DropCount);
  NS_LOG_INFO ("[ENB][SUMMARY] MSG3 recibidos = " << m_msg3RxCount);
  NS_LOG_INFO ("[ENB][SUMMARY] MSG3 enviados = " << m_msg3AcceptedCount);
  const uint32_t msg4Ok = SaraReport::GetMsg4UeAcceptedUniqueCount ();
  if (msg4Ok > 0)
    {
      NS_LOG_INFO ("[ENB][SUMMARY] MSG4 recibidos correctamente = " << msg4Ok);
    }
 //limpia vartiables de estado
  m_dlCqiReceived.clear ();
  m_ulCqiReceived.clear ();
  m_ulCeReceived.clear ();
  m_dlInfoListReceived.clear ();
  m_ulInfoListReceived.clear ();
  m_miDlHarqProcessesPackets.clear ();
  m_rntiToRapId.clear ();
  m_saraUlTags.clear ();
  m_rntiMsg3WindowEnd.clear ();
  m_msg3Buffers.clear ();
  m_tempRntiToDefRnti.clear ();
  m_sharedFallbackBlockedRnti.clear ();
  m_nprachRxMetaByRapid.clear ();
  m_expectedScmaMsg3ByTcRnti.clear ();
  m_sharedFallbackMsg3ByTcRnti.clear ();

  // Detector de colisiones: suelta la ref del RNG (smart pointer)
  m_collisionDetectorRng = 0;

  // Solo si existe; y nulificar luego
  if (m_schedulerNb)
    {
      m_schedulerNb->DoDispose();
      m_schedulerNb = nullptr;
    }
// Elimina los miembros SAP
  delete m_macSapProvider;    m_macSapProvider = 0;
  delete m_cmacSapProvider;   m_cmacSapProvider = 0;
  delete m_schedSapUser;      m_schedSapUser = 0;
  delete m_cschedSapUser;     m_cschedSapUser = 0;
  delete m_enbPhySapUser;     m_enbPhySapUser = 0;
  delete m_ccmMacSapProvider; m_ccmMacSapProvider = 0;

  Object::DoDispose ();
}


void
LteEnbMac::SetComponentCarrierId (uint8_t index)
{
  m_componentCarrierId = index;
}

void
LteEnbMac::SetFfMacSchedSapProvider (FfMacSchedSapProvider *s)
{
  m_schedSapProvider = s;
}

FfMacSchedSapUser *
LteEnbMac::GetFfMacSchedSapUser (void)
{
  return m_schedSapUser;
}

void
LteEnbMac::SetFfMacCschedSapProvider (FfMacCschedSapProvider *s)
{
  m_cschedSapProvider = s;
}

FfMacCschedSapUser *
LteEnbMac::GetFfMacCschedSapUser (void)
{
  return m_cschedSapUser;
}

void
LteEnbMac::SetLteMacSapUser (LteMacSapUser *s)
{
  m_macSapUser = s;
}

LteMacSapProvider *
LteEnbMac::GetLteMacSapProvider (void)
{
  return m_macSapProvider;
}

void
LteEnbMac::SetLteEnbCmacSapUser (LteEnbCmacSapUser *s)
{
  m_cmacSapUser = s;
}

LteEnbCmacSapProvider *
LteEnbMac::GetLteEnbCmacSapProvider (void)
{
  return m_cmacSapProvider;
}

void
LteEnbMac::SetLteEnbPhySapProvider (LteEnbPhySapProvider *s)
{
  m_enbPhySapProvider = s;
}

LteEnbPhySapUser *
LteEnbMac::GetLteEnbPhySapUser ()
{
  return m_enbPhySapUser;
}

void
LteEnbMac::SetLteCcmMacSapUser (LteCcmMacSapUser *s)
{
  m_ccmMacSapUser = s;
}

LteCcmMacSapProvider *
LteEnbMac::GetLteCcmMacSapProvider ()
{
  return m_ccmMacSapProvider;
}

//DoSubframeIndication  trabaja unicamente en lte clasico no en NB-IoT
void
LteEnbMac::DoSubframeIndication (uint32_t frameNo, uint32_t subframeNo)
{
  NS_LOG_FUNCTION (this << " EnbMac - frame " << frameNo << " subframe " << subframeNo);

  // Store current frame / subframe number
  m_frameNo = frameNo; //trama actual
  m_subframeNo = subframeNo; // numero de subtrama dentro de la trama actual

  // --- DOWNLINK ---
  // 1: envia Dl-CQI info a el scheduler
  if (m_dlCqiReceived.size () > 0)
    {
      FfMacSchedSapProvider::SchedDlCqiInfoReqParameters dlcqiInfoReq;
      // recortando o enmascarando frameNo y subframeNo a 10 y 4 bits respectivamente
      //<<4 desplaza 4 bits a la izquierda para combinar los 4 bits de subframeNo con los 10 bits de frameNo
      dlcqiInfoReq.m_sfnSf = ((0x3FF & frameNo) << 4) | (0xF & subframeNo);

      //se agregan a la lista de CQI, borramos el primer registro y enviamos la cqi req al scheduler
      dlcqiInfoReq.m_cqiList.insert (dlcqiInfoReq.m_cqiList.begin (), m_dlCqiReceived.begin (),
                                     m_dlCqiReceived.end ());
      m_dlCqiReceived.erase (m_dlCqiReceived.begin (), m_dlCqiReceived.end ());
      m_schedSapProvider->SchedDlCqiInfoReq (dlcqiInfoReq);
    }

      //2: Procesar los preámbulos RACH recibidos y notificar al programador
  if (!m_receivedRachPreambleCount.empty ())
    {
      FfMacSchedSapProvider::SchedDlRachInfoReqParameters rachInfoReqParams;
      NS_ASSERT (subframeNo > 0 && subframeNo <= 10); // validacion la subtrama debe ser entre 1 y 10
      for (std::map<uint8_t, uint32_t>::const_iterator it = m_receivedRachPreambleCount.begin ();
           it != m_receivedRachPreambleCount.end (); ++it) // mapa RAPID - count
        {
          NS_LOG_INFO (this << " preambleId " << (uint32_t) it->first << ": " << it->second
                            << " received");
          NS_ASSERT (it->second != 0); //validacion, avanza si el count es diferente de 0
          if (it->second > 1)
            {
              NS_LOG_INFO ("preambleId " << (uint32_t) it->first << ": collision");
              //En caso de colisión, asumimos que no se recibió ningún preámbulo, por lo que no se envía ningún RAR. no hay mas logica...
            }
          else
            {
              uint16_t rnti;
              std::map<uint8_t, NcRaPreambleInfo>::iterator jt = //vemos en que preambulo estamos parados
                  m_allocatedNcRaPreambleMap.find (it->first);
              if (jt != m_allocatedNcRaPreambleMap.end ()) //si el preambulo ya fue asignado
                {
                  rnti = jt->second.rnti;
                  NS_LOG_INFO ("preambleId previously allocated for NC based RA, RNTI ="
                               << (uint32_t) rnti << ", sending RAR");
                }
              else  //si el preambulo no era esperado, se asigna un T-C-RNTI
                {
                  rnti = m_cmacSapUser->AllocateTemporaryCellRnti ();
                  NS_LOG_INFO ("preambleId " << (uint32_t) it->first << ": allocated T-C-RNTI "
                                             << (uint32_t) rnti << ", sending RAR");
                }

              RachListElement_s rachLe;
              rachLe.m_rnti = rnti;
              rachLe.m_estimatedSize = 144; // to be confirmed
              rachInfoReqParams.m_rachList.push_back (rachLe);
              m_rapIdRntiMap.insert (std::pair<uint16_t, uint32_t> (rnti, it->first));
            }
        }
      m_schedSapProvider->SchedDlRachInfoReq (rachInfoReqParams);
      m_receivedRachPreambleCount.clear ();
    }
  // Get downlink transmission opportunities
  uint32_t dlSchedFrameNo = m_frameNo;
  uint32_t dlSchedSubframeNo = m_subframeNo;
  // NS_LOG_DEBUG (this << " sfn " << frameNo << " sbfn " << subframeNo);
  if (dlSchedSubframeNo + m_macChTtiDelay > 10) // si el tiempo de subtrama + m_macChTtiDelay es mayor a 10
    {
      dlSchedFrameNo++; //aumentamos subtrama
      dlSchedSubframeNo = (dlSchedSubframeNo + m_macChTtiDelay) % 10; //calculamos indice de subtrama
    }
  else
    { //seguimos en la misma trama
      dlSchedSubframeNo = dlSchedSubframeNo + m_macChTtiDelay; // calculamos indice de subtrama con delay
    }
  FfMacSchedSapProvider::SchedDlTriggerReqParameters dlparams;
  dlparams.m_sfnSf = ((0x3FF & dlSchedFrameNo) << 4) | (0xF & dlSchedSubframeNo);

  // Forward DL HARQ feebacks collected during last TTI
  if (m_dlInfoListReceived.size () > 0)
    {
      dlparams.m_dlInfoList = m_dlInfoListReceived;
      // empty local buffer
      m_dlInfoListReceived.clear ();
    }

  m_schedSapProvider->SchedDlTriggerReq (dlparams);

  // --- UPLINK ---
  // Send UL-CQI info to the scheduler
  for (uint16_t i = 0; i < m_ulCqiReceived.size (); i++)
    {
      if (subframeNo > 1)
        {
          m_ulCqiReceived.at (i).m_sfnSf = ((0x3FF & frameNo) << 4) | (0xF & (subframeNo - 1));
        }
      else
        {
          m_ulCqiReceived.at (i).m_sfnSf = ((0x3FF & (frameNo - 1)) << 4) | (0xF & 10);
        }
      m_schedSapProvider->SchedUlCqiInfoReq (m_ulCqiReceived.at (i));
    }
  m_ulCqiReceived.clear ();

  // Send BSR reports to the scheduler
  if (m_ulCeReceived.size () > 0)
    {
      FfMacSchedSapProvider::SchedUlMacCtrlInfoReqParameters ulMacReq;
      ulMacReq.m_sfnSf = ((0x3FF & frameNo) << 4) | (0xF & subframeNo);
      ulMacReq.m_macCeList.insert (ulMacReq.m_macCeList.begin (), m_ulCeReceived.begin (),
                                   m_ulCeReceived.end ());
      m_ulCeReceived.erase (m_ulCeReceived.begin (), m_ulCeReceived.end ());
      m_schedSapProvider->SchedUlMacCtrlInfoReq (ulMacReq);
    }

  // Get uplink transmission opportunities
  uint32_t ulSchedFrameNo = m_frameNo;
  uint32_t ulSchedSubframeNo = m_subframeNo;
  //   NS_LOG_DEBUG (this << " sfn " << frameNo << " sbfn " << subframeNo);
  if (ulSchedSubframeNo + (m_macChTtiDelay + UL_PUSCH_TTIS_DELAY) > 10)
    {
      ulSchedFrameNo++;
      ulSchedSubframeNo = (ulSchedSubframeNo + (m_macChTtiDelay + UL_PUSCH_TTIS_DELAY)) % 10;
    }
  else
    {
      ulSchedSubframeNo = ulSchedSubframeNo + (m_macChTtiDelay + UL_PUSCH_TTIS_DELAY);
    }
  FfMacSchedSapProvider::SchedUlTriggerReqParameters ulparams;
  ulparams.m_sfnSf = ((0x3FF & ulSchedFrameNo) << 4) | (0xF & ulSchedSubframeNo);

  // Forward DL HARQ feebacks collected during last TTI
  if (m_ulInfoListReceived.size () > 0)
    {
      ulparams.m_ulInfoList = m_ulInfoListReceived;
      // empty local buffer
      m_ulInfoListReceived.clear ();
    }

  m_schedSapProvider->SchedUlTriggerReq (ulparams);
}

//======= PREAMBULOS VALIDOS RECIBIDOS =======
void LteEnbMac::CheckPreambleReceptionForAllCoverageClases(){
  //CE0 
  CheckIfPreambleWasReceived(m_ce0Parameter, false);
  //CE1 
  CheckIfPreambleWasReceived(m_ce1Parameter, false);
  //CE2 
  CheckIfPreambleWasReceived(m_ce2Parameter, false);
  if(m_edt){

    //CE0 Edt
    CheckIfPreambleWasReceived(m_ce0ParameterEdt, true);
    //CE1 Edt
    CheckIfPreambleWasReceived(m_ce1ParameterEdt, true);
    //CE2 Edt
    CheckIfPreambleWasReceived(m_ce2ParameterEdt, true);
  }
}

void
LteEnbMac::CheckIfPreambleWasReceived (NbIotRrcSap::NprachParametersNb ce, bool edt) 
{
  //======= PROCESO 1: CALCULO OCCASION NPRACH =======
 
  // cantidad de subtramas que han avanzado desde tiempo 0 hasta el momento actual
  uint32_t currentsubframe = Simulator::Now().GetMilliSeconds();

  //calculamos el modulo de la totalidad de tramas con el periodo del ciclo Nprach (en tramas)
  // el resultado (residuo) es el numero del ciclo Nprach en el que estamos  
  uint16_t window_condition = ( currentsubframe/10) % (NbIotRrcSap::ConvertNprachPeriodicity2int (ce) / 10);

  //restamos a la cantidad de tramas actuales, la cantidad de tramas que corresponden al ciclo Nprach
  //para poder saber exactamente donde inicia el ciclo Nprach
  uint32_t lastPeriodStart = (currentsubframe/10) - window_condition;

  //en subtramas calculamos el inicio real del ciclo Nprach, teniendo encuenta el offset de inicio
  uint32_t startSubframeNprachOccasion = lastPeriodStart*10 + NbIotRrcSap::ConvertNprachStartTime2int(ce);

  //la diferencia nos da como resultado el tiempo en milisegundos que han transcurrido desde el inicio del ciclo Nprach
  //hasta el momento actual
  uint16_t timeSinceOcassion = currentsubframe - startSubframeNprachOccasion;

  //======= PROCESO 2: CALCULO DEL INSTANTE EXACTO DE MUESTREO =======

//3GPP define dos longitudes de CP para NP‑RACH: ~66.7 µs y ~266.7 µs. El más habitual para CE levels altos es 266.7 µs.
/* La especificación NB‑IoT (3GPP TS 36.211 §5.12) define el “symbol group” de NPRACH como:
Un bloque formado por:
Un único prefijo cíclico (CP)
Cinco símbolos contiguos (cada uno de 8192 muestras) */
  m_sib2Nb.radioResourceConfigCommon.nprachConfig.nprachCpLength =
  NbIotRrcSap::NprachConfig::NprachCpLength::us266dot7; //1/3.75khz
  double ts = 1000.0 / (15000.0 * 2048.0); //calculamos la duracion de una muestra de la FFT en ms
  double preambleSymbolTime = 8192.0 * ts; //calculamos el tiempo de simbolo de preambulo nprach
  //cada simbolo nprach dura 4 veces mas que un ofdm (2048x4) 

  double preambleGroupTimeNoCP = 5.0 * preambleSymbolTime; //tiempo de un preamble group sin CP
  double preambleGroupTime =
      NbIotRrcSap::ConvertNprachCpLenght2double (m_sib2Nb.radioResourceConfigCommon.nprachConfig) +
      preambleGroupTimeNoCP; //tiempo de un preamble group con CP

  //La norma exige 4 repeticiones de cada “preamble group” para reforzar la detección en canales débiles
  double preambleRepetition = 4.0 * preambleGroupTime;

 //El tiempo completo que el UE dedicará a transmitir su preámbulo (Msg 1), haciendo todas las repeticiones definidas para su nivel de cobertura.
  double time = NbIotRrcSap::ConvertNumRepetitionsPerPreambleAttempt2int (ce) *
                                  preambleRepetition;


  //======= PROCESO 3: COMPROBACION DE LA OCASION EXACTA NP-RACH =======
  if (std::ceil(time)+1 != timeSinceOcassion){ 
    return; //Sale inmediatamente de la función CheckIfPreambleWasReceived, sin ejecutar ninguna de las líneas que siguen.
  }//le ejecucion continua solo cuando ha pasado el tiempo necesario para que el ue haya echo si envio de poreambulo 



  std::map<uint8_t, uint32_t> receivedNprachs; //creamos variables que recibira un map de dos valores rapid-count
  uint8_t subcarrierOffset = NbIotRrcSap::ConvertNprachSubcarrierOffset2int (ce); //extraemos la subportadora del ce level 

  receivedNprachs = m_receivedNprachPreambleCount[subcarrierOffset]; //con la subportadora extraemos el conteo de rapid

  std::vector<std::pair<int, NbIotRrcSap::Rar>> m_rarQueue; // vector de pares contendra en cada indice pair<Ranti, Rar>
  if (receivedNprachs.size () > 0)
    {
      //int rnti = m_cmacSapUser->AllocateTemporaryCellRnti ();

      for (std::map<uint8_t, uint32_t>::iterator iter = receivedNprachs.begin ();
           iter != receivedNprachs.end (); ++iter)
        {
          const uint16_t rapid = static_cast<uint16_t> (subcarrierOffset + iter->first);
          const uint16_t ranti = m_rapIdRantiMap[rapid];
          NS_LOG_INFO ("[ENB][MSG1][OCCASION] rapid=" << rapid
                       << " ra-rnti=" << ranti
                       << " preamble=" << static_cast<uint32_t> (iter->first)
                       << " ueCount=" << iter->second);

          const bool schemeActive = (m_newSchemaActivated || m_saraActivated);
          const bool actualCollision = (iter->second > 1);
          bool predictCollision = actualCollision;

          if (schemeActive)
            {
              if (m_collisionDetectorRng == 0)
                {
                  m_collisionDetectorRng = CreateObject<UniformRandomVariable> ();
                  m_collisionDetectorRng->SetAttribute ("Min", DoubleValue (0.0));
                  m_collisionDetectorRng->SetAttribute ("Max", DoubleValue (1.0));
                }
              double u = m_collisionDetectorRng->GetValue (0.0, 1.0);
              if (actualCollision)
                {
                  predictCollision = (u < m_collisionDetectorTpr);
                }
              else
                {
                  predictCollision = (u < m_collisionDetectorFpr);
                }
            }

          if (actualCollision)
            {
              ++m_collisionRapidCount;
              m_collisionUeCount += iter->second;
              std::map<uint16_t, std::vector<NprachRxMeta>>::const_iterator rxIt =
                  m_nprachRxMetaByRapid.find (rapid);
              if (rxIt != m_nprachRxMetaByRapid.end ())
                {
                  std::ostringstream oss;
                  for (std::vector<NprachRxMeta>::const_iterator e = rxIt->second.begin ();
                       e != rxIt->second.end (); ++e)
                    {
                      if (e != rxIt->second.begin ())
                        {
                          oss << ";";
                        }
                      oss << "meta=" << e->senderMetaId << "/toa=" << e->toaBin;
                    }
                  NS_LOG_INFO ("[ENB][MSG1][COLLISION] rapid=" << rapid
                               << " ra-rnti=" << ranti
                               << " ueCount=" << iter->second
                               << " participantes=[" << oss.str () << "]");
                }
              else
                {
                  NS_LOG_INFO ("[ENB][MSG1][COLLISION] rapid=" << rapid
                               << " ra-rnti=" << ranti
                               << " ueCount=" << iter->second
                               << " participantes=[sin-meta]");
                }

              if (m_mac_logging)
                {
                  std::string logfile_path = m_logdir + "MAC.log";
                  std::ofstream logfile;
                  logfile.open (logfile_path, std::ios_base::app);
                  logfile << ",PreambleCollision," << Simulator::Now ().GetMilliSeconds () << "\n";
                  logfile.close ();
                }

              m_rapIdCollisionMap[rapid] = true;
            }

          auto BuildSingleRar = [&](uint16_t useRanti, bool sharedFallback, const char *tipo)
            {
              NbIotRrcSap::Rar rar;
              rar.cellRnti = m_cmacSapUser->AllocateTemporaryCellRnti ();
              rar.rapId = rapid;
              rar.rarPayload.cellRnti = rar.cellRnti;
              rar.ceLevel = ce.coverageEnhancementLevel;
              rar.toaValid = false;
              rar.toaBin = 0;
              rar.codebookId = 0;
              rar.virtualId = 0;
              rar.sharedCollisionFallback = sharedFallback;

              m_rntiToRapId[rar.cellRnti] = rar.rapId;
              m_rarQueue.push_back (std::make_pair (useRanti, rar));
              m_RntiCeMap.insert (std::make_pair (rar.cellRnti, ce.coverageEnhancementLevel));

              NS_LOG_INFO ("[ENB][MSG2][PLAN] rapid=" << rapid
                           << " tipo=" << tipo
                           << " sharedFallback=" << (sharedFallback ? "1" : "0")
                           << " tc-rnti=" << rar.cellRnti);
            };

          if (!actualCollision)
            {
              if (schemeActive && predictCollision)
                {
                  NS_LOG_INFO ("[ENB][MSG2][DETECTOR] rapid=" << rapid
                               << " tipo=false-positive"
                               << " accion=no-colision-normal");
                }

              if (m_mac_logging)
                {
                  std::string logfile_path = m_logdir + "MAC.log";
                  std::ofstream logfile;
                  logfile.open (logfile_path, std::ios_base::app);
                  logfile << "rapid=" << rapid << ",PreambleReceived,"
                          << Simulator::Now ().GetMilliSeconds () << "\n";
                  logfile.close ();
                }

              BuildSingleRar (ranti, false, "no-colision");
              m_receivedNprachPreambleCount[subcarrierOffset].erase (iter->first);
              m_nprachRxMetaByRapid.erase (rapid);
              continue;
            }

          if (schemeActive && predictCollision && m_newSchemaActivated)
            {
              const uint8_t groupSize = std::min<uint8_t> (m_scmaMaxGroupSize, 2);
              std::vector<NprachRxMeta> selected = SelectCollisionCandidates (rapid, groupSize);
              const uint8_t selectedCount = static_cast<uint8_t> (selected.size ());
              if (selectedCount == 0)
                {
                  BuildSingleRar (ranti, true, "shared-fallback-no-meta");
                  m_receivedNprachPreambleCount[subcarrierOffset].erase (iter->first);
                  m_nprachRxMetaByRapid.erase (rapid);
                  continue;
                }

              NS_LOG_INFO ("eNB SCMA: generar " << int(selectedCount)
                          << " RAR para RAPID=" << int(rapid)
                          << " RA-RNTI=" << ranti);
              std::ostringstream sel;
              for (uint8_t n = 0; n < selectedCount; ++n)
                {
                  if (n > 0)
                    {
                      sel << ";";
                    }
                  sel << "meta=" << selected[n].senderMetaId
                      << "/toa=" << selected[n].toaBin;
                }
              NS_LOG_INFO ("[ENB][MSG2][COLLISION-SELECT] rapid=" << rapid
                           << " seleccion=[" << sel.str () << "]");

              for (uint8_t n = 0; n < selectedCount; ++n)
                {
                  const NprachRxMeta &candidate = selected[n];
                  NbIotRrcSap::Rar rar;
                  rar.cellRnti = m_cmacSapUser->AllocateTemporaryCellRnti ();
                  rar.rapId = rapid;
                  rar.rarPayload.cellRnti = rar.cellRnti;
                  rar.ceLevel = ce.coverageEnhancementLevel;
                  rar.toaValid = true;
                  rar.toaBin = candidate.toaBin;
                  rar.codebookId = static_cast<uint8_t> (n % std::max<uint8_t> (1, m_numScmaCodebooks));
                  rar.virtualId = 0;
                  rar.sharedCollisionFallback = false;

                  m_rntiToRapId[rar.cellRnti] = rar.rapId;
                  m_rarQueue.push_back (std::make_pair (candidate.ranti, rar));
                  m_RntiCeMap.insert (std::make_pair (rar.cellRnti, ce.coverageEnhancementLevel));

                  NS_LOG_INFO ("eNB RAR encolado: RAPID=" << int(rar.rapId)
                              << " TCRNTI=" << rar.cellRnti
                              << " codebook=" << int(rar.codebookId)
                              << " toaValid=" << (rar.toaValid ? "1" : "0")
                              << " toaBin=" << rar.toaBin
                              << " senderMetaId=" << candidate.senderMetaId);
                  NS_LOG_INFO ("[ENB][MSG2][PLAN] rapid=" << rapid
                               << " tipo=colision"
                               << " tc-rnti=" << rar.cellRnti
                               << " senderMetaId=" << candidate.senderMetaId
                               << " toaBin=" << rar.toaBin
                               << " codebook=" << static_cast<uint32_t> (rar.codebookId));
                }

              m_receivedNprachPreambleCount[subcarrierOffset].erase (iter->first);
              m_nprachRxMetaByRapid.erase (rapid);
              continue;
            }

          if (schemeActive && predictCollision && m_saraActivated)
            {
              const uint8_t groupSize = std::min<uint8_t> (m_saraMaxGroupSize, 2);
              NS_LOG_INFO ("[ENB][MSG2][SARA] generar " << int(groupSize)
                          << " RAR para RAPID=" << int(rapid)
                          << " RA-RNTI=" << ranti);

              for (uint8_t n = 0; n < groupSize; ++n)
                {
                  NbIotRrcSap::Rar rar;
                  rar.cellRnti = m_cmacSapUser->AllocateTemporaryCellRnti ();
                  rar.rapId = rapid;
                  rar.rarPayload.cellRnti = rar.cellRnti;
                  rar.ceLevel = ce.coverageEnhancementLevel;
                  rar.saraGroup = true;
                  rar.saraGroupSize = groupSize;
                  rar.saraTag = n;
                  rar.toaValid = false;
                  rar.toaBin = 0;
                  rar.codebookId = 0;
                  rar.virtualId = 0;
                  rar.sharedCollisionFallback = false;

                  m_rntiToRapId[rar.cellRnti] = rar.rapId;
                  m_rarQueue.push_back (std::make_pair (ranti, rar));
                  m_RntiCeMap.insert (std::make_pair (rar.cellRnti, ce.coverageEnhancementLevel));

                  NS_LOG_INFO ("[ENB][MSG2] RAR encolado: RAPID=" << int(rar.rapId)
                              << " TCRNTI=" << rar.cellRnti
                              << " SARA[group=1, size=" << int(groupSize)
                              << ", tag=" << int(n) << "]");
                }

              m_receivedNprachPreambleCount[subcarrierOffset].erase (iter->first);
              m_nprachRxMetaByRapid.erase (rapid);
              continue;
            }

          // Legacy collision or detector miss in new/SARA:
          // send one shared fallback RAR to consume resources and resolve Msg3 safely.
          BuildSingleRar (ranti, true, "shared-fallback");
          m_receivedNprachPreambleCount[subcarrierOffset].erase (iter->first);
          m_nprachRxMetaByRapid.erase (rapid);
          NS_LOG_INFO ("[ENB][MSG2][DETECTOR] rapid=" << rapid
                       << " actualCollision=1 predictedCollision=" << (predictCollision ? "1" : "0")
                       << " mode=" << (m_newSchemaActivated ? "new" : (m_saraActivated ? "sara" : "legacy")));
        }
      std::vector<NbIotRrcSap::NpdcchMessage> rar_dcis;
      if (m_rarQueue.size () > 0)
        {
          int size_mac_pdu = 0;
          size_mac_pdu += 8; // E/T/R/R/BI Header 8 bit ETSI 136.321 13.7 6.1.5-2
          int max_pdu_size_mac = 680;
          int size_mac_sdu_header = 8; // E/T/RAPID Header 8 bit ETSI 136.321 13.7 6.1.5-1
          int size_rar = 48; // MAC RAR for NB-IoT UEs 48 bit ETSI 136.321 13.7 6.1.5-3b
          while ((m_rarQueue.size () > 0))
            {
              //correccion se resetea siempre el espacio de max pdu
              size_mac_pdu = 0;
              size_mac_pdu += 8;
              // Crea un nuevo mensaje NPDCCH (DCI) para programar RARs y configurarlos
              rar_dcis.push_back (NbIotRrcSap::NpdcchMessage ());
              rar_dcis.back ().ranti = m_rarQueue.front ().first;
              rar_dcis.back ().npdcchFormat = NbIotRrcSap::NpdcchMessage::NpdcchFormat::format1;
              rar_dcis.back ().dciType = NbIotRrcSap::NpdcchMessage::DciType::n1;
              rar_dcis.back ().searchSpaceType = NbIotRrcSap::NpdcchMessage::SearchSpaceType::type2;
              rar_dcis.back ().ce = ce.coverageEnhancementLevel;
              rar_dcis.back ().isRar = true;
              rar_dcis.back ().isEdt = edt;
              rar_dcis.back ().dciN1.numNpdschSubframesPerRepetition =
                  NbIotRrcSap::DciN1::NumNpdschSubframesPerRepetition::s2;
              rar_dcis.back ().dciN1.numNpdschRepetitions =
                  NbIotRrcSap::DciN1::NumNpdschRepetitions::r2;
              // Intenta llenar el mensaje con tantos RARs como quepan en el PDU MAC
               
              while ((m_rarQueue.size () > 0) &&
                     (size_mac_pdu + size_mac_sdu_header + size_rar < max_pdu_size_mac))
                {
                  size_mac_pdu += size_mac_sdu_header;
                  size_mac_pdu += size_rar;
                  std::pair<int, NbIotRrcSap::Rar> rar = m_rarQueue.front ();
                  rar_dcis.back ().rars.push_back (rar.second);
                  m_rarQueue.erase (m_rarQueue.begin ());
                }
            }

          for (std::vector<NbIotRrcSap::NpdcchMessage>::iterator it = rar_dcis.begin ();
               it != rar_dcis.end (); ++it)
            {
              m_schedulerNb->ScheduleRarReq(*it,NbiotScheduler::ConvertNprachParametersNb2SearchSpaceConfig(ce));
            }
        }
    }
}

//esta funcion se llama solo la primera vez que entra a doSubframeIndicationNb
void LteEnbMac::SetCoverageLevelAndSib2Nb(){
  //Obtiene el SIB2 NB-IoT actual y extrae parametros de NPRACH por cada CE level
  m_sib2Nb = m_cmacSapUser->GetCurrentSystemInformationBlockType2Nb ();
  m_ce0Parameter =
      m_sib2Nb.radioResourceConfigCommon.nprachConfig.nprachParametersList.nprachParametersNb0; //contiene la configuración del canal NPRACH para NB-IoT
  m_ce1Parameter =
          m_sib2Nb.radioResourceConfigCommon.nprachConfig.nprachParametersList.nprachParametersNb1;
  m_ce2Parameter =
          m_sib2Nb.radioResourceConfigCommon.nprachConfig.nprachParametersList.nprachParametersNb2;

  // Temp variables to assign relevant edt parameters
  NbIotRrcSap::NprachParametersNb tmp; //parametros basicos de NP-RACH para cada CE 
  NbIotRrcSap::NprachParametersNbR14 tmpr14; //redefine algunos parámetros para soportar EDT 

  //Se sobrescriben los campos de tmp con los valores de tmpr14
  // EDT CE0
  tmp = m_sib2Nb.radioResourceConfigCommon.nprachConfig.nprachParametersList.nprachParametersNb0;
  tmpr14 = m_sib2Nb.radioResourceConfigCommon.nprachConfigR15.nprachParameterListEdt.nprachParametersNb0; ////contiene la configuración del canal NPRACH para NB-IoT con mejoras EDT
  tmp.coverageEnhancementLevel= tmpr14.coverageEnhancementLevel;
  tmp.nprachPeriodicity = tmpr14.nprachPeriodicity; 
  tmp.nprachStartTime = tmpr14.nprachStartTime;
  tmp.nprachSubcarrierOffset = tmpr14.nprachSubcarrierOffset;
  tmp.nprachNumSubcarriers = tmpr14.nprachNumSubcarriers;
  tmp.nprachSubcarrierMsg3RangeStart= tmpr14.nprachSubcarrierMsg3RangeStart;
  tmp.npdcchNumRepetitionsRA = tmpr14.npdcchNumRepetitionsRA;
  tmp.npdcchStartSfCssRa = tmpr14.npdcchStartSfCssRa;
  tmp.npdcchOffsetRa = tmpr14.npdcchOffsetRa;

  m_ce0ParameterEdt = tmp;
 
  // EDT CE1
  tmp = m_sib2Nb.radioResourceConfigCommon.nprachConfig.nprachParametersList.nprachParametersNb1;
  tmpr14 = m_sib2Nb.radioResourceConfigCommon.nprachConfigR15.nprachParameterListEdt.nprachParametersNb1;
  tmp.coverageEnhancementLevel= tmpr14.coverageEnhancementLevel;
  tmp.nprachPeriodicity = tmpr14.nprachPeriodicity; 
  tmp.nprachStartTime = tmpr14.nprachStartTime;
  tmp.nprachSubcarrierOffset = tmpr14.nprachSubcarrierOffset;
  tmp.nprachNumSubcarriers = tmpr14.nprachNumSubcarriers;
  tmp.nprachSubcarrierMsg3RangeStart= tmpr14.nprachSubcarrierMsg3RangeStart;
  tmp.npdcchNumRepetitionsRA = tmpr14.npdcchNumRepetitionsRA;
  tmp.npdcchStartSfCssRa = tmpr14.npdcchStartSfCssRa;
  tmp.npdcchOffsetRa = tmpr14.npdcchOffsetRa;     

  m_ce1ParameterEdt = tmp;
 
  // EDT CE2
  tmp = m_sib2Nb.radioResourceConfigCommon.nprachConfig.nprachParametersList.nprachParametersNb2;
  tmpr14 = m_sib2Nb.radioResourceConfigCommon.nprachConfigR15.nprachParameterListEdt.nprachParametersNb2;
  tmp.coverageEnhancementLevel= tmpr14.coverageEnhancementLevel;
  tmp.nprachPeriodicity = tmpr14.nprachPeriodicity; 
  tmp.nprachStartTime = tmpr14.nprachStartTime;
  tmp.nprachSubcarrierOffset = tmpr14.nprachSubcarrierOffset;
  tmp.nprachNumSubcarriers = tmpr14.nprachNumSubcarriers;
  tmp.nprachSubcarrierMsg3RangeStart= tmpr14.nprachSubcarrierMsg3RangeStart;
  tmp.npdcchNumRepetitionsRA = tmpr14.npdcchNumRepetitionsRA;
  tmp.npdcchStartSfCssRa = tmpr14.npdcchStartSfCssRa;
  tmp.npdcchOffsetRa = tmpr14.npdcchOffsetRa;     

  m_ce2ParameterEdt = tmp;
//tmp queda con los valores más actualizados y relevantes para EDT.
}

void
// ===== paso 2 =====
LteEnbMac::DoSubframeIndicationNb (uint32_t frameNo, uint32_t subframeNo)
{
  NS_LOG_FUNCTION (this << " EnbMac - frame " << frameNo << " subframe " << subframeNo);
  m_edt = true; // PASCAL: Warum immer true? Was, wenn EDT als false definiert wurde?
  m_frameNo = frameNo;
  m_subframeNo = subframeNo;
  if (m_schedulerNb == nullptr) //no hay schedulerNb
    {
      // ====== se registra el tiempo, se inicializa el scheduler NB-IoT
      SetCoverageLevelAndSib2Nb();
      // ====== creando un objeto NbiotScheduler ======
      // ======  las primeras veces, usando los parámetros NPrach de SIB2. ======
      m_schedulerNb = new NbiotScheduler (
          std::vector<NbIotRrcSap::NprachParametersNb>{m_ce0Parameter, m_ce1Parameter,
                                                       m_ce2Parameter},
          m_sib2Nb,
          m_newSchemaActivated,
          m_numScmaCodebooks);
      m_schedulerNb->SetLogDir(m_logdir);
    }

  const uint64_t nowSubframe = static_cast<uint64_t> (10 * (m_frameNo - 1) + (m_subframeNo - 1));
  CleanupExpiredSharedFallbackContexts (nowSubframe);

  // Implement NB-IoT DCI Searchspaces Type2-CSS All AL2  Liberg et al. p 282
  // Find out if current subframe is start of Type2/UE-specific search space
  // A Tutorial to NB-IoT Design zeugs

  // ====== Paso 3 ======
  // ====== Comprobamos que preambulos NPRACH validos ha recibido ======
  CheckPreambleReceptionForAllCoverageClases();

  // ====== prepara al scheduler NB pasándole la información de RSRP ======
  //m_schedulerNb->SetCeLevel (m_ce0Parameter, m_ce1Parameter, m_ce2Parameter);
  m_schedulerNb->SetRntiRsrpMap (m_ulRsrpReceivedNb);

  // ====== Se pide al scheduler que se programe DL en la subtrama actual
  std::vector<NbIotRrcSap::NpdcchMessage> scheduled = m_schedulerNb->Schedule (frameNo, subframeNo);

  int currentsubframe = static_cast<int> (nowSubframe);
  std::map<int16_t, bool> contention_resolution;
  for (std::vector<NbIotRrcSap::NpdcchMessage>::iterator it = scheduled.begin ();
       it != scheduled.end (); ++it)
    {

      if (it->isRar)
        {
          Ptr<RarNbiotControlMessage> msg = Create<RarNbiotControlMessage> ();
          int subframestowait = *(it->dciN1.npdschOpportunity.end () - 1) - currentsubframe;
          for (std::vector<NbIotRrcSap::Rar>::iterator rar = it->rars.begin ();
               rar != it->rars.end (); ++rar)
            {
              if (!rar->rarPayload.ulGrant.subframes.second.empty ())
                {
                  m_rntiMsg3WindowEnd[rar->cellRnti] =
                    rar->rarPayload.ulGrant.subframes.second.back ();
                }
              NS_LOG_INFO ("eNB RAR tx prepared: RA-RNTI=" << it->ranti
                          << " RAPID=" << static_cast<uint32_t> (rar->rapId)
                          << " C-RNTI=" << rar->cellRnti
                          << " virtualId=" << rar->virtualId
                          << " codebook=" << static_cast<uint32_t> (rar->codebookId)
                          << " toaValid=" << (rar->toaValid ? "1" : "0")
                          << " toaBin=" << rar->toaBin
                          << " grantPhysical=" << static_cast<uint32_t> (rar->rarPayload.ulGrant.subframes.first));
              ++m_msg2TxCount;
              NS_LOG_INFO ("[ENB][MSG2][TX] ra-rnti=" << it->ranti
                           << " rapid=" << static_cast<uint32_t> (rar->rapId)
                           << " tc-rnti=" << rar->cellRnti
                           << " sharedFallback=" << (rar->sharedCollisionFallback ? "1" : "0")
                           << " toaValid=" << (rar->toaValid ? "1" : "0")
                           << " toaBin=" << rar->toaBin
                           << " virtualId=" << rar->virtualId
                           << " codebook=" << static_cast<uint32_t> (rar->codebookId)
                           << " physicalCarrier="
                           << static_cast<uint32_t> (rar->rarPayload.ulGrant.subframes.first));

              if (rar->sharedCollisionFallback)
                {
                  SharedFallbackMsg3Context ctx;
                  ctx.firstAccepted = false;
                  ctx.windowEndSf = !rar->rarPayload.ulGrant.subframes.second.empty ()
                                      ? rar->rarPayload.ulGrant.subframes.second.back ()
                                      : static_cast<uint64_t> (currentsubframe);
                  m_sharedFallbackMsg3ByTcRnti[rar->cellRnti] = ctx;
                  m_sharedFallbackBlockedRnti.insert (rar->cellRnti);
                }
              else if (m_newSchemaActivated)
                {
                  ScmaMsg3ExpectedContext expected;
                  expected.tcRnti = rar->cellRnti;
                  expected.virtualId = rar->virtualId;
                  expected.codebookId = rar->codebookId;
                  expected.physicalCarrier = rar->rarPayload.ulGrant.subframes.first;
                  expected.rapid = rar->rapId;
                  expected.toaValid = rar->toaValid;
                  expected.toaBin = rar->toaBin;
                  m_expectedScmaMsg3ByTcRnti[expected.tcRnti] = expected;
                }

              // Arm Msg3 watchdog when this RAR is actually transmitted.
              Simulator::Schedule (MilliSeconds (subframestowait),
                                   &LteEnbCmacSapUser::NotifyRaResponseTransmitted,
                                   m_cmacSapUser, rar->cellRnti);
              msg->AddRar (*rar);
            }
          msg->SetRaRnti (it->ranti);
          m_connectionSuccessful[it->rnti] = false;
          Simulator::Schedule (MilliSeconds (subframestowait),
                               &LteEnbPhySapProvider::SendLteControlMessage, m_enbPhySapProvider,
                               msg);
        }
      else
        {
          if (it->dciType == NbIotRrcSap::NpdcchMessage::DciType::n1)
            {
              if (!contention_resolution[it->rnti])
                {
                  int subframestowait = *(it->dciN1.npdschOpportunity.end () - 1) - currentsubframe;
                  std::map<uint16_t, std::map<uint8_t, LteMacSapUser *>>::iterator rntiIt =
                      m_rlcAttached.find (it->rnti);
                  NS_ASSERT_MSG (rntiIt != m_rlcAttached.end (), "could not find RNTI" << it->rnti);
                  //NS_LOG_DEBUG (this << " rnti= " << rnti << " lcid= " << (uint32_t) lcid << " layer= " << k);
                  std::vector<uint8_t> activeLcs;
                  std::map<uint8_t, LteMacSapProvider::ReportBufferStatusParameters>::iterator itBsr;
                  for (itBsr = m_lastDlBSR[it->rnti].begin (); itBsr != m_lastDlBSR[it->rnti].end (); itBsr++)
                  {
                    if (((*itBsr).second.statusPduSize > 0) || ((*itBsr).second.retxQueueSize > 0) ||
                        ((*itBsr).second.txQueueSize > 0))
                      {
                        activeLcs.push_back(itBsr->first);
                      }
                  }
                  LteMacSapUser::TxOpportunityParameters txOpParams;
                  // Prioritise SRBs over DataBs
                  uint64_t bytesforallLc = it->tbs/8; //TBS del NPDSCH decidido por el scheduler (bits). Se pasa a bytes
                  for(std::vector<uint8_t>::iterator lcit = activeLcs.begin(); lcit != activeLcs.end(); ++lcit){
                    std::map<uint8_t, LteMacSapProvider::ReportBufferStatusParameters>::iterator bsr = m_lastDlBSR[it->rnti].find((*lcit));
                    std::map<uint8_t, LteMacSapUser *>::iterator lcidIt = rntiIt->second.find (bsr->second.lcid);
                    if ((bsr->second.statusPduSize > 0) &&
                            (bytesforallLc >= bsr->second.statusPduSize))
                      {
                        txOpParams.bytes = bsr->second.statusPduSize;
                        txOpParams.layer = 0;
                        txOpParams.harqId = 0;
                        txOpParams.componentCarrierId = m_componentCarrierId;
                        txOpParams.rnti = bsr->second.rnti;
                        txOpParams.lcid = bsr->second.lcid;
                        (*lcidIt).second->NotifyTxOpportunityNb(txOpParams, subframestowait);
                        //Simulator::Schedule (MilliSeconds (subframestowait), &LteMacSapUser::NotifyTxOpportunity,
                        //   (*lcidIt).second, txOpParams);
                        bytesforallLc -= bsr->second.statusPduSize;
                        bsr->second.statusPduSize = 0;
                      }
                    else
                      {
                        if (bsr->second.statusPduSize > bytesforallLc)
                          {
                            NS_FATAL_ERROR (
                                "Insufficient Tx Opportunity for sending a status message");
                          }
                      }

                    if ((bytesforallLc> 7) // 7 is the min TxOpportunity useful for Rlc
                        && ((bsr->second.retxQueueSize > 0) ||
                            (bsr->second.txQueueSize > 0)))
                      {
                        if (bsr->second.retxQueueSize > 0)
                          {
                            NS_LOG_DEBUG (this << " serve retx DATA, bytes " << bytesforallLc);
                            if(bsr->second.retxQueueSize > bytesforallLc){
                              txOpParams.bytes = bytesforallLc;
                              bsr->second.retxQueueSize -= bytesforallLc;
                              bytesforallLc = 0;
                            }else{
                              if(bsr->second.retxQueueSize +4 < 7){
                                txOpParams.bytes = 7;
                                bytesforallLc -= 7;
                              }else{
                                txOpParams.bytes = bsr->second.retxQueueSize+4;
                                bytesforallLc -= bsr->second.retxQueueSize+4;
                              }
                                bsr->second.retxQueueSize = 0;
                            }
                            txOpParams.layer = 0;
                            txOpParams.harqId = 0;
                            txOpParams.componentCarrierId = m_componentCarrierId;
                            txOpParams.rnti = bsr->second.rnti;
                            txOpParams.lcid = bsr->second.lcid;
                            //Simulator::Schedule (MilliSeconds (subframestowait), &LteMacSapUser::NotifyTxOpportunity,
                            //  (*lcidIt).second, txOpParams);
                            (*lcidIt).second->NotifyTxOpportunityNb(txOpParams, subframestowait);
                          }
                        else if (bsr->second.txQueueSize > 0)
                          {

                            if(bsr->second.txQueueSize > bytesforallLc){
                              txOpParams.bytes = bytesforallLc;
                              bsr->second.txQueueSize -= bytesforallLc;
                              bytesforallLc = 0;
                            }else{
                              if(bsr->second.txQueueSize+4 < 7){
                                txOpParams.bytes = 7;
                                bytesforallLc -= 7;
                              }else{
                                txOpParams.bytes = bsr->second.txQueueSize+4;
                                bytesforallLc -= bsr->second.txQueueSize+4;
                              }

                              bsr->second.txQueueSize = 0;
                            }
                            txOpParams.layer = 0;
                            txOpParams.harqId = 0;
                            txOpParams.componentCarrierId = m_componentCarrierId;
                            txOpParams.rnti = bsr->second.rnti;
                            txOpParams.lcid = bsr->second.lcid;

                            //Simulator::Schedule (MilliSeconds (subframestowait), &LteMacSapUser::NotifyTxOpportunity,
                            //  (*lcidIt).second, txOpParams);
                            (*lcidIt).second->NotifyTxOpportunityNb(txOpParams,subframestowait);
                            
                          }
                          }
                        else
                          {
                            if ((bsr->second.retxQueueSize > 0) ||
                                (bsr->second.txQueueSize > 0))
                              {
                                //NS_BUILD_DEBUG(std::cout << "Not enough space" << std::endl);
                              }
                          }
                  }
                  std::vector<uint8_t> remaining;
                  for (itBsr = m_lastDlBSR[it->rnti].begin (); itBsr != m_lastDlBSR[it->rnti].end (); itBsr++)
                  {
                    if (((*itBsr).second.statusPduSize > 0) || ((*itBsr).second.retxQueueSize > 0) ||
                        ((*itBsr).second.txQueueSize > 0))
                      {
                        remaining.push_back(itBsr->first);
                      }
                  }
                  if(remaining.size()>0){
                    m_schedulerNb->ScheduleDlRlcBufferReq(it->rnti,m_lastDlBSR[it->rnti]);
                  }
                }
            }
        }
        //Emision del NPDCCH(control) timers y timers de inactividad
        //siempre despues de dar las TxOpportunity
      if (!contention_resolution[it->rnti])
        {
          if (it->dciType == NbIotRrcSap::NpdcchMessage::DciType::n1)
            {

              Ptr<DlDciN1NbiotControlMessage> msg = Create<DlDciN1NbiotControlMessage> ();
              msg->SetDci (it->dciN1);
              msg->SetRnti (it->rnti);
              int subframestowait = *(it->dciRepetitionsubframes.end () - 1) - currentsubframe;
              Simulator::Schedule (MilliSeconds (subframestowait),
                                   &LteEnbPhySapProvider::SendLteControlMessage,
                                   m_enbPhySapProvider, msg);

              // Implement DataInactivity-Timer 
              // Notify RRC about last scheduled NPDSCH Transmission for the rnti
              if(it->rnti != 0){ 
                int subframestillDataInactivity = it->dciN1.npdschOpportunity.back()- currentsubframe;
                m_cmacSapUser->NotifyDataActivitySchedulerNb(it->rnti);

                if(!m_noDataIndicators[it->rnti].IsExpired()){
                  m_noDataIndicators[it->rnti].Cancel();
                }
                m_noDataIndicators[it->rnti] = Simulator::Schedule (MilliSeconds (subframestillDataInactivity),
                                    &LteEnbCmacSapUser::NotifyDataInactivitySchedulerNb,
                                    m_cmacSapUser, it->rnti);
              }

            }
          else if (it->dciType == NbIotRrcSap::NpdcchMessage::DciType::n0){
            Ptr<UlDciN0NbiotControlMessage> msg = Create<UlDciN0NbiotControlMessage> ();
            msg->SetDci(it->dciN0);
            msg->SetRnti(it->rnti);
            int subframestowait = *(it->dciRepetitionsubframes.end () - 1) - currentsubframe;
            Simulator::Schedule (MilliSeconds (subframestowait),
                                  &LteEnbPhySapProvider::SendLteControlMessage,
                                  m_enbPhySapProvider, msg);
            // Implement DataInactivity-Timer 
            // Notify RRC about last scheduled NPDSCH Transmission for the rnti
            if(it->rnti != 0){
              int subframestillDataInactivity = it->dciN0.npuschOpportunity.back().second.back() - currentsubframe;
              m_cmacSapUser->NotifyDataActivitySchedulerNb(it->rnti);
              if(!m_noDataIndicators[it->rnti].IsExpired()){
                  m_noDataIndicators[it->rnti].Cancel();
              }
              m_noDataIndicators[it->rnti] = Simulator::Schedule (MilliSeconds (subframestillDataInactivity),
                                  &LteEnbCmacSapUser::NotifyDataInactivitySchedulerNb,
                                  m_cmacSapUser, it->rnti);
            }
          }
        }
    }
    scheduled.clear();
}

void
LteEnbMac::DoReceiveLteControlMessage (Ptr<LteControlMessage> msg)
{
  NS_LOG_FUNCTION (this << msg);
  if (msg->GetMessageType () == LteControlMessage::DL_CQI)
    {
      Ptr<DlCqiLteControlMessage> dlcqi = DynamicCast<DlCqiLteControlMessage> (msg);
      ReceiveDlCqiLteControlMessage (dlcqi);
    }
  else if (msg->GetMessageType () == LteControlMessage::BSR)
    {
      Ptr<BsrLteControlMessage> bsr = DynamicCast<BsrLteControlMessage> (msg);
      ReceiveBsrMessage (bsr->GetBsr ());
    }
  else if (msg->GetMessageType () == LteControlMessage::DL_HARQ)
    {
      Ptr<DlHarqFeedbackLteControlMessage> dlharq =
          DynamicCast<DlHarqFeedbackLteControlMessage> (msg);
      DoDlInfoListElementHarqFeeback (dlharq->GetDlHarqFeedback ());
    }
  else if (msg->GetMessageType () == LteControlMessage::DL_HARQ_NB)
    {
      Ptr<DlHarqFeedbackNbiotControlMessage> dlharq =
          DynamicCast<DlHarqFeedbackNbiotControlMessage> (msg);
      // If connectionSuccessful == false, device hasnt completed its Connection yet
      // Device has received MSG4 and needs UL-Resources for MSG5.
      uint16_t tempRnti = dlharq->GetRnti ();
      std::map<uint16_t, std::deque<uint16_t> >::iterator mapIt = m_tempRntiToDefRnti.find (tempRnti);
      bool hasSaraQueue = (mapIt != m_tempRntiToDefRnti.end ()) && (!mapIt->second.empty ());

      if (hasSaraQueue || !m_connectionSuccessful[tempRnti])
        {
          uint16_t grantRnti = tempRnti;
          if (hasSaraQueue)
            {
              grantRnti = mapIt->second.front ();
              mapIt->second.pop_front ();
              if (mapIt->second.empty ())
                {
                  m_tempRntiToDefRnti.erase (mapIt);
                }
            }

          uint16_t bsr = m_ueStoredBSR[tempRnti];
          m_schedulerNb->ScheduleUlRlcBufferReq (grantRnti, bsr);

          if (!hasSaraQueue)
            {
              m_connectionSuccessful[tempRnti] = true;
              m_ueStoredBSR[tempRnti] = 0;
            }
          else if (m_tempRntiToDefRnti.find (tempRnti) == m_tempRntiToDefRnti.end ())
            {
              m_connectionSuccessful[tempRnti] = true;
              m_ueStoredBSR[tempRnti] = 0;
            }
        }
    }
  else
    {
      NS_LOG_LOGIC (this << " LteControlMessage type " << msg->GetMessageType ()
                         << " not recognized");
    }
}

void
LteEnbMac::DoReceiveRachPreamble (uint8_t rapId)
{
  NS_LOG_FUNCTION (this << (uint32_t) rapId);
  // just record that the preamble has been received; it will be processed later
  ++m_receivedRachPreambleCount[rapId]; // will create entry if not exists
}

uint16_t
LteEnbMac::EstimateToaBinFromSender (uint32_t senderMetaId) const
{
  return NbIotToaUtils::ComputeToaBin (senderMetaId, m_toaNumBins);
}

std::vector<LteEnbMac::NprachRxMeta>
LteEnbMac::SelectCollisionCandidates (uint16_t rapid, uint8_t maxCandidates) const
{
  std::vector<NprachRxMeta> selected;
  if (maxCandidates == 0)
    {
      return selected;
    }

  std::map<uint16_t, std::vector<NprachRxMeta>>::const_iterator it =
      m_nprachRxMetaByRapid.find (rapid);
  if (it == m_nprachRxMetaByRapid.end ())
    {
      return selected;
    }

  std::vector<NprachRxMeta> entries = it->second;
  std::sort (entries.begin (), entries.end (),
             [] (const NprachRxMeta &a, const NprachRxMeta &b)
             {
               if (a.toaBin != b.toaBin)
                 {
                   return a.toaBin < b.toaBin;
                 }
               if (a.senderMetaId != b.senderMetaId)
                 {
                   return a.senderMetaId < b.senderMetaId;
                 }
               return a.ranti < b.ranti;
             });

  // Keep one candidate per sender to avoid selecting the same UE twice.
  std::vector<NprachRxMeta> uniqueEntries;
  for (std::vector<NprachRxMeta>::const_iterator e = entries.begin (); e != entries.end (); ++e)
    {
      bool alreadySeenSender = false;
      for (std::vector<NprachRxMeta>::const_iterator u = uniqueEntries.begin ();
           u != uniqueEntries.end (); ++u)
        {
          if (u->senderMetaId == e->senderMetaId)
            {
              alreadySeenSender = true;
              break;
            }
        }
      if (!alreadySeenSender)
        {
          uniqueEntries.push_back (*e);
        }
    }

  if (uniqueEntries.size () <= maxCandidates)
    {
      return uniqueEntries;
    }

  if (maxCandidates >= 2 && uniqueEntries.size () >= 2)
    {
      // NEW phase A: pick the pair with maximum ToA-bin separation.
      bool foundPair = false;
      uint16_t bestDiff = 0;
      uint32_t bestMinMeta = 0;
      uint32_t bestMaxMeta = 0;
      uint16_t bestMinToa = 0;
      uint16_t bestMaxToa = 0;
      size_t bestI = 0;
      size_t bestJ = 1;

      for (size_t i = 0; i < uniqueEntries.size (); ++i)
        {
          for (size_t j = i + 1; j < uniqueEntries.size (); ++j)
            {
              const NprachRxMeta &a = uniqueEntries[i];
              const NprachRxMeta &b = uniqueEntries[j];
              const uint16_t diff = (a.toaBin >= b.toaBin) ? (a.toaBin - b.toaBin)
                                                           : (b.toaBin - a.toaBin);

              const uint32_t minMeta = std::min (a.senderMetaId, b.senderMetaId);
              const uint32_t maxMeta = std::max (a.senderMetaId, b.senderMetaId);
              const uint16_t minToa = std::min (a.toaBin, b.toaBin);
              const uint16_t maxToa = std::max (a.toaBin, b.toaBin);

              bool better = false;
              if (!foundPair || (diff > bestDiff))
                {
                  better = true;
                }
              else if (diff == bestDiff)
                {
                  // Deterministic tie-break among equally-separated pairs.
                  if (minMeta < bestMinMeta)
                    {
                      better = true;
                    }
                  else if ((minMeta == bestMinMeta) && (maxMeta < bestMaxMeta))
                    {
                      better = true;
                    }
                  else if ((minMeta == bestMinMeta) && (maxMeta == bestMaxMeta) &&
                           (minToa < bestMinToa))
                    {
                      better = true;
                    }
                  else if ((minMeta == bestMinMeta) && (maxMeta == bestMaxMeta) &&
                           (minToa == bestMinToa) && (maxToa < bestMaxToa))
                    {
                      better = true;
                    }
                }

              if (better)
                {
                  foundPair = true;
                  bestDiff = diff;
                  bestMinMeta = minMeta;
                  bestMaxMeta = maxMeta;
                  bestMinToa = minToa;
                  bestMaxToa = maxToa;
                  bestI = i;
                  bestJ = j;
                }
            }
        }

      if (foundPair)
        {
          selected.push_back (uniqueEntries[bestI]);
          selected.push_back (uniqueEntries[bestJ]);
        }
    }

  // Fill additional slots (if any) with remaining distinct senders deterministically.
  for (std::vector<NprachRxMeta>::const_iterator e = uniqueEntries.begin ();
       e != uniqueEntries.end () && selected.size () < maxCandidates; ++e)
    {
      bool senderAlreadySelected = false;
      for (std::vector<NprachRxMeta>::const_iterator s = selected.begin (); s != selected.end (); ++s)
        {
          senderAlreadySelected = senderAlreadySelected || (s->senderMetaId == e->senderMetaId);
        }
      if (!senderAlreadySelected)
        {
          selected.push_back (*e);
        }
    }

  return selected;
}

//  ===== paso 1 RA recibe preambulo y aumenta contador para esa subtrama y rapId =====
void
LteEnbMac::DoReceiveNprachPreamble (uint8_t rapId, uint8_t subcarrierOffset, uint32_t ranti,
                                    uint32_t senderMetaId)
{
  // ====== registra el preámbulo recibido: incrementa el contador correspondiente ======
  NS_LOG_FUNCTION (this << (uint32_t) rapId);
  // Sólo constancia de que se ha recibido el preámbulo; se procesará más adelante.

  // ====== guarda en una matriz LOGICA cuántas veces recibe un preámbulo por subportadora (intento de acceso) =====
  ++(m_receivedNprachPreambleCount[subcarrierOffset][rapId]); // creará una entrada si no existe
 
  // ====== asocia el par (subportadora + rapId) con el RNTI del UE en un vector clave valor ======
  // ====== para crear una clave unica que permite distinguir entre preámbulos similares en distintas subportadoras ======
  const uint16_t rapid = static_cast<uint16_t> (subcarrierOffset + rapId);
  m_rapIdRantiMap[rapid] = ranti;

  NprachRxMeta rx;
  rx.rapid = rapid;
  rx.ranti = ranti;
  rx.senderMetaId = senderMetaId;
  rx.toaBin = EstimateToaBinFromSender (senderMetaId);
  rx.subcarrierOffset = subcarrierOffset;
  m_nprachRxMetaByRapid[rapid].push_back (rx);
  ++m_msg1RxCount;

  NS_LOG_INFO ("[ENB][MSG1][RX] rapid=" << rapid
               << " ra-rnti=" << ranti
               << " preamble=" << static_cast<uint32_t> (rapId)
               << " subcarrierOffset=" << static_cast<uint32_t> (subcarrierOffset)
               << " senderMetaId=" << senderMetaId
               << " toaBin=" << rx.toaBin);
}

void
LteEnbMac::DoUlCqiReport (FfMacSchedSapProvider::SchedUlCqiInfoReqParameters ulcqi)
{
  if (ulcqi.m_ulCqi.m_type == UlCqi_s::PUSCH)
    {
      NS_LOG_DEBUG (this << " eNB rxed an PUSCH UL-CQI");
    }
  else if (ulcqi.m_ulCqi.m_type == UlCqi_s::SRS)
    {
      NS_LOG_DEBUG (this << " eNB rxed an SRS UL-CQI");
    }
  m_ulCqiReceived.push_back (ulcqi);
}

void
LteEnbMac::DoUlCqiReportNb (std::vector<double> cqi)
{
  //NS_BUILD_DEBUG (std::cout << "Received CQI: ");
  for (std::vector<double>::iterator it = cqi.begin (); it != cqi.end (); ++it)
    {
      //NS_BUILD_DEBUG (std::cout << *it << " ");
    }
  //NS_BUILD_DEBUG (std::cout << std::endl);
  m_ulCqiReceivedNb.push_back (cqi);
}

void
LteEnbMac::ReceiveDlCqiLteControlMessage (Ptr<DlCqiLteControlMessage> msg)
{
  NS_LOG_FUNCTION (this << msg);

  CqiListElement_s dlcqi = msg->GetDlCqi ();
  //
  NS_LOG_LOGIC (this << "Enb Received DL-CQI rnti" << dlcqi.m_rnti);
  NS_ASSERT (dlcqi.m_rnti != 0);
  m_ulRsrpReceivedNb.insert (std::pair<uint16_t, double> (dlcqi.m_rnti, msg->rsrp));
  //m_dlCqiReceived.push_back (dlcqi);
}

void
LteEnbMac::ReceiveBsrMessage (MacCeListElement_s bsr)
{
  NS_LOG_FUNCTION (this);
  m_ccmMacSapUser->UlReceiveMacCe (bsr, m_componentCarrierId);
}

void
LteEnbMac::DoReportMacCeToScheduler (MacCeListElement_s bsr)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG (this << " bsr Size " << (uint16_t) m_ulCeReceived.size ());
  //send to LteCcmMacSapUser
  m_ulCeReceived.push_back (
      bsr); // this to called when LteUlCcmSapProvider::ReportMacCeToScheduler is called
  NS_LOG_DEBUG (this << " bsr Size after push_back " << (uint16_t) m_ulCeReceived.size ());
}

bool
LteEnbMac::ForwardMsg3ToRlc (Ptr<Packet> p, uint16_t rnti, uint8_t lcid)
{
  uint64_t msg3Imsi = 0;
  bool isSara = false;
  uint8_t saraCodebook = 0;
  NbIotMsg3ImsiTag msg3ImsiTag;
  if (p->PeekPacketTag (msg3ImsiTag))
    {
      msg3Imsi = msg3ImsiTag.GetImsi ();
    }
  SaraUlIdTag saraTag;
  if (p->PeekPacketTag (saraTag))
    {
      uint8_t cd = 0;
      uint8_t dmrs = 0;
      saraTag.Get (cd, dmrs);
      (void) dmrs;
      isSara = true;
      saraCodebook = cd;
    }

  std::map<uint16_t, std::map<uint8_t, LteMacSapUser *>>::iterator rntiIt =
    m_rlcAttached.find (rnti);
  if (rntiIt == m_rlcAttached.end ())
    {
      NS_LOG_WARN ("Drop stale Msg3 for unknown RNTI=" << rnti
                   << " lcid=" << static_cast<uint32_t> (lcid));
      if (SaraReport::IsEnabled ())
        {
          SaraReport::LogMsg3Forward (rnti, msg3Imsi, lcid, isSara, saraCodebook, 0, 1);
        }
      return false;
    }
  std::map<uint8_t, LteMacSapUser *>::iterator lcidIt =
    rntiIt->second.find (lcid);

  LteMacSapUser::ReceivePduParameters rxPduParams;
  rxPduParams.p = p;
  rxPduParams.rnti = rnti;
  rxPduParams.lcid = lcid;

  if (lcidIt != rntiIt->second.end ())
    {
      (*lcidIt).second->ReceivePdu (rxPduParams);
      if (SaraReport::IsEnabled ())
        {
          SaraReport::LogMsg3Forward (rnti, msg3Imsi, lcid, isSara, saraCodebook, 1, 0);
        }
      return true;
    }
  if (SaraReport::IsEnabled ())
    {
      SaraReport::LogMsg3Forward (rnti, msg3Imsi, lcid, isSara, saraCodebook, 0, 2);
    }
  return false;
}

void
LteEnbMac::ResolveMsg3Window (uint16_t rnti, uint64_t windowEnd)
{
  const std::pair<uint16_t, uint64_t> key = std::make_pair (rnti, windowEnd);
  std::map<std::pair<uint16_t, uint64_t>, Msg3Buffer>::iterator it =
    m_msg3Buffers.find (key);
  if (it == m_msg3Buffers.end ())
    {
      return;
    }

  std::map<uint8_t, uint32_t> counts;
  for (std::vector<Msg3BufferEntry>::const_iterator e = it->second.entries.begin ();
       e != it->second.entries.end (); ++e)
    {
      counts[e->codebook] += 1;
    }

  uint32_t acceptedTotal = 0;
  for (std::vector<Msg3BufferEntry>::const_iterator e = it->second.entries.begin ();
       e != it->second.entries.end (); ++e)
    {
      if (counts[e->codebook] == 1)
        {
          ++acceptedTotal;
        }
    }

  uint32_t remainingAccepted = acceptedTotal;
  for (std::vector<Msg3BufferEntry>::const_iterator e = it->second.entries.begin ();
       e != it->second.entries.end (); ++e)
    {
      uint64_t msg3Imsi = 0;
      NbIotMsg3ImsiTag msg3ImsiTag;
      if (e->p->PeekPacketTag (msg3ImsiTag))
        {
          msg3Imsi = msg3ImsiTag.GetImsi ();
        }

      if (counts[e->codebook] > 1)
        {
          NS_LOG_INFO ("[ENB][MSG3][SARA-COLLISION] TC-RNTI="
                       << rnti
                       << " cd=" << (uint32_t) e->codebook
                       << " count=" << counts[e->codebook]);
          if (SaraReport::IsEnabled ())
            {
              SaraReport::LogMsg3Separated (rnti, msg3Imsi, e->codebook, false);
            }
          continue;
        }

      NS_LOG_INFO ("[ENB][MSG3][SARA-SEPARATED] TC-RNTI="
                   << rnti
                   << " cd=" << (uint32_t) e->codebook
                   << " accepted");
      if (SaraReport::IsEnabled ())
        {
          SaraReport::LogMsg3Separated (rnti, msg3Imsi, e->codebook, true);
        }
      if (remainingAccepted > 0)
        {
          bool isLast = (remainingAccepted == 1);
          SaraMsg3GroupTag groupTag;
          groupTag.Set (rnti, windowEnd, isLast);
          e->p->AddPacketTag (groupTag);
          --remainingAccepted;
        }
      if (ForwardMsg3ToRlc (e->p, rnti, e->lcid))
        {
          if (SaraReport::IsEnabled ())
            {
              SaraReport::LogMsg3ToRrc (rnti, msg3Imsi, e->lcid, true, e->codebook);
            }
          ++m_msg3AcceptedCount;
        }
    }

  m_msg3Buffers.erase (it);
  m_rntiMsg3WindowEnd.erase (rnti);
}

void
LteEnbMac::CleanupExpiredSharedFallbackContexts (uint64_t nowSubframe)
{
  std::vector<uint16_t> rntisToCleanup;
  for (std::map<uint16_t, SharedFallbackMsg3Context>::iterator it =
         m_sharedFallbackMsg3ByTcRnti.begin ();
       it != m_sharedFallbackMsg3ByTcRnti.end ();)
    {
      if (nowSubframe > (it->second.windowEndSf + 1))
        {
          const uint16_t rnti = it->first;
          NS_LOG_INFO ("[ENB][MSG3][CTX-CLEANUP] tc-rnti=" << it->first
                       << " reason=shared-fallback-window-expired"
                       << " firstAccepted=" << (it->second.firstAccepted ? "1" : "0")
                       << " windowEnd=" << it->second.windowEndSf
                       << " now=" << nowSubframe);
          m_sharedFallbackBlockedRnti.erase (rnti);
          rntisToCleanup.push_back (rnti);
          it = m_sharedFallbackMsg3ByTcRnti.erase (it);
        }
      else
        {
          ++it;
        }
    }

  for (std::vector<uint16_t>::const_iterator rnti = rntisToCleanup.begin ();
       rnti != rntisToCleanup.end (); ++rnti)
    {
      m_cmacSapUser->NotifySharedFallbackWindowClosed (*rnti);
    }
}

void
LteEnbMac::DoReceivePhyPdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this);
  LteRadioBearerTag tag;
  p->RemovePacketTag (tag);
  uint16_t rnti = tag.GetRnti ();
  uint8_t lcid = tag.GetLcid ();

  DataVolumeAndPowerHeadroomTag dprTag;
  BufferStatusReportTag bsrTag;
  uint32_t buffersize = 0;

  if (p->RemovePacketTag (dprTag))
    { // it's MSG3
      buffersize = DataVolumeDPR::DVId2BufferSize (dprTag.GetDataVolumeValue ());
      m_ueStoredBSR[rnti] = buffersize;
      uint64_t msg3Imsi = 0;
      NbIotMsg3ImsiTag msg3ImsiTag;
      if (p->PeekPacketTag (msg3ImsiTag))
        {
          msg3Imsi = msg3ImsiTag.GetImsi ();
        }

      NS_LOG_INFO ("[ENB][MSG3][RX] TC-RNTI=" << rnti
                   << " lcid=" << (uint32_t) lcid
                   << " bufferSize=" << buffersize
                   << " pktUid=" << p->GetUid ());

      if (m_sharedFallbackBlockedRnti.find (rnti) != m_sharedFallbackBlockedRnti.end ())
        {
          NS_LOG_WARN ("[ENB][MSG3][DROP] tc-rnti=" << rnti
                       << " reason=shared-fallback-blocked");
          ++m_msg3DropCount;
          return;
        }

      std::map<uint16_t, SharedFallbackMsg3Context>::iterator sharedIt =
        m_sharedFallbackMsg3ByTcRnti.find (rnti);
      if (sharedIt != m_sharedFallbackMsg3ByTcRnti.end ())
        {
          const uint64_t nowSf =
            static_cast<uint64_t> (10 * (m_frameNo - 1) + (m_subframeNo - 1));
          if (nowSf > (sharedIt->second.windowEndSf + 1))
            {
              NS_LOG_WARN ("[ENB][MSG3][DROP] tc-rnti=" << rnti
                           << " reason=shared-fallback-window-expired"
                           << " now=" << nowSf
                           << " windowEnd=" << sharedIt->second.windowEndSf);
              ++m_msg3DropCount;
              m_sharedFallbackMsg3ByTcRnti.erase (sharedIt);
              return;
            }

          // Shared fallback is the unresolved-collision path common to legacy/sara/new.
          // It must consume Msg3 resources but never promote to RRC.
          if (!sharedIt->second.firstAccepted)
            {
              sharedIt->second.firstAccepted = true;
              NS_LOG_WARN ("[ENB][MSG3][DROP] tc-rnti=" << rnti
                           << " reason=shared-fallback-forced-drop-first");
            }
          else
            {
              NS_LOG_WARN ("[ENB][MSG3][DROP] tc-rnti=" << rnti
                           << " reason=shared-fallback-forced-drop-duplicate");
            }
          ++m_msg3DropCount;
          return;
        }

      if (m_newSchemaActivated)
        {
          NbIotScmaMsg3Tag scmaMsg3Tag;
          const bool hasScmaMsg3Tag = p->PeekPacketTag (scmaMsg3Tag);
          if (hasScmaMsg3Tag)
            {
              p->RemovePacketTag (scmaMsg3Tag);
            }

          std::map<uint16_t, ScmaMsg3ExpectedContext>::iterator expectedIt =
            m_expectedScmaMsg3ByTcRnti.find (rnti);
          if (expectedIt == m_expectedScmaMsg3ByTcRnti.end ())
            {
              NS_LOG_WARN ("eNB MSG3 drop: no expected SCMA context for tcRnti=" << rnti);
              NS_LOG_WARN ("[ENB][MSG3][DROP] tc-rnti=" << rnti << " reason=no-expected-context");
              if (SaraReport::IsEnabled ())
                {
                  SaraReport::LogMsg3EnbRx (rnti, msg3Imsi, lcid, false, 0, -1, -1, 0, 1);
                }
              ++m_msg3DropCount;
              return;
            }
          if (!hasScmaMsg3Tag)
            {
              NS_LOG_WARN ("eNB MSG3 drop: missing SCMA tag for tcRnti=" << rnti
                          << " expectedVirtualId=" << expectedIt->second.virtualId
                          << " expectedCodebook=" << static_cast<uint32_t> (expectedIt->second.codebookId));
              NS_LOG_WARN ("[ENB][MSG3][DROP] tc-rnti=" << rnti << " reason=missing-tag");
              if (SaraReport::IsEnabled ())
                {
                  SaraReport::LogMsg3EnbRx (rnti, msg3Imsi, lcid, false, 0,
                                            expectedIt->second.virtualId,
                                            expectedIt->second.physicalCarrier, 0, 2);
                }
              ++m_msg3DropCount;
              return;
            }

          const bool matches =
            (scmaMsg3Tag.GetTcRnti () == expectedIt->second.tcRnti) &&
            (scmaMsg3Tag.GetVirtualId () == expectedIt->second.virtualId) &&
            (scmaMsg3Tag.GetCodebookId () == expectedIt->second.codebookId) &&
            (scmaMsg3Tag.GetPhysicalCarrier () == expectedIt->second.physicalCarrier);
          if (!matches)
            {
              NS_LOG_WARN ("eNB MSG3 drop: SCMA context mismatch tcRnti=" << rnti
                          << " rx(tc=" << scmaMsg3Tag.GetTcRnti ()
                          << ",v=" << scmaMsg3Tag.GetVirtualId ()
                          << ",cb=" << static_cast<uint32_t> (scmaMsg3Tag.GetCodebookId ())
                          << ",pc=" << static_cast<uint32_t> (scmaMsg3Tag.GetPhysicalCarrier ())
                          << ") expected(tc=" << expectedIt->second.tcRnti
                          << ",v=" << expectedIt->second.virtualId
                          << ",cb=" << static_cast<uint32_t> (expectedIt->second.codebookId)
                          << ",pc=" << static_cast<uint32_t> (expectedIt->second.physicalCarrier)
                          << ")");
              NS_LOG_WARN ("[ENB][MSG3][DROP] tc-rnti=" << rnti << " reason=context-mismatch");
              if (SaraReport::IsEnabled ())
                {
                  SaraReport::LogMsg3EnbRx (rnti, msg3Imsi, lcid, false,
                                            scmaMsg3Tag.GetCodebookId (),
                                            scmaMsg3Tag.GetVirtualId (),
                                            scmaMsg3Tag.GetPhysicalCarrier (), 0, 3);
                }
              ++m_msg3DropCount;
              return;
            }

          NS_LOG_INFO ("[ENB][MSG3][ACCEPT] tc-rnti=" << rnti
                      << " virtualId=" << scmaMsg3Tag.GetVirtualId ()
                      << " codebook=" << static_cast<uint32_t> (scmaMsg3Tag.GetCodebookId ())
                      << " physicalCarrier=" << static_cast<uint32_t> (scmaMsg3Tag.GetPhysicalCarrier ()));
          if (SaraReport::IsEnabled ())
            {
              SaraReport::LogMsg3EnbRx (rnti, msg3Imsi, lcid, false,
                                        scmaMsg3Tag.GetCodebookId (),
                                        scmaMsg3Tag.GetVirtualId (),
                                        scmaMsg3Tag.GetPhysicalCarrier (), 1, 0);
            }
          m_expectedScmaMsg3ByTcRnti.erase (expectedIt);
          ++m_msg3RxCount;
          if (ForwardMsg3ToRlc (p, rnti, lcid))
            {
              if (SaraReport::IsEnabled ())
                {
                  SaraReport::LogMsg3ToRrc (rnti, msg3Imsi, lcid, false,
                                            scmaMsg3Tag.GetCodebookId (),
                                            scmaMsg3Tag.GetVirtualId (),
                                            scmaMsg3Tag.GetPhysicalCarrier ());
                }
              ++m_msg3AcceptedCount;
            }
          return;
        }

      // Match newSchema original semantics:
      // count MSG3 as RX_OK only once it passes mode-specific acceptance checks.
      ++m_msg3RxCount;

      SaraUlIdTag saraTag;
      if (p->PeekPacketTag (saraTag))
        {
          uint8_t cd;
          uint8_t dmrs;
          saraTag.Get (cd, dmrs);
          (void) dmrs;

          NS_LOG_INFO ("[ENB][MSG3][TAG-SARA] TC-RNTI=" << rnti
                       << " lcid=" << (uint32_t) lcid
                       << " cd=" << (uint32_t) cd);
          if (SaraReport::IsEnabled ())
            {
              SaraReport::LogMsg3EnbRx (rnti, msg3Imsi, lcid, true, cd);
            }

          uint64_t nowSubframe = static_cast<uint64_t> (Simulator::Now ().GetMilliSeconds ());
          uint64_t windowEnd = nowSubframe;
          std::map<uint16_t, uint64_t>::iterator wIt = m_rntiMsg3WindowEnd.find (rnti);
          if (wIt != m_rntiMsg3WindowEnd.end ())
            {
              windowEnd = wIt->second;
            }

          std::pair<uint16_t, uint64_t> key = std::make_pair (rnti, windowEnd);
          Msg3Buffer &buf = m_msg3Buffers[key];
          Msg3BufferEntry entry;
          entry.p = p;
          entry.lcid = lcid;
          entry.codebook = cd;
          buf.entries.push_back (entry);

          if (!buf.resolveEvent.IsRunning ())
            {
              uint64_t resolveSubframe = windowEnd;
              if (resolveSubframe < nowSubframe)
                {
                  resolveSubframe = nowSubframe;
                }
              Time delay = MilliSeconds (resolveSubframe - nowSubframe + 1);
              buf.resolveEvent =
                Simulator::Schedule (delay, &LteEnbMac::ResolveMsg3Window, this, rnti, windowEnd);
            }

          return; // se resolverá en bloque al final de la ventana
        }
      else
        {
          NS_LOG_INFO ("[ENB][MSG3][NO-SARA-TAG] TC-RNTI=" << rnti
                       << " lcid=" << (uint32_t) lcid);
          if (SaraReport::IsEnabled ())
            {
              SaraReport::LogMsg3EnbRx (rnti, msg3Imsi, lcid, false, 0);
            }
          if (ForwardMsg3ToRlc (p, rnti, lcid))
            {
              if (SaraReport::IsEnabled ())
                {
                  SaraReport::LogMsg3ToRrc (rnti, msg3Imsi, lcid, false, 0);
                }
              ++m_msg3AcceptedCount;
            }
          return;
        }
    }
  else if (p->RemovePacketTag (bsrTag))
    {
      buffersize = BufferSizeLevelBsr::BsrId2BufferSize (bsrTag.GetBufferStatusReportIndex ());
      buffersize += 4; // Compensate RLC Header etc
      m_schedulerNb->ScheduleUlRlcBufferReq (rnti, buffersize);
      return;
    }

  ForwardMsg3ToRlc (p, rnti, lcid);
}

// ////////////////////////////////////////////
// CMAC SAP  14 funciones
// ////////////////////////////////////////////

void
LteEnbMac::DoConfigureMac (uint16_t ulBandwidth, uint16_t dlBandwidth)
{
  NS_LOG_FUNCTION (this << " ulBandwidth=" << ulBandwidth << " dlBandwidth=" << dlBandwidth);
  FfMacCschedSapProvider::CschedCellConfigReqParameters params;
  // Configure the subset of parameters used by FfMacScheduler
  params.m_ulBandwidth = ulBandwidth;
  params.m_dlBandwidth = dlBandwidth;
  m_macChTtiDelay = m_enbPhySapProvider->GetMacChTtiDelay ();
  // ...more parameters can be configured
  m_cschedSapProvider->CschedCellConfigReq (params);
}

void
LteEnbMac::DoAddUe (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << " rnti=" << rnti);
  if (SaraReport::IsEnabled ())
    {
      SaraReport::LogEnbContextEvent ("mac", "add-ue", rnti);
    }
  std::map<uint8_t, LteMacSapUser *> empty;
  std::pair<std::map<uint16_t, std::map<uint8_t, LteMacSapUser *>>::iterator, bool> ret =
      m_rlcAttached.insert (std::pair<uint16_t, std::map<uint8_t, LteMacSapUser *>> (rnti, empty));
  NS_ASSERT_MSG (ret.second, "element already present, RNTI already existed");

  FfMacCschedSapProvider::CschedUeConfigReqParameters params;
  params.m_rnti = rnti;
  params.m_transmissionMode =
      0; // set to default value (SISO) for avoiding random initialization (valgrind error)

  m_cschedSapProvider->CschedUeConfigReq (params);

  // Create DL transmission HARQ buffers
  std::vector<Ptr<PacketBurst>> dlHarqLayer0pkt;
  dlHarqLayer0pkt.resize (8);
  for (uint8_t i = 0; i < 8; i++)
    {
      Ptr<PacketBurst> pb = CreateObject<PacketBurst> ();
      dlHarqLayer0pkt.at (i) = pb;
    }
  std::vector<Ptr<PacketBurst>> dlHarqLayer1pkt;
  dlHarqLayer1pkt.resize (8);
  for (uint8_t i = 0; i < 8; i++)
    {
      Ptr<PacketBurst> pb = CreateObject<PacketBurst> ();
      dlHarqLayer1pkt.at (i) = pb;
    }
  DlHarqProcessesBuffer_t buf;
  buf.push_back (dlHarqLayer0pkt);
  buf.push_back (dlHarqLayer1pkt);
  m_miDlHarqProcessesPackets.insert (std::pair<uint16_t, DlHarqProcessesBuffer_t> (rnti, buf));
}

void 
LteEnbMac::DoMoveUeToResume(uint16_t rnti, uint64_t resumeId){
  m_resumeRlcAttached[resumeId] = m_rlcAttached[rnti];
  m_connectionSuccessful[rnti]= false;
}
void 
LteEnbMac::DoResumeUe(uint16_t rnti, uint64_t resumeId){
  m_rlcAttached[rnti] = m_resumeRlcAttached[resumeId];
  m_resumeRlcAttached.erase(resumeId);

  // Reinitialize HARQ 
  // Create DL transmission HARQ buffers
  std::vector<Ptr<PacketBurst>> dlHarqLayer0pkt;
  dlHarqLayer0pkt.resize (8);
  for (uint8_t i = 0; i < 8; i++)
    {
      Ptr<PacketBurst> pb = CreateObject<PacketBurst> ();
      dlHarqLayer0pkt.at (i) = pb;
    }
  std::vector<Ptr<PacketBurst>> dlHarqLayer1pkt;
  dlHarqLayer1pkt.resize (8);
  for (uint8_t i = 0; i < 8; i++)
    {
      Ptr<PacketBurst> pb = CreateObject<PacketBurst> ();
      dlHarqLayer1pkt.at (i) = pb;
    }
  DlHarqProcessesBuffer_t buf;
  buf.push_back (dlHarqLayer0pkt);
  buf.push_back (dlHarqLayer1pkt);
  m_miDlHarqProcessesPackets.insert (std::pair<uint16_t, DlHarqProcessesBuffer_t> (rnti, buf));
}

void LteEnbMac::DoRemoveUeFromScheduler(uint16_t rnti){
  m_schedulerNb->RemoveUe(rnti);
}
void LteEnbMac::DoSetLogDir(std::string logdir){
  m_logdir = logdir;
  m_mac_logging = true;
}

void
LteEnbMac::DoMapTempRntiToDefRnti (uint16_t tempRnti, uint16_t assignedRnti)
{
  m_tempRntiToDefRnti[tempRnti].push_back (assignedRnti);
  // Keep shared-fallback guards untouched here.
  // They are cleaned by DoRemoveUe/timeout paths and must not be cleared during
  // temp->def mapping, otherwise unresolved-collision Msg3 can leak to RRC.

  std::map<uint16_t, uint16_t>::const_iterator bsrIt = m_ueStoredBSR.find (tempRnti);
  if (bsrIt != m_ueStoredBSR.end ())
    {
      m_ueStoredBSR[assignedRnti] = bsrIt->second;
    }
  m_connectionSuccessful[assignedRnti] = false;
  if (m_schedulerNb)
    {
      m_schedulerNb->CloneUeConfig (tempRnti, assignedRnti);
    }
}

void
LteEnbMac::DoRegisterMsg4ValidityContext (
    uint16_t rnti, const LteEnbCmacSapProvider::Msg4ValidityContext &ctx)
{
  if (m_schedulerNb == nullptr)
    {
      return;
    }
  Msg4Context schedCtx;
  schedCtx.active = true;
  schedCtx.imsi = ctx.imsi;
  schedCtx.raAttemptId = ctx.raAttemptId;
  schedCtx.tcRnti = ctx.tcRnti;
  schedCtx.msg3EndSubframe = ctx.msg3EndSubframe;
  schedCtx.deadlineSubframe = ctx.deadlineSubframe;
  m_schedulerNb->RegisterMsg4Context (rnti, schedCtx);
}

void
LteEnbMac::DoInvalidateMsg4ValidityContext (uint16_t rnti)
{
  if (m_schedulerNb == nullptr)
    {
      return;
    }
  m_schedulerNb->InvalidateMsg4Context (rnti);
}

void
LteEnbMac::DoRemoveUe (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << " rnti=" << rnti);
  if (m_schedulerNb)
    {
      m_schedulerNb->InvalidateMsg4Context (rnti);
    }
  if (SaraReport::IsEnabled ())
    {
      SaraReport::LogEnbContextEvent ("mac", "remove-ue", rnti);
    }
  FfMacCschedSapProvider::CschedUeReleaseReqParameters params;
  params.m_rnti = rnti;
  m_cschedSapProvider->CschedUeReleaseReq (params);
  m_rlcAttached.erase (rnti);
  m_miDlHarqProcessesPackets.erase (rnti);
  m_tempRntiToDefRnti.erase (rnti);
  for (std::map<uint16_t, std::deque<uint16_t> >::iterator it = m_tempRntiToDefRnti.begin ();
       it != m_tempRntiToDefRnti.end (); )
    {
      std::deque<uint16_t> &q = it->second;
      for (std::deque<uint16_t>::iterator qit = q.begin (); qit != q.end (); )
        {
          if (*qit == rnti)
            {
              qit = q.erase (qit);
            }
          else
            {
              ++qit;
            }
        }
      if (q.empty ())
        {
          it = m_tempRntiToDefRnti.erase (it);
        }
      else
        {
          ++it;
        }
    }
  m_expectedScmaMsg3ByTcRnti.erase (rnti);
  m_sharedFallbackMsg3ByTcRnti.erase (rnti);
  m_sharedFallbackBlockedRnti.erase (rnti);

  NS_LOG_DEBUG ("start checking for unprocessed preamble for rnti: " << rnti);
  //remove unprocessed preamble received for RACH during handover
  std::map<uint8_t, NcRaPreambleInfo>::iterator jt = m_allocatedNcRaPreambleMap.begin ();
  while (jt != m_allocatedNcRaPreambleMap.end ())
    {
      if (jt->second.rnti == rnti)
        {
          std::map<uint8_t, uint32_t>::const_iterator it =
              m_receivedRachPreambleCount.find (jt->first);
          if (it != m_receivedRachPreambleCount.end ())
            {
              m_receivedRachPreambleCount.erase (it->first);
            }
          jt = m_allocatedNcRaPreambleMap.erase (jt);
        }
      else
        {
          ++jt;
        }
    }

  std::vector<MacCeListElement_s>::iterator itCeRxd = m_ulCeReceived.begin ();
  while (itCeRxd != m_ulCeReceived.end ())
    {
      if (itCeRxd->m_rnti == rnti)
        {
          itCeRxd = m_ulCeReceived.erase (itCeRxd);
        }
      else
        {
          itCeRxd++;
        }
    }

}

void
LteEnbMac::DoAddLc (LteEnbCmacSapProvider::LcInfo lcinfo, LteMacSapUser *msu)
{
  NS_LOG_FUNCTION (this << lcinfo.rnti << (uint16_t) lcinfo.lcId);

  std::map<LteFlowId_t, LteMacSapUser *>::iterator it;

  LteFlowId_t flow (lcinfo.rnti, lcinfo.lcId);

  std::map<uint16_t, std::map<uint8_t, LteMacSapUser *>>::iterator rntiIt =
      m_rlcAttached.find (lcinfo.rnti);
  NS_ASSERT_MSG (rntiIt != m_rlcAttached.end (), "RNTI not found");
  std::map<uint8_t, LteMacSapUser *>::iterator lcidIt = rntiIt->second.find (lcinfo.lcId);
  if (lcidIt == rntiIt->second.end ())
    {
      rntiIt->second.insert (std::pair<uint8_t, LteMacSapUser *> (lcinfo.lcId, msu));
    }
  else
    {
      NS_LOG_ERROR ("LC already exists");
    }

  // CCCH (LCID 0) is pre-configured
  // see FF LTE MAC Scheduler
  // Interface Specification v1.11,
  // 4.3.4 logicalChannelConfigListElement
  if (lcinfo.lcId != 0)
    {
      struct FfMacCschedSapProvider::CschedLcConfigReqParameters params;
      params.m_rnti = lcinfo.rnti;
      params.m_reconfigureFlag = false;

      struct LogicalChannelConfigListElement_s lccle;
      lccle.m_logicalChannelIdentity = lcinfo.lcId;
      lccle.m_logicalChannelGroup = lcinfo.lcGroup;
      lccle.m_direction = LogicalChannelConfigListElement_s::DIR_BOTH;
      lccle.m_qosBearerType = lcinfo.isGbr ? LogicalChannelConfigListElement_s::QBT_GBR
                                           : LogicalChannelConfigListElement_s::QBT_NON_GBR;
      lccle.m_qci = lcinfo.qci;
      lccle.m_eRabMaximulBitrateUl = lcinfo.mbrUl;
      lccle.m_eRabMaximulBitrateDl = lcinfo.mbrDl;
      lccle.m_eRabGuaranteedBitrateUl = lcinfo.gbrUl;
      lccle.m_eRabGuaranteedBitrateDl = lcinfo.gbrDl;
      params.m_logicalChannelConfigList.push_back (lccle);

      m_cschedSapProvider->CschedLcConfigReq (params);
    }
}

void
LteEnbMac::DoReconfigureLc (LteEnbCmacSapProvider::LcInfo lcinfo)
{
  NS_FATAL_ERROR ("not implemented");
}

void
LteEnbMac::DoReleaseLc (uint16_t rnti, uint8_t lcid)
{
  NS_LOG_FUNCTION (this);

  //Find user based on rnti and then erase lcid stored against the same
  std::map<uint16_t, std::map<uint8_t, LteMacSapUser *>>::iterator rntiIt =
      m_rlcAttached.find (rnti);
  rntiIt->second.erase (lcid);

  struct FfMacCschedSapProvider::CschedLcReleaseReqParameters params;
  params.m_rnti = rnti;
  params.m_logicalChannelIdentity.push_back (lcid);
  m_cschedSapProvider->CschedLcReleaseReq (params);
}

void
LteEnbMac::DoUeUpdateConfigurationReq (LteEnbCmacSapProvider::UeConfig params)
{
  NS_LOG_FUNCTION (this);

  // propagates to scheduler
  FfMacCschedSapProvider::CschedUeConfigReqParameters req;
  req.m_rnti = params.m_rnti;
  req.m_transmissionMode = params.m_transmissionMode;
  req.m_reconfigureFlag = true;
  m_cschedSapProvider->CschedUeConfigReq (req);
}

LteEnbCmacSapProvider::RachConfig
LteEnbMac::DoGetRachConfig ()
{
  struct LteEnbCmacSapProvider::RachConfig rc;
  rc.numberOfRaPreambles = m_numberOfRaPreambles;
  rc.preambleTransMax = m_preambleTransMax;
  rc.raResponseWindowSize = m_raResponseWindowSize;
  rc.connEstFailCount = m_connEstFailCount;
  return rc;
}
LteEnbCmacSapProvider::RachConfigNb
LteEnbMac::DoGetRachConfigNb ()
{
  struct LteEnbCmacSapProvider::RachConfigNb rc;
  //rc.numberOfRaPreambles = m_numberOfRaPreambles;
  //rc.preambleTransMax = m_preambleTransMax;
  //rc.raResponseWindowSize = m_raResponseWindowSize;
  //rc.connEstFailCount = m_connEstFailCount;
  return rc;
}
LteEnbCmacSapProvider::AllocateNcRaPreambleReturnValue
LteEnbMac::DoAllocateNcRaPreamble (uint16_t rnti)
{
  bool found = false;
  uint8_t preambleId;
  for (preambleId = m_numberOfRaPreambles; preambleId < 64; ++preambleId)
    {
      std::map<uint8_t, NcRaPreambleInfo>::iterator it =
          m_allocatedNcRaPreambleMap.find (preambleId);
      /**
       * Allocate preamble only if its free. The non-contention preamble
       * assigned to UE during handover or PDCCH order is valid only until the
       * time duration of the “expiryTime” of the preamble is reached. This
       * timer value is only maintained at the eNodeB and the UE has no way of
       * knowing if this timer has expired. If the UE tries to send the preamble
       * again after the expiryTime and the preamble is re-assigned to another
       * UE, it results in errors. This has been solved by re-assigning the
       * preamble to another UE only if it is not being used (An UE can be using
       * the preamble even after the expiryTime duration).
       */
      if ((it != m_allocatedNcRaPreambleMap.end ()) && (it->second.expiryTime < Simulator::Now ()))
        {
          if (!m_cmacSapUser->IsRandomAccessCompleted (rnti))
            {
              //random access of the UE is not completed,
              //check other preambles
              continue;
            }
        }
      if ((it == m_allocatedNcRaPreambleMap.end ()) || (it->second.expiryTime < Simulator::Now ()))
        {
          found = true;
          NcRaPreambleInfo preambleInfo;
          uint32_t expiryIntervalMs =
              (uint32_t) m_preambleTransMax * ((uint32_t) m_raResponseWindowSize + 5);

          preambleInfo.expiryTime = Simulator::Now () + MilliSeconds (expiryIntervalMs);
          preambleInfo.rnti = rnti;
          NS_LOG_INFO ("allocated preamble for NC based RA: preamble "
                       << preambleId << ", RNTI " << preambleInfo.rnti << ", exiryTime "
                       << preambleInfo.expiryTime);
          m_allocatedNcRaPreambleMap[preambleId] =
              preambleInfo; // create if not exist, update otherwise
          break;
        }
    }
  LteEnbCmacSapProvider::AllocateNcRaPreambleReturnValue ret;
  if (found)
    {
      ret.valid = true;
      ret.raPreambleId = preambleId;
      ret.raPrachMaskIndex = 0;
    }
  else
    {
      ret.valid = false;
      ret.raPreambleId = 0;
      ret.raPrachMaskIndex = 0;
    }
  return ret;
}

// ////////////////////////////////////////////
// MAC SAP
// ////////////////////////////////////////////

void
LteEnbMac::DoTransmitPdu (LteMacSapProvider::TransmitPduParameters params)
{
  NS_LOG_FUNCTION (this);

  if (SaraReport::IsEnabled () && (params.lcid == 0))
    {
      // Log actual "on-air" Msg4 when handed to PHY.
      Ptr<Packet> copy = params.pdu->Copy ();
      RrcDlCcchMessage dlMsg;
      if ((copy->PeekHeader (dlMsg) != 0) && (dlMsg.GetMessageType () == 3))
        {
          RrcConnectionSetupHeader setupHdr;
          copy->RemoveHeader (setupHdr);
          LteRrcSap::RrcConnectionSetup msg = setupHdr.GetMessage ();
          const bool sara = (msg.ueIdentity != 0) || (msg.assignedRnti != 0);
          const uint16_t cRnti = (sara && (msg.assignedRnti != 0)) ? msg.assignedRnti : params.rnti;
          SaraReport::LogMsg4AirEnb (params.rnti, msg.ueIdentity, cRnti, sara, params.pdu->GetSize ());
        }
    }

  // Peek RLC Header to issue StatusPDU scheduling if needed

  LteRlcAmHeader rlcAmHeader;
  if(params.pdu->PeekHeader(rlcAmHeader) != 0){
    // Is RLC AM, check if status pdu is requested
    if ( rlcAmHeader.GetPollingBit () == LteRlcAmHeader::STATUS_REPORT_IS_REQUESTED )
      {
        m_schedulerNb->AddToUlBufferReq(params.rnti, 4); // Add Status Pdu to be scheduled (4 Byte)
      }
  }

  LteRadioBearerTag tag (params.rnti, params.lcid, params.layer);
  params.pdu->AddPacketTag (tag);
  params.componentCarrierId = m_componentCarrierId;
  // Store pkt in HARQ buffer
  //std::map<uint16_t, DlHarqProcessesBuffer_t>::iterator it =
  //    m_miDlHarqProcessesPackets.find (params.rnti);
  //NS_ASSERT (it != m_miDlHarqProcessesPackets.end ());
  //NS_LOG_DEBUG (this << " LAYER " << (uint16_t) tag.GetLayer () << " HARQ ID "
  //                   << (uint16_t) params.harqProcessId);

  ////(*it).second.at (params.layer).at (params.harqProcessId) = params.pdu;//->Copy ();
  //(*it).second.at (params.layer).at (params.harqProcessId)->AddPacket (params.pdu);
  m_enbPhySapProvider->SendMacPdu (params.pdu);
}

void
LteEnbMac::DoReportBufferStatus (LteMacSapProvider::ReportBufferStatusParameters params)
{
  NS_LOG_FUNCTION (this);
  FfMacSchedSapProvider::SchedDlRlcBufferReqParameters req;
  req.m_rnti = params.rnti;
  req.m_logicalChannelIdentity = params.lcid;
  req.m_rlcTransmissionQueueSize = params.txQueueSize;
  req.m_rlcTransmissionQueueHolDelay = params.txQueueHolDelay;
  req.m_rlcRetransmissionQueueSize = params.retxQueueSize;
  req.m_rlcRetransmissionHolDelay = params.retxQueueHolDelay;
  req.m_rlcStatusPduSize = params.statusPduSize;
  m_schedSapProvider->SchedDlRlcBufferReq (req);
}

void
LteEnbMac::DoReportBufferStatusNb (LteMacSapProvider::ReportBufferStatusParameters params,
                                   NbIotRrcSap::NpdcchMessage::SearchSpaceType searchspace)
{
  NS_LOG_FUNCTION (this);
  m_lastDlBSR[params.rnti][params.lcid] = params;
  if(params.lcid == 1 || params.lcid == 3){
    // We are using SRB1 or DRB -> RLC AM Header
  //m_lastDlBSR[params.rnti][params.lcid].txQueueSize;

  }
  m_schedulerNb->ScheduleDlRlcBufferReq(params.rnti, m_lastDlBSR[params.rnti]);
  
}
// ////////////////////////////////////////////
// SCHED SAP
// ////////////////////////////////////////////

void
LteEnbMac::DoSchedDlConfigInd (FfMacSchedSapUser::SchedDlConfigIndParameters ind)
{
  NS_LOG_FUNCTION (this);
  // Create DL PHY PDU
  Ptr<PacketBurst> pb = CreateObject<PacketBurst> ();
  std::map<LteFlowId_t, LteMacSapUser *>::iterator it;
  LteMacSapUser::TxOpportunityParameters txOpParams;

  for (unsigned int i = 0; i < ind.m_buildDataList.size (); i++)
    {
      for (uint16_t layer = 0; layer < ind.m_buildDataList.at (i).m_dci.m_ndi.size (); layer++)
        {
          if (ind.m_buildDataList.at (i).m_dci.m_ndi.at (layer) == 1)
            {
              // new data -> force emptying correspondent harq pkt buffer
              std::map<uint16_t, DlHarqProcessesBuffer_t>::iterator it =
                  m_miDlHarqProcessesPackets.find (ind.m_buildDataList.at (i).m_rnti);
              NS_ASSERT (it != m_miDlHarqProcessesPackets.end ());
              for (uint16_t lcId = 0; lcId < (*it).second.size (); lcId++)
                {
                  Ptr<PacketBurst> pb = CreateObject<PacketBurst> ();
                  (*it).second.at (lcId).at (ind.m_buildDataList.at (i).m_dci.m_harqProcess) = pb;
                }
            }
        }
      for (unsigned int j = 0; j < ind.m_buildDataList.at (i).m_rlcPduList.size (); j++)
        {
          for (uint16_t k = 0; k < ind.m_buildDataList.at (i).m_rlcPduList.at (j).size (); k++)
            {
              if (ind.m_buildDataList.at (i).m_dci.m_ndi.at (k) == 1)
                {
                  // New Data -> retrieve it from RLC
                  uint16_t rnti = ind.m_buildDataList.at (i).m_rnti;
                  uint8_t lcid = ind.m_buildDataList.at (i)
                                     .m_rlcPduList.at (j)
                                     .at (k)
                                     .m_logicalChannelIdentity;
                  std::map<uint16_t, std::map<uint8_t, LteMacSapUser *>>::iterator rntiIt =
                      m_rlcAttached.find (rnti);
                  NS_ASSERT_MSG (rntiIt != m_rlcAttached.end (), "could not find RNTI" << rnti);
                  std::map<uint8_t, LteMacSapUser *>::iterator lcidIt = rntiIt->second.find (lcid);
                  NS_ASSERT_MSG (lcidIt != rntiIt->second.end (),
                                 "could not find LCID" << (uint32_t) lcid << " carrier id:"
                                                       << (uint16_t) m_componentCarrierId);
                  NS_LOG_DEBUG (this << " rnti= " << rnti << " lcid= " << (uint32_t) lcid
                                     << " layer= " << k);
                  txOpParams.bytes = ind.m_buildDataList.at (i).m_rlcPduList.at (j).at (k).m_size;
                  txOpParams.layer = k;
                  txOpParams.harqId = ind.m_buildDataList.at (i).m_dci.m_harqProcess;
                  txOpParams.componentCarrierId = m_componentCarrierId;
                  txOpParams.rnti = rnti;
                  txOpParams.lcid = lcid;
                  (*lcidIt).second->NotifyTxOpportunity (txOpParams);
                }
              else
                {
                  if (ind.m_buildDataList.at (i).m_dci.m_tbsSize.at (k) > 0)
                    {
                      // HARQ retransmission -> retrieve TB from HARQ buffer
                      std::map<uint16_t, DlHarqProcessesBuffer_t>::iterator it =
                          m_miDlHarqProcessesPackets.find (ind.m_buildDataList.at (i).m_rnti);
                      NS_ASSERT (it != m_miDlHarqProcessesPackets.end ());
                      Ptr<PacketBurst> pb =
                          (*it).second.at (k).at (ind.m_buildDataList.at (i).m_dci.m_harqProcess);
                      for (std::list<Ptr<Packet>>::const_iterator j = pb->Begin (); j != pb->End ();
                           ++j)
                        {
                          Ptr<Packet> pkt = (*j)->Copy ();
                          m_enbPhySapProvider->SendMacPdu (pkt);
                        }
                    }
                }
            }
        }
      // send the relative DCI
      Ptr<DlDciLteControlMessage> msg = Create<DlDciLteControlMessage> ();
      msg->SetDci (ind.m_buildDataList.at (i).m_dci);
      m_enbPhySapProvider->SendLteControlMessage (msg);
    }

  // Fire the trace with the DL information
  for (uint32_t i = 0; i < ind.m_buildDataList.size (); i++)
    {
      // Only one TB used
      if (ind.m_buildDataList.at (i).m_dci.m_tbsSize.size () == 1)
        {
          DlSchedulingCallbackInfo dlSchedulingCallbackInfo;
          dlSchedulingCallbackInfo.frameNo = m_frameNo;
          dlSchedulingCallbackInfo.subframeNo = m_subframeNo;
          dlSchedulingCallbackInfo.rnti = ind.m_buildDataList.at (i).m_dci.m_rnti;
          dlSchedulingCallbackInfo.mcsTb1 = ind.m_buildDataList.at (i).m_dci.m_mcs.at (0);
          dlSchedulingCallbackInfo.sizeTb1 = ind.m_buildDataList.at (i).m_dci.m_tbsSize.at (0);
          dlSchedulingCallbackInfo.mcsTb2 = 0;
          dlSchedulingCallbackInfo.sizeTb2 = 0;
          dlSchedulingCallbackInfo.componentCarrierId = m_componentCarrierId;
          m_dlScheduling (dlSchedulingCallbackInfo);
        }
      // Two TBs used
      else if (ind.m_buildDataList.at (i).m_dci.m_tbsSize.size () == 2)
        {
          DlSchedulingCallbackInfo dlSchedulingCallbackInfo;
          dlSchedulingCallbackInfo.frameNo = m_frameNo;
          dlSchedulingCallbackInfo.subframeNo = m_subframeNo;
          dlSchedulingCallbackInfo.rnti = ind.m_buildDataList.at (i).m_dci.m_rnti;
          dlSchedulingCallbackInfo.mcsTb1 = ind.m_buildDataList.at (i).m_dci.m_mcs.at (0);
          dlSchedulingCallbackInfo.sizeTb1 = ind.m_buildDataList.at (i).m_dci.m_tbsSize.at (0);
          dlSchedulingCallbackInfo.mcsTb2 = ind.m_buildDataList.at (i).m_dci.m_mcs.at (1);
          dlSchedulingCallbackInfo.sizeTb2 = ind.m_buildDataList.at (i).m_dci.m_tbsSize.at (1);
          dlSchedulingCallbackInfo.componentCarrierId = m_componentCarrierId;
          m_dlScheduling (dlSchedulingCallbackInfo);
        }
      else
        {
          NS_FATAL_ERROR ("Found element with more than two transport blocks");
        }
    }

  // Random Access procedure: send RARs
  Ptr<RarLteControlMessage> rarMsg = Create<RarLteControlMessage> ();
  // see TS 36.321 5.1.4;  preambles were sent two frames ago
  // (plus 3GPP counts subframes from 0, not 1)
  uint16_t raRnti;
  if (m_subframeNo < 3)
    {
      raRnti = m_subframeNo + 7; // equivalent to +10-3
    }
  else
    {
      raRnti = m_subframeNo - 3;
    }
  rarMsg->SetRaRnti (raRnti);
  for (unsigned int i = 0; i < ind.m_buildRarList.size (); i++)
    {
      std::map<uint16_t, uint32_t>::iterator itRapId =
          m_rapIdRntiMap.find (ind.m_buildRarList.at (i).m_rnti);
      if (itRapId == m_rapIdRntiMap.end ())
        {
          NS_FATAL_ERROR ("Unable to find rapId of RNTI " << ind.m_buildRarList.at (i).m_rnti);
        }
      RarLteControlMessage::Rar rar;
      rar.rapId = itRapId->second;
      rar.rarPayload = ind.m_buildRarList.at (i);
      rarMsg->AddRar (rar);
      NS_LOG_INFO (this << " Send RAR message to RNTI " << ind.m_buildRarList.at (i).m_rnti
                        << " rapId " << itRapId->second);
    }
  if (ind.m_buildRarList.size () > 0)
    {
      m_enbPhySapProvider->SendLteControlMessage (rarMsg);
    }
  m_rapIdRntiMap.clear ();
}

void
LteEnbMac::DoSchedUlConfigInd (FfMacSchedSapUser::SchedUlConfigIndParameters ind)
{
  NS_LOG_FUNCTION (this);

  for (unsigned int i = 0; i < ind.m_dciList.size (); i++)
    {
      // send the correspondent ul dci
      Ptr<UlDciLteControlMessage> msg = Create<UlDciLteControlMessage> ();
      msg->SetDci (ind.m_dciList.at (i));
      m_enbPhySapProvider->SendLteControlMessage (msg);
    }

  // Fire the trace with the UL information
  for (uint32_t i = 0; i < ind.m_dciList.size (); i++)
    {
      m_ulScheduling (m_frameNo, m_subframeNo, ind.m_dciList.at (i).m_rnti,
                      ind.m_dciList.at (i).m_mcs, ind.m_dciList.at (i).m_tbSize,
                      m_componentCarrierId);
    }
}

// ////////////////////////////////////////////
// CSCHED SAP
// ////////////////////////////////////////////

void
LteEnbMac::DoCschedCellConfigCnf (FfMacCschedSapUser::CschedCellConfigCnfParameters params)
{
  NS_LOG_FUNCTION (this);
}

void
LteEnbMac::DoCschedUeConfigCnf (FfMacCschedSapUser::CschedUeConfigCnfParameters params)
{
  NS_LOG_FUNCTION (this);
}

void
LteEnbMac::DoCschedLcConfigCnf (FfMacCschedSapUser::CschedLcConfigCnfParameters params)
{
  NS_LOG_FUNCTION (this);
  // Call the CSCHED primitive
  // m_cschedSap->LcConfigCompleted();
}

void
LteEnbMac::DoCschedLcReleaseCnf (FfMacCschedSapUser::CschedLcReleaseCnfParameters params)
{
  NS_LOG_FUNCTION (this);
}

void
LteEnbMac::DoCschedUeReleaseCnf (FfMacCschedSapUser::CschedUeReleaseCnfParameters params)
{
  NS_LOG_FUNCTION (this);
}

void
LteEnbMac::DoCschedUeConfigUpdateInd (FfMacCschedSapUser::CschedUeConfigUpdateIndParameters params)
{
  NS_LOG_FUNCTION (this);
  // propagates to RRC
  LteEnbCmacSapUser::UeConfig ueConfigUpdate;
  ueConfigUpdate.m_rnti = params.m_rnti;
  ueConfigUpdate.m_transmissionMode = params.m_transmissionMode;
  m_cmacSapUser->RrcConfigurationUpdateInd (ueConfigUpdate);
}

void
LteEnbMac::DoCschedCellConfigUpdateInd (
    FfMacCschedSapUser::CschedCellConfigUpdateIndParameters params)
{
  NS_LOG_FUNCTION (this);
}

void
LteEnbMac::DoUlInfoListElementHarqFeeback (UlInfoListElement_s params)
{
  NS_LOG_FUNCTION (this);
  m_ulInfoListReceived.push_back (params);
}

void
LteEnbMac::DoDlInfoListElementHarqFeeback (DlInfoListElement_s params)
{
  NS_LOG_FUNCTION (this);
  // Update HARQ buffer
  std::map<uint16_t, DlHarqProcessesBuffer_t>::iterator it =
      m_miDlHarqProcessesPackets.find (params.m_rnti);
  NS_ASSERT (it != m_miDlHarqProcessesPackets.end ());
  for (uint8_t layer = 0; layer < params.m_harqStatus.size (); layer++)
    {
      if (params.m_harqStatus.at (layer) == DlInfoListElement_s::ACK)
        {
          // discard buffer
          Ptr<PacketBurst> emptyBuf = CreateObject<PacketBurst> ();
          (*it).second.at (layer).at (params.m_harqProcessId) = emptyBuf;
          NS_LOG_DEBUG (this << " HARQ-ACK UE " << params.m_rnti << " harqId "
                             << (uint16_t) params.m_harqProcessId << " layer " << (uint16_t) layer);
        }
      else if (params.m_harqStatus.at (layer) == DlInfoListElement_s::NACK)
        {
          NS_LOG_DEBUG (this << " HARQ-NACK UE " << params.m_rnti << " harqId "
                             << (uint16_t) params.m_harqProcessId << " layer " << (uint16_t) layer);
        }
      else
        {
          NS_FATAL_ERROR (" HARQ functionality not implemented");
        }
    }
  m_dlInfoListReceived.push_back (params);
}

void LteEnbMac::DoNotifyConnectionSuccessful(uint16_t rnti){
  m_connectionSuccessful[rnti] = true;
  if (m_schedulerNb)
    {
      m_schedulerNb->InvalidateMsg4Context (rnti);
      for (std::map<uint16_t, std::deque<uint16_t> >::const_iterator it =
               m_tempRntiToDefRnti.begin ();
           it != m_tempRntiToDefRnti.end (); ++it)
        {
          const std::deque<uint16_t> &q = it->second;
          if (std::find (q.begin (), q.end (), rnti) != q.end ())
            {
              m_schedulerNb->InvalidateMsg4Context (it->first);
            }
        }
    }
  //if (m_ueStoredBSR[rnti] > 0){
  //    uint64_t dataSize = BufferSizeLevelBsr::BsrId2BufferSize(m_ueStoredBSR[rnti]);
  //    m_schedulerNb->ScheduleUlRlcBufferReq(rnti,dataSize,NbIotRrcSap::NpdcchMessage::SearchSpaceType::type2);
  //    m_ueStoredBSR[rnti]=0;
  //}
}

void LteEnbMac::CheckForDataInactivity(uint16_t rnti){
 std::map<uint8_t, LteMacSapProvider::ReportBufferStatusParameters>::iterator it; 
  bool buffer_remaining = false;
  for(it = m_lastDlBSR[rnti].begin(); it != m_lastDlBSR[rnti].end(); ++it){
    if(!(it->second.txQueueSize == 0 && it->second.retxQueueSize == 0 && it->second.statusPduSize == 0)){
      buffer_remaining = true;
      break;
    }
  }
  if(!buffer_remaining && m_ueStoredBSR[rnti] ==0){
    m_cmacSapUser->NotifyDataInactivitySchedulerNb(rnti);
  }
}
void LteEnbMac::DoReportNoTransmissionNb(uint16_t rnti, uint8_t lcid){
}
} // namespace ns3
