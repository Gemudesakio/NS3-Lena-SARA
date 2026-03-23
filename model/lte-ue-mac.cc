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
 * Author: Nicola Baldo  <nbaldo@cttc.es>
 * Author: Marco Miozzo <mmiozzo@cttc.es>
 * Modified by: 
 *          Tim Gebauer <tim.gebauer@tu-dortmund.de> (NB-IoT Extension)
 *          Pascal Jörke <pascal.joerke@tu-dortmund.de> (NB-IoT Extension)
 */

#include <ns3/log.h>
#include <ns3/pointer.h>
#include <ns3/packet.h>
#include <ns3/packet-burst.h>
#include <ns3/random-variable-stream.h>
#include <ns3/boolean.h>
#include <ns3/uinteger.h>
#include <ns3/build-profile.h>

#include "lte-ue-mac.h"
#include "lte-ue-net-device.h"
#include "lte-radio-bearer-tag.h"
#include "nb-iot-toa-utils.h"
#include "nb-iot-data-volume-and-power-headroom-tag.h"
#include "nb-iot-buffer-status-report-tag.h"
#include "nb-iot-scma-msg3-tag.h"
#include "nb-iot-msg3-imsi-tag.h"
#include "sara-report.h"
#include "sara-ul-id-tag.h"
#include <ns3/ff-mac-common.h>
#include <ns3/lte-control-messages.h>
#include <ns3/simulator.h>
#include <ns3/lte-common.h>
#include <fstream>
#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LteUeMac");

NS_OBJECT_ENSURE_REGISTERED (LteUeMac);

///////////////////////////////////////////////////////////
// SAP forwarders
///////////////////////////////////////////////////////////

/// UeMemberLteUeCmacSapProvider class
class UeMemberLteUeCmacSapProvider : public LteUeCmacSapProvider
{
public:
  /**
   * Constructor
   *
   * \param mac the UE MAC
   */
  UeMemberLteUeCmacSapProvider (LteUeMac *mac);

  // inherited from LteUeCmacSapProvider
  virtual void ConfigureRach (RachConfig rc);
  virtual void ConfigureRadioResourceConfig (NbIotRrcSap::RadioResourceConfigCommonNb rc);
  virtual void StartContentionBasedRandomAccessProcedure ();
  virtual void StartRandomAccessProcedureNb (bool edt);
  virtual void StartNonContentionBasedRandomAccessProcedure (uint16_t rnti, uint8_t preambleId,
                                                             uint8_t prachMask);
  virtual void SetRnti (uint16_t rnti);
  virtual void AddLc (uint8_t lcId, LteUeCmacSapProvider::LogicalChannelConfig lcConfig,
                      LteMacSapUser *msu);
  virtual void RemoveLc (uint8_t lcId);
  virtual void Reset ();
  virtual void NotifyConnectionSuccessful ();
  virtual void SetImsi (uint64_t imsi);
  virtual void NotifyEdrx();
  virtual void NotifyPsm();
  virtual void SetMsg5Buffer(uint32_t buffersize);
  virtual NbIotRrcSap::NprachParametersNb::CoverageEnhancementLevel GetCoverageEnhancementLevel();

private:
  LteUeMac *m_mac; ///< the UE MAC
};

UeMemberLteUeCmacSapProvider::UeMemberLteUeCmacSapProvider (LteUeMac *mac) : m_mac (mac)
{
}

void
UeMemberLteUeCmacSapProvider::ConfigureRach (RachConfig rc)
{
  m_mac->DoConfigureRach (rc);
}
void
UeMemberLteUeCmacSapProvider::ConfigureRadioResourceConfig (
    NbIotRrcSap::RadioResourceConfigCommonNb rc)
{
  m_mac->DoConfigureRadioResourceConfig (rc);
}
void
UeMemberLteUeCmacSapProvider::StartContentionBasedRandomAccessProcedure ()
{
  m_mac->DoStartContentionBasedRandomAccessProcedure ();
}
void
UeMemberLteUeCmacSapProvider::StartRandomAccessProcedureNb (bool edt)
{
  m_mac->DoStartRandomAccessProcedureNb (edt);
}
void
UeMemberLteUeCmacSapProvider::StartNonContentionBasedRandomAccessProcedure (uint16_t rnti,
                                                                            uint8_t preambleId,
                                                                            uint8_t prachMask)
{
  m_mac->DoStartNonContentionBasedRandomAccessProcedure (rnti, preambleId, prachMask);
}

void
UeMemberLteUeCmacSapProvider::SetRnti (uint16_t rnti)
{
  m_mac->DoSetRnti (rnti);
}

void
UeMemberLteUeCmacSapProvider::AddLc (uint8_t lcId, LogicalChannelConfig lcConfig,
                                     LteMacSapUser *msu)
{
  m_mac->DoAddLc (lcId, lcConfig, msu);
}

void
UeMemberLteUeCmacSapProvider::RemoveLc (uint8_t lcid)
{
  m_mac->DoRemoveLc (lcid);
}

void
UeMemberLteUeCmacSapProvider::Reset ()
{
  m_mac->DoReset ();
}

void
UeMemberLteUeCmacSapProvider::NotifyConnectionSuccessful ()
{
  m_mac->DoNotifyConnectionSuccessful ();
}

void
UeMemberLteUeCmacSapProvider::SetImsi (uint64_t imsi)
{
  m_mac->DoSetImsi (imsi);
}

void
UeMemberLteUeCmacSapProvider::NotifyEdrx()
{
  m_mac->DoNotifyEdrx();
}
void
UeMemberLteUeCmacSapProvider::NotifyPsm()
{
  m_mac->DoNotifyPsm();
}

void 
UeMemberLteUeCmacSapProvider::SetMsg5Buffer(uint32_t buffersize){
  m_mac->DoSetMsg5Buffer(buffersize);
}

NbIotRrcSap::NprachParametersNb::CoverageEnhancementLevel UeMemberLteUeCmacSapProvider::GetCoverageEnhancementLevel(){
  return m_mac->DoGetCoverageEnhancementLevel();
}

/// UeMemberLteMacSapProvider class
class UeMemberLteMacSapProvider : public LteMacSapProvider
{
public:
  /**
   * Constructor
   *
   * \param mac the UE MAC
   */
  UeMemberLteMacSapProvider (LteUeMac *mac);

  // inherited from LteMacSapProvider
  virtual void TransmitPdu (TransmitPduParameters params);
  virtual void ReportBufferStatus (ReportBufferStatusParameters params);
  virtual void ReportBufferStatusNb (ReportBufferStatusParameters params,
                                     NbIotRrcSap::NpdcchMessage::SearchSpaceType searchspace);
  virtual void ReportNoTransmissionNb(uint16_t rnti, uint8_t lcid);

private:
  LteUeMac *m_mac; ///< the UE MAC
};

UeMemberLteMacSapProvider::UeMemberLteMacSapProvider (LteUeMac *mac) : m_mac (mac)
{
}

void
UeMemberLteMacSapProvider::TransmitPdu (TransmitPduParameters params)
{
  m_mac->DoTransmitPdu (params);
}

void
UeMemberLteMacSapProvider::ReportBufferStatus (ReportBufferStatusParameters params)
{
  m_mac->DoReportBufferStatus (params);
}

void
UeMemberLteMacSapProvider::ReportBufferStatusNb (
    ReportBufferStatusParameters params, NbIotRrcSap::NpdcchMessage::SearchSpaceType searchspace)
{
  m_mac->DoReportBufferStatus (params);
}

void 
UeMemberLteMacSapProvider::ReportNoTransmissionNb(uint16_t rnti, uint8_t lcid){

}

/**
 * UeMemberLteUePhySapUser
 */
class UeMemberLteUePhySapUser : public LteUePhySapUser
{
public:
  /**
   * Constructor
   *
   * \param mac the UE MAC
   */
  UeMemberLteUePhySapUser (LteUeMac *mac);

  // inherited from LtePhySapUser
  virtual void ReceivePhyPdu (Ptr<Packet> p);
  virtual void SubframeIndication (uint32_t frameNo, uint32_t subframeNo);
  virtual void ReceiveLteControlMessage (Ptr<LteControlMessage> msg);
  virtual void NotifyAboutHarqOpportunity (std::vector<std::pair<uint64_t, std::vector<uint64_t>>> subframes);

private:
  LteUeMac *m_mac; ///< the UE MAC
};

UeMemberLteUePhySapUser::UeMemberLteUePhySapUser (LteUeMac *mac) : m_mac (mac)
{
}

void
UeMemberLteUePhySapUser::ReceivePhyPdu (Ptr<Packet> p)
{
  m_mac->DoReceivePhyPdu (p);
}

void
UeMemberLteUePhySapUser::SubframeIndication (uint32_t frameNo, uint32_t subframeNo)
{
  m_mac->DoSubframeIndication (frameNo, subframeNo);
}

void
UeMemberLteUePhySapUser::ReceiveLteControlMessage (Ptr<LteControlMessage> msg)
{
  m_mac->DoReceiveLteControlMessage (msg);
}

void
UeMemberLteUePhySapUser::NotifyAboutHarqOpportunity (
    std::vector<std::pair<uint64_t, std::vector<uint64_t>>> subframes)
{
  m_mac->DoNotifyAboutHarqOpportunity (subframes);
}

//////////////////////////////////////////////////////////
// LteUeMac methods
///////////////////////////////////////////////////////////
void
LteUeMac::SetImsi (uint64_t imsi)
{
  m_imsi = imsi;
}

TypeId
LteUeMac::GetTypeId (void)
{
  static TypeId tid =
      TypeId ("ns3::LteUeMac")
          .SetParent<Object> ()
          .SetGroupName ("Lte")
          .AddConstructor<LteUeMac> ()
          .AddAttribute ("NewSchemaActivated",
                         "Enable ToA-based RAR matching for the new random-access schema.",
                         BooleanValue (false),
                         MakeBooleanAccessor (&LteUeMac::m_newSchemaActivated),
                         MakeBooleanChecker ())
          .AddAttribute ("ToaNumBins",
                         "Number of quantization bins used for ToA matching.",
                         UintegerValue (64),
                         MakeUintegerAccessor (&LteUeMac::m_toaNumBins),
                         MakeUintegerChecker<uint16_t> (2, 4096))
          .AddAttribute ("ToaToleranceBins",
                         "Tolerance in bins for ToA matching.",
                         UintegerValue (1),
                         MakeUintegerAccessor (&LteUeMac::m_toaToleranceBins),
                         MakeUintegerChecker<uint16_t> (0, 64))
          .AddAttribute ("NbRaBackoffEnabled",
                         "Enable random backoff before NB-IoT Msg1 retries after RAR timeout.",
                         BooleanValue (false),
                         MakeBooleanAccessor (&LteUeMac::m_nbRaBackoffEnabled),
                         MakeBooleanChecker ())
          .AddAttribute ("NbRaBackoffMinMs",
                         "Minimum backoff in ms applied before NB-IoT Msg1 retries.",
                         UintegerValue (0),
                         MakeUintegerAccessor (&LteUeMac::m_nbRaBackoffMinMs),
                         MakeUintegerChecker<uint16_t> (0, 10000))
          .AddAttribute ("NbRaBackoffMaxMs",
                         "Maximum backoff in ms applied before NB-IoT Msg1 retries.",
                         UintegerValue (256),
                         MakeUintegerAccessor (&LteUeMac::m_nbRaBackoffMaxMs),
                         MakeUintegerChecker<uint16_t> (0, 10000))
          .AddTraceSource ("RaResponseTimeout", "trace fired upon RA response timeout",
                           MakeTraceSourceAccessor (&LteUeMac::m_raResponseTimeoutTrace),
                           "ns3::LteUeMac::RaResponseTimeoutTracedCallback")

      ;
  return tid;
}

LteUeMac::LteUeMac ()
    : m_bsrPeriodicity (MilliSeconds (1)), // ideal behavior
      m_bsrLast (MilliSeconds (0)),
      m_freshUlBsr (false),
      m_harqProcessId (0),
      m_rnti (0),
      m_imsi (0),
      m_rachConfigured (false),
      m_waitingForRaResponse (false),
      m_transmissionScheduled(false),
      m_listenToSearchSpaces(false),
      m_newSchemaActivated (false),
      m_toaNumBins (64),
      m_toaToleranceBins (1),
      m_pendingScmaMsg3Tag (false),
      m_pendingScmaTcRnti (0),
      m_pendingScmaVirtualId (0),
      m_pendingScmaCodebookId (0),
      m_pendingScmaPhysicalCarrier (0)

{
  NS_LOG_FUNCTION (this);
  m_miUlHarqProcessesPacket.resize (HARQ_PERIOD);
  for (uint8_t i = 0; i < m_miUlHarqProcessesPacket.size (); i++)
    {
      Ptr<PacketBurst> pb = CreateObject<PacketBurst> ();
      m_miUlHarqProcessesPacket.at (i) = pb;
    }
  m_miUlHarqProcessesPacketTimer.resize (HARQ_PERIOD, 0);

  m_macSapProvider = new UeMemberLteMacSapProvider (this);
  m_cmacSapProvider = new UeMemberLteUeCmacSapProvider (this);
  m_uePhySapUser = new UeMemberLteUePhySapUser (this);
  m_raPreambleUniformVariable = CreateObject<UniformRandomVariable> ();
  m_raBackoffUniformVariable = CreateObject<UniformRandomVariable> ();
  m_backoffParameter = 0;
  m_nbRaBackoffEnabled = false;
  m_nbRaBackoffMinMs = 0;
  m_nbRaBackoffMaxMs = 256;
  m_componentCarrierId = 0;
  m_nextIsMsg5 = false;
  m_mac_logging = false;
}

LteUeMac::~LteUeMac ()
{
  NS_LOG_FUNCTION (this);
}

void
LteUeMac::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_miUlHarqProcessesPacket.clear ();
  delete m_macSapProvider;
  delete m_cmacSapProvider;
  delete m_uePhySapUser;
  Object::DoDispose ();
}

LteUePhySapUser *
LteUeMac::GetLteUePhySapUser (void)
{
  return m_uePhySapUser;
}

void
LteUeMac::SetLteUePhySapProvider (LteUePhySapProvider *s)
{
  m_uePhySapProvider = s;
}

LteMacSapProvider *
LteUeMac::GetLteMacSapProvider (void)
{
  return m_macSapProvider;
}

void
LteUeMac::SetLteUeCmacSapUser (LteUeCmacSapUser *s)
{
  m_cmacSapUser = s;
}

LteUeCmacSapProvider *
LteUeMac::GetLteUeCmacSapProvider (void)
{
  return m_cmacSapProvider;
}

void
LteUeMac::SetComponentCarrierId (uint8_t index)
{
  m_componentCarrierId = index;
}
uint64_t 
LteUeMac::GetBufferSize(){
  std::map<uint8_t, LteMacSapProvider::ReportBufferStatusParameters>::iterator it;
  uint64_t buffersize=0;
  for(it = m_ulBsrReceived.begin(); it != m_ulBsrReceived.end(); ++it){
      if((*it).second.lcid > 2){
        uint64_t data_per_lc =((*it).second.txQueueSize + (*it).second.retxQueueSize + (*it).second.statusPduSize);
        buffersize += data_per_lc;
      }
  }
  return buffersize;
}
uint64_t 
LteUeMac::GetBufferSizeComplete(){
  uint64_t buffersize=0;
  std::map<uint8_t, LteMacSapProvider::ReportBufferStatusParameters>::iterator it;
  for(it = m_ulBsrReceived.begin(); it != m_ulBsrReceived.end(); ++it){
        uint64_t data_per_lc =((*it).second.txQueueSize + (*it).second.retxQueueSize + (*it).second.statusPduSize);
        buffersize += data_per_lc;
  }
  return buffersize;
}
void
LteUeMac::DoTransmitPdu (LteMacSapProvider::TransmitPduParameters params)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG (m_rnti == params.rnti, "RNTI mismatch between RLC and MAC");
  LteRadioBearerTag radioTag (params.rnti, params.lcid, 0 /* UE works in SISO mode*/);
  DataVolumeAndPowerHeadroomTag dprTag;
  BufferStatusReportTag bsrTag;
  uint64_t bsr =0;
  //DoSetTransmissionScheduled(false);
  if(m_msg5Buffer > 0){
    // We are just about to send MSG3, add DPR Element for MSG5 (potentially CIoT-Opt)
    //std::cout << " set payload" << std::endl;
    uint8_t dataVolumeIndex = DataVolumeDPR::BufferSize2DVId(m_msg5Buffer);
    m_msg5Buffer = 0;
    m_nextIsMsg5 = true;
    dprTag.SetDataVolumeValue(dataVolumeIndex);
    params.pdu->AddPacketTag(dprTag);
    // Carry UE identity with Msg3 so eNB can recover owner IMSI in legacy/new
    // when RRCConnectionRequest identity is missing at receive side.
    NbIotMsg3ImsiTag msg3ImsiTag;
    msg3ImsiTag.SetImsi (m_imsi);
    params.pdu->AddPacketTag (msg3ImsiTag);

    if (m_newSchemaActivated && m_pendingScmaMsg3Tag)
      {
        NbIotScmaMsg3Tag scmaMsg3Tag;
        scmaMsg3Tag.SetTcRnti (m_pendingScmaTcRnti);
        scmaMsg3Tag.SetVirtualId (m_pendingScmaVirtualId);
        scmaMsg3Tag.SetCodebookId (m_pendingScmaCodebookId);
        scmaMsg3Tag.SetPhysicalCarrier (m_pendingScmaPhysicalCarrier);
        params.pdu->AddPacketTag (scmaMsg3Tag);
        m_pendingScmaMsg3Tag = false;
        NS_LOG_INFO ("[UE][MSG3][TX] imsi=" << m_imsi
                    << " tc-rnti=" << m_pendingScmaTcRnti
                    << " virtualId=" << m_pendingScmaVirtualId
                    << " codebook=" << static_cast<uint32_t> (m_pendingScmaCodebookId)
                    << " physicalCarrier=" << static_cast<uint32_t> (m_pendingScmaPhysicalCarrier));
        if (SaraReport::IsEnabled ())
          {
            SaraReport::LogMsg3Ue (Simulator::GetContext (), m_imsi, params.rnti,
                                   params.lcid, false, m_pendingScmaCodebookId, 0,
                                   m_pendingScmaVirtualId, m_pendingScmaPhysicalCarrier, 1, 0);
          }
      }
    else if (m_saraGroupActive)
      {
        Ptr<UniformRandomVariable> rng = CreateObject<UniformRandomVariable> ();
        uint8_t cd = rng->GetInteger (1, 6);
        uint8_t dmrs = rng->GetInteger (1, 6);

        SaraUlIdTag stag;
        stag.Set (cd, dmrs);
        params.pdu->AddPacketTag (stag);

        NS_LOG_INFO ("[UE][MSG3][TAG-SARA] node=" << Simulator::GetContext ()
                     << " imsi=" << m_imsi
                     << " TC-RNTI=" << (uint32_t) params.rnti
                     << " lcid=" << (uint32_t) params.lcid
                     << " cd=" << (uint32_t) cd
                     << " dmrs=" << (uint32_t) dmrs);
        if (SaraReport::IsEnabled ())
          {
            SaraReport::LogMsg3Ue (Simulator::GetContext (), m_imsi, params.rnti,
                                   params.lcid, true, cd, dmrs);
          }
      }
    else
      {
        NS_LOG_INFO ("[UE][MSG3][NO-SARA] node=" << Simulator::GetContext ()
                     << " imsi=" << m_imsi
                     << " TC-RNTI=" << (uint32_t) params.rnti
                     << " lcid=" << (uint32_t) params.lcid
                     << " (sin cd/dmrs)");
        if (SaraReport::IsEnabled ())
          {
            SaraReport::LogMsg3Ue (Simulator::GetContext (), m_imsi, params.rnti,
                                   params.lcid, false, 0, 0);
          }
      }
  }
  else{

    bsr = GetBufferSizeComplete();
    if(bsr > 0){

      bsrTag.SetBufferStatusReportIndex(BufferSizeLevelBsr::BufferSize2BsrId (bsr));
      params.pdu->AddPacketTag(bsrTag);
    }
    // Normal PDU just add BSR for next Packet
  }
  
  params.pdu->AddPacketTag (radioTag);
  // store pdu in HARQ buffer
  //m_miUlHarqProcessesPacket.at (m_harqProcessId)->AddPacket (params.pdu);
  //m_miUlHarqProcessesPacketTimer.at (m_harqProcessId) = HARQ_PERIOD;
  m_uePhySapProvider->SendMacPdu (params.pdu);
}

void
LteUeMac::DoReportBufferStatus (LteMacSapProvider::ReportBufferStatusParameters params)
{
  NS_LOG_FUNCTION (this << (uint32_t) params.lcid);

  std::map<uint8_t, LteMacSapProvider::ReportBufferStatusParameters>::iterator it;

  it = m_ulBsrReceived.find (params.lcid);
  if (it != m_ulBsrReceived.end ())
    {
      // update entry
      (*it).second = params;
    }
  else
    {
      m_ulBsrReceived.insert (std::pair<uint8_t, LteMacSapProvider::ReportBufferStatusParameters> (
          params.lcid, params));
    }
  m_freshUlBsr = true;
}

void
LteUeMac::SendReportBufferStatus (void)
{
  NS_LOG_FUNCTION (this);

  if (m_rnti == 0)
    {
      NS_LOG_INFO ("MAC not initialized, BSR deferred");
      return;
    }

  if (m_ulBsrReceived.size () == 0)
    {
      NS_LOG_INFO ("No BSR report to transmit");
      return;
    }
  MacCeListElement_s bsr;
  bsr.m_rnti = m_rnti;
  bsr.m_macCeType = MacCeListElement_s::BSR;

  // BSR is reported for each LCG
  std::map<uint8_t, LteMacSapProvider::ReportBufferStatusParameters>::iterator it;
  std::vector<uint32_t> queue (4, 0); // one value per each of the 4 LCGs, initialized to 0
  for (it = m_ulBsrReceived.begin (); it != m_ulBsrReceived.end (); it++)
    {
      uint8_t lcid = it->first;
      std::map<uint8_t, LcInfo>::iterator lcInfoMapIt;
      lcInfoMapIt = m_lcInfoMap.find (lcid);
      NS_ASSERT (lcInfoMapIt != m_lcInfoMap.end ());
      NS_ASSERT_MSG ((lcid != 0) ||
                         (((*it).second.txQueueSize == 0) && ((*it).second.retxQueueSize == 0) &&
                          ((*it).second.statusPduSize == 0)),
                     "BSR should not be used for LCID 0");
      uint8_t lcg = lcInfoMapIt->second.lcConfig.logicalChannelGroup;
      queue.at (lcg) +=
          ((*it).second.txQueueSize + (*it).second.retxQueueSize + (*it).second.statusPduSize);
    }

  // FF API says that all 4 LCGs are always present
  bsr.m_macCeValue.m_bufferStatus.push_back (BufferSizeLevelBsr::BufferSize2BsrId (queue.at (0)));
  bsr.m_macCeValue.m_bufferStatus.push_back (BufferSizeLevelBsr::BufferSize2BsrId (queue.at (1)));
  bsr.m_macCeValue.m_bufferStatus.push_back (BufferSizeLevelBsr::BufferSize2BsrId (queue.at (2)));
  bsr.m_macCeValue.m_bufferStatus.push_back (BufferSizeLevelBsr::BufferSize2BsrId (queue.at (3)));

  // create the feedback to eNB
  Ptr<BsrLteControlMessage> msg = Create<BsrLteControlMessage> ();
  msg->SetBsr (bsr);
  m_uePhySapProvider->SendLteControlMessage (msg);
}

void
LteUeMac::RandomlySelectAndSendRaPreamble ()
{
  NS_LOG_FUNCTION (this);
  // 3GPP 36.321 5.1.1
  NS_ASSERT_MSG (m_rachConfigured, "RACH not configured");
  // assume that there is no Random Access Preambles group B
  m_raPreambleId =
      m_raPreambleUniformVariable->GetInteger (0, m_rachConfig.numberOfRaPreambles - 1);
  bool contention = true;
  SendRaPreamble (contention);
}
//prepara mensaje 1 para NB-IoT
void
LteUeMac::RandomlySelectAndSendRaPreambleNb ()
{
  NS_LOG_FUNCTION (this);
  // 3GPP 36.321 5.1.1
  NS_ASSERT_MSG (m_nprachConfigured, "NPRACH not configured");
  //Se escoge de manera uniforme entre 12 subportadoras displnibles 
  m_raPreambleId = m_raPreambleUniformVariable->GetInteger (0, NbIotRrcSap::ConvertNprachNumSubcarriers2int (m_CeLevel) - 1);
  bool contention = true;

  uint32_t currentsubframe = Simulator::Now().GetMilliSeconds();
  uint16_t window_condition = ( currentsubframe/10) % (NbIotRrcSap::ConvertNprachPeriodicity2int (m_CeLevel) / 10);
  uint32_t lastPeriodStart = (currentsubframe/10) - window_condition;
  uint32_t startSubframeNprachOccasion = lastPeriodStart*10 + NbIotRrcSap::ConvertNprachStartTime2int(m_CeLevel);
  if (startSubframeNprachOccasion != currentsubframe)
    {
      uint16_t subframesToWait = 0;
      if(currentsubframe < startSubframeNprachOccasion){
        subframesToWait = startSubframeNprachOccasion-currentsubframe;
      }else{
        subframesToWait = (NbIotRrcSap::ConvertNprachPeriodicity2int(m_CeLevel) - (currentsubframe % NbIotRrcSap::ConvertNprachPeriodicity2int(m_CeLevel)))+NbIotRrcSap::ConvertNprachStartTime2int(m_CeLevel);
      }

      //uint16_t frames_to_wait = (NbIotRrcSap::ConvertNprachPeriodicity2int (m_CeLevel) - window_condition*10) + (10-(m_subframeNo-1))%10;
      //NS_BUILD_DEBUG(std::cout << m_frameNo*10+m_subframeNo << std::endl);
      m_logging.push_back(currentsubframe+subframesToWait);
      //NS_BUILD_DEBUG(std::cout  << "Frames to wait:" << subframesToWait << std::endl);
      Simulator::Schedule (MilliSeconds (subframesToWait), &LteUeMac::SendRaPreambleNb, this,
                           contention);
    }
  else{
    SendRaPreambleNb(contention);
  }

}

void
LteUeMac::SendRaPreamble (bool contention)
{
  NS_LOG_FUNCTION (this << (uint32_t) m_raPreambleId << contention);
  // Since regular UL LteControlMessages need m_ulConfigured = true in
  // order to be sent by the UE, the rach preamble needs to be sent
  // with a dedicated primitive (not
  // m_uePhySapProvider->SendLteControlMessage (msg)) so that it can
  // bypass the m_ulConfigured flag. This is reasonable, since In fact
  // the RACH preamble is sent on 6RB bandwidth so the uplink
  // bandwidth does not need to be configured.
  NS_ASSERT (m_subframeNo > 0); // sanity check for subframe starting at 1
  m_raRnti = m_subframeNo - 1;
  m_uePhySapProvider->SendRachPreamble (m_raPreambleId, m_raRnti);
  NS_LOG_INFO (this << " sent preamble id " << (uint32_t) m_raPreambleId << ", RA-RNTI "
                    << (uint32_t) m_raRnti);
  if (SaraReport::IsEnabled ())
    {
      SaraReport::LogMsg1 (Simulator::GetContext (), m_imsi, m_raPreambleId, m_raRnti);
    }
  // 3GPP 36.321 5.1.4
  //Time raWindowBegin = MilliSeconds (3);
  //Time raWindowEnd = MilliSeconds (3 + m_rachConfig.raResponseWindowSize);
  //Simulator::Schedule (raWindowBegin, &LteUeMac::StartWaitingForRaResponse, this);
  //m_noRaResponseReceivedEvent = Simulator::Schedule (raWindowEnd, &LteUeMac::RaResponseTimeout, this, contention);
}
//envia mensaje 1 para NB-IoT
void
LteUeMac::SendRaPreambleNb (bool contention)
{
  NS_LOG_FUNCTION (this << (uint32_t) m_raPreambleId << contention);

  NS_ASSERT (m_frameNo > 0); // sanity check for subframe starting at 1

  //Norma para calcular RA-RTNI ETSI 36.321 5.1.4
  //la ecuacion garantiza que el raRnti se mantenga constante durante 4 subframes consecutivos, que es el numero de subframes entre dos ocasiones de nprach para un mismo CE level, necesario para que el UE pueda escuchar las search spaces correspondientes a su CE level durante toda la ventana de recepcion del RAR
  m_raRnti = 1 + floor (m_frameNo / 4);

  //Cp estandar dado por la norma para nprach
  m_radioResourceConfig.nprachConfig.nprachCpLength =
      NbIotRrcSap::NprachConfig::NprachCpLength::us266dot7;
  //cada 15khz hay una subportadora, el simbolo de preambulo esta formado por 2048 subportadoras (IFFT), cada una lleva en si un simbolo M-ARIO, por lo que la frecuencia de muestreo es de 15khz*2048, y el tiempo de simbolo es el inverso de la frecuencia de muestreo
  double ts = 1000.0 / (15000.0 * 2048.0); // duracion de una muestra en ms
  double preambleSymbolTime = 8192.0 * ts; // duracion del simbolo nprach en 3.75khz 
  double preambleGroupTimeNoCP = 5.0 * preambleSymbolTime; // el preambulo esta formado por 5 simbolos nprach
  double preambleGroupTime =
      NbIotRrcSap::ConvertNprachCpLenght2double (m_radioResourceConfig.nprachConfig) +
      preambleGroupTimeNoCP;
  double preambleRepetition = 4.0 * preambleGroupTime; // el preambulo se repite 4 veces segun la norma para nprach
  const uint16_t ceRepetitions =
      NbIotRrcSap::ConvertNumRepetitionsPerPreambleAttempt2int (m_CeLevel);
  double time = ceRepetitions * preambleRepetition; // repeticiones segun el nivel de covertura
  
  m_cmacSapUser->NotifyEnergyState(NbiotEnergyModel::PowerState::RRC_CONNECTED_SENDING_NPRACH);
  //Schedule EnergyStateChange on the next subframe after transmission
  Simulator::Schedule (MilliSeconds (time+1), &LteUeCmacSapUser::NotifyEnergyState, m_cmacSapUser, NbiotEnergyModel::PowerState::RRC_CONNECTED_IDLE); // 1ms despues de enviar el preambulo se cambia a estado idle, que es el estado en el que el UE se queda escuchando el RAR durante la ventana de recepcion del RAR y se modela consumo de energia por estados sostenidos x tiempo
  Simulator::Schedule (MilliSeconds (time), &LteUePhySapProvider::SendNprachPreamble,
                       m_uePhySapProvider, m_raPreambleId, m_raRnti,
                       NbIotRrcSap::ConvertNprachSubcarrierOffset2int (m_CeLevel)); //se envia el preambulo a phy
  const uint16_t rapid =
      static_cast<uint16_t> (NbIotRrcSap::ConvertNprachSubcarrierOffset2int (m_CeLevel) + m_raPreambleId); //offser para separar preambulos de diferentes niveles de covertua CE
  NS_LOG_INFO ("[UE][MSG1][TX] imsi=" << m_imsi
               << " preamble=" << static_cast<uint32_t> (m_raPreambleId)
               << " rapid=" << rapid
               << " ra-rnti=" << static_cast<uint32_t> (m_raRnti)
               << " ceLevel=" << static_cast<uint32_t> (m_CeLevel.coverageEnhancementLevel)
               << " ceRepetitions=" << ceRepetitions
               << " nprachTxDurationMs=" << time);
  if (SaraReport::IsEnabled ())
    {
      SaraReport::LogMsg1 (
          Simulator::GetContext (), m_imsi, m_raPreambleId, m_raRnti,
          static_cast<int32_t> (m_CeLevel.coverageEnhancementLevel), static_cast<int32_t> (rapid));
    }

  if (m_mac_logging)
  {
    LogMessage("SendRaPreambleNb");
  }

  // 3GPP 36.321 5.1.4
  //despues de enviar el preambulo se calcula la ventana de recepcion del RAR, que depende del nivel de cobertura CE, y se programa el timeout para el caso en el que no se reciba ningun RAR dentro de esa ventana
  Time raWindowBegin;
  Time raWindowEnd;
  uint32_t npdcchPeriod = NbIotRrcSap::ConvertNpdcchNumRepetitionsRa2int (m_CeLevel) *
                          NbIotRrcSap::ConvertNpdcchStartSfCssRa2double (m_CeLevel); // tamaño del periodo entre inicios de bloques NPDCCH con el RAR, se calcula en funcion del tamaño del bloque (numero de repeticiones de subframe segun el (CE) y start que me dice cada cuanto tiempo inicia el siguiente bloque en relacion al tamaño del mismo 

  if (NbIotRrcSap::ConvertNumRepetitionsPerPreambleAttempt2int (m_CeLevel) >= 64)
    {
      raWindowBegin = MilliSeconds (41);
      //NS_BUILD_DEBUG(std::cout << (m_frameNo - 1) * 10 + (m_subframeNo - 1) + time + 41 + NbIotRrcSap::ConvertRaResponseWindowSize2int (m_rachConfigCe) * npdcchPeriod << std::endl);
      raWindowEnd = MilliSeconds (
          time + 41 + NbIotRrcSap::ConvertRaResponseWindowSize2int (m_rachConfigCe) * npdcchPeriod); //calcula la cantidad total de subtramas que el ue debe esperar antes de dejar de escuchar npdcch
    }
  else
    {
      raWindowBegin = MilliSeconds (4);
      //NS_BUILD_DEBUG(std::cout << (m_frameNo - 1) * 10 + (m_subframeNo - 1) + time + 4 + NbIotRrcSap::ConvertRaResponseWindowSize2int (m_rachConfigCe) * npdcchPeriod << std::endl);
      raWindowEnd = MilliSeconds (
          time + 4 + NbIotRrcSap::ConvertRaResponseWindowSize2int (m_rachConfigCe) * npdcchPeriod);
    }
  //Time raWindowEnd = MilliSeconds (4 + 8*10240);
  //Time raWindowEnd = MilliSeconds (4 + m_rachConfig.raResponseWindowSize);
  //NS_BUILD_DEBUG(std::cout << (m_frameNo - 1) * 10 + (m_subframeNo - 1) + time << std::endl);

  //una vez calculada la ventana de recepcion de rar, se disparan los eventos
  Simulator::Schedule (raWindowBegin, &LteUeMac::StartWaitingForRaResponse, this); //avisa a las capas inferiores phy que el ue quiere escuchar npdcch
  m_listenToSearchSpaces = true;
  m_noRaResponseReceivedEvent =
      Simulator::Schedule (raWindowEnd, &LteUeMac::RaResponseTimeoutNb, this, contention); // programa el timeout para el caso en el que no se reciba ningun RAR dentro de la ventana de recepcion
}
void
LteUeMac::StartWaitingForRaResponse ()
{
  NS_LOG_FUNCTION (this);
  m_waitingForRaResponse = true;
}
void
LteUeMac::StartWaitingForRaResponseNb ()
{
  NS_LOG_FUNCTION (this);
  m_waitingForRaResponse = true;
}

void
LteUeMac::RecvRaResponse (BuildRarListElement_s raResponse)
{
  NS_LOG_FUNCTION (this);
  m_waitingForRaResponse = false;
  m_noRaResponseReceivedEvent.Cancel ();
  NS_LOG_INFO ("got RAR for RAPID " << (uint32_t) m_raPreambleId
                                    << ", setting T-C-RNTI = " << raResponse.m_rnti);
  m_rnti = raResponse.m_rnti;
  m_cmacSapUser->SetTemporaryCellRnti (m_rnti);
  // in principle we should wait for contention resolution,
  // but in the current LTE model when two or more identical
  // preambles are sent no one is received, so there is no need
  // for contention resolution
  m_cmacSapUser->NotifyRandomAccessSuccessful (false);
  // trigger tx opportunity for Message 3 over LC 0
  // this is needed since Message 3's UL GRANT is in the RAR, not in UL-DCIs
  const uint8_t lc0Lcid = 0;
  std::map<uint8_t, LcInfo>::iterator lc0InfoIt = m_lcInfoMap.find (lc0Lcid);
  NS_ASSERT (lc0InfoIt != m_lcInfoMap.end ());
  std::map<uint8_t, LteMacSapProvider::ReportBufferStatusParameters>::iterator lc0BsrIt =
      m_ulBsrReceived.find (lc0Lcid);
  if ((lc0BsrIt != m_ulBsrReceived.end ()) && (lc0BsrIt->second.txQueueSize > 0))
    {
      NS_ASSERT_MSG (raResponse.m_grant.m_tbSize > lc0BsrIt->second.txQueueSize,
                     "segmentation of Message 3 is not allowed");
      // this function can be called only from primary carrier
      if (m_componentCarrierId > 0)
        {
          NS_FATAL_ERROR ("Function called on wrong componentCarrier");
        }
      LteMacSapUser::TxOpportunityParameters txOpParams;
      txOpParams.bytes = raResponse.m_grant.m_tbSize;
      txOpParams.layer = 0;
      txOpParams.harqId = 0;
      txOpParams.componentCarrierId = m_componentCarrierId;
      txOpParams.rnti = m_rnti;
      txOpParams.lcid = lc0Lcid;
      lc0InfoIt->second.macSapUser->NotifyTxOpportunity (txOpParams);
      lc0BsrIt->second.txQueueSize = 0;
    }
}

void
LteUeMac::RecvRaResponseNb (NbIotRrcSap::Rar raResponse)
{
  NS_LOG_FUNCTION (this);
  m_waitingForRaResponse = false;
  m_noRaResponseReceivedEvent.Cancel ();
  const uint32_t msg3GrantCount =
      static_cast<uint32_t> (raResponse.rarPayload.ulGrant.subframes.second.size ());
  const uint32_t msg3GrantFirst =
      msg3GrantCount > 0 ? static_cast<uint32_t> (raResponse.rarPayload.ulGrant.subframes.second.front ())
                         : 0;
  const uint32_t msg3GrantLast =
      msg3GrantCount > 0 ? static_cast<uint32_t> (raResponse.rarPayload.ulGrant.subframes.second.back ())
                         : 0;
  NS_LOG_INFO ("[UE][MSG2][SELECT] imsi=" << m_imsi
               << " preamble=" << static_cast<uint32_t> (m_raPreambleId)
               << " tc-rnti=" << raResponse.rarPayload.cellRnti
               << " toaValid=" << (raResponse.toaValid ? "1" : "0")
               << " toaBin=" << static_cast<uint32_t> (raResponse.toaBin)
               << " virtualId=" << static_cast<uint32_t> (raResponse.virtualId)
               << " codebook=" << static_cast<uint32_t> (raResponse.codebookId)
               << " physicalCarrier="
               << static_cast<uint32_t> (raResponse.rarPayload.ulGrant.subframes.first)
               << " msg3GrantCount=" << msg3GrantCount
               << " msg3GrantFirst=" << msg3GrantFirst
               << " msg3GrantLast=" << msg3GrantLast);

                                    
  if (m_mac_logging)
  {
    std::string msg = "RecvRaResponseNb,cellRNTI," + std::to_string(raResponse.rarPayload.cellRnti) + ",";
    LogMessage(msg);
  }

  m_rnti = raResponse.rarPayload.cellRnti;
  m_cmacSapUser->SetTemporaryCellRnti (m_rnti);
  if (m_newSchemaActivated)
    {
      m_pendingScmaMsg3Tag = true;
      m_pendingScmaTcRnti = raResponse.rarPayload.cellRnti;
      m_pendingScmaVirtualId = raResponse.virtualId;
      m_pendingScmaCodebookId = raResponse.codebookId;
      m_pendingScmaPhysicalCarrier = raResponse.rarPayload.ulGrant.subframes.first;
    }
  else
    {
      m_pendingScmaMsg3Tag = false;
    }
  if (m_newSchemaActivated)
    {
      // In the new schema we clear RA identifiers right after RAR acceptance.
      // Legacy/SARA keep original rc3 behavior.
      m_raPreambleId = 255;
      m_raRnti = 11;
    }
  // in principle we should wait for contention resolution,
  // but in the current LTE model when two or more identical
  // preambles are sent no one is received, so there is no need
  // for contention resolution

  // To be comented in
  bool edt;
  if(raResponse.rarPayload.ulGrant.tbs_size > 88){
    // We got a grant for EDT 
    edt = true;
  }else{
    edt = false;
  }
  m_cmacSapUser->NotifyRandomAccessSuccessful (edt);

  // trigger tx opportunity for Message 3 over LC 0
  // this is needed since Message 3's UL GRANT is in the RAR, not in UL-DCIs
  const uint8_t lc0Lcid = 0;
  std::map<uint8_t, LcInfo>::iterator lc0InfoIt = m_lcInfoMap.find (lc0Lcid);
  NS_ASSERT (lc0InfoIt != m_lcInfoMap.end ());
  std::map<uint8_t, LteMacSapProvider::ReportBufferStatusParameters>::iterator lc0BsrIt =
      m_ulBsrReceived.find (lc0Lcid);
  if ((lc0BsrIt != m_ulBsrReceived.end ()) && (lc0BsrIt->second.txQueueSize > 0))
    {
      // NS_ASSERT_MSG (raResponse.m_grant.m_tbSize > lc0BsrIt->second.txQueueSize,
      //               "segmentation of Message 3 is not allowed");
      // this function can be called only from primary carrier
      if (m_componentCarrierId > 0)
        {
          NS_FATAL_ERROR ("Function called on wrong componentCarrier");
        }
      LteMacSapUser::TxOpportunityParameters txOpParams;



      txOpParams.bytes = raResponse.rarPayload.ulGrant.tbs_size/8;
      txOpParams.layer = 0;
      txOpParams.harqId = 0;
      txOpParams.componentCarrierId = m_componentCarrierId;
      txOpParams.rnti = m_rnti;
      txOpParams.lcid = lc0Lcid;
      int subframes = raResponse.rarPayload.ulGrant.subframes.second.back() -
                      (10 * (m_frameNo - 1) + m_subframeNo - 1);
      const bool scheduleMsg3FromMac = (m_newSchemaActivated || m_saraGroupActive);
      if (scheduleMsg3FromMac)
        {
          const uint8_t subcarrier = raResponse.rarPayload.ulGrant.subframes.first;
          // MAC owns the final RAR acceptance decision:
          // - new: RAPID + ToA
          // - SARA grouped: RAPID + tag
          // Only now we program Msg3 UL resources in PHY.
          m_uePhySapProvider->ScheduleNprachMsg3Transmission (
              subcarrier,
              static_cast<uint32_t> (std::max (0, subframes)));

          // Keep RSRP/CQI feedback tied to the accepted RAR only.
          Ptr<DlCqiLteControlMessage> report = Create<DlCqiLteControlMessage> ();
          CqiListElement_s dlcqi;
          dlcqi.m_rnti = raResponse.rarPayload.cellRnti;
          dlcqi.m_ri = 1;
          dlcqi.m_cqiType = CqiListElement_s::P10;
          report->SetDlCqi (dlcqi);
          report->rsrp = m_uePhySapProvider->GetRSRP ();
          m_uePhySapProvider->SendLteControlMessage (report);
        }

      uint32_t subframesTillNpusch = raResponse.rarPayload.ulGrant.subframes.second.front() - (10*(m_frameNo-1)+m_subframeNo-1);

      m_transmissionScheduled = true;
      Simulator::Schedule(MilliSeconds(subframesTillNpusch), &LteUeCmacSapUser::NotifyEnergyState, m_cmacSapUser, NbiotEnergyModel::PowerState::RRC_CONNECTED_SENDING_NPUSCH);

      //EnergyStateChange on the next Subframe after Transmission Completed
      Simulator::Schedule(MilliSeconds(subframes+1), &LteUeCmacSapUser::NotifyEnergyState, m_cmacSapUser, NbiotEnergyModel::PowerState::RRC_CONNECTED_IDLE);

      //Simulator::Schedule (MilliSeconds (subframes), &LteMacSapUser::NotifyTxOpportunity,
      //                     lc0InfoIt->second.macSapUser, txOpParams);
      lc0InfoIt->second.macSapUser->NotifyTxOpportunityNb(txOpParams,subframes);
      lc0BsrIt->second.txQueueSize = 0;
    }
}

void
LteUeMac::RaResponseTimeout (bool contention)
{
  NS_LOG_FUNCTION (this << contention);
  m_waitingForRaResponse = false;
  // 3GPP 36.321 5.1.4
  ++m_preambleTransmissionCounter;
  //fire RA response timeout trace
  m_raResponseTimeoutTrace (m_imsi, contention, m_preambleTransmissionCounter,
                            m_rachConfig.preambleTransMax + 1);
  if (m_preambleTransmissionCounter == m_rachConfig.preambleTransMax + 1)
    {
      NS_LOG_INFO ("RAR timeout, preambleTransMax reached => giving up");
      m_cmacSapUser->NotifyRandomAccessFailed ();
    }
  else
    {
      NS_LOG_INFO ("RAR timeout, re-send preamble");
      if (contention)
        {
          RandomlySelectAndSendRaPreamble ();
        }
      else
        {
          SendRaPreamble (contention);
        }
    }
}

void
LteUeMac::RaResponseTimeoutNb (bool contention)
{
  // Based on ETSI TS 136 321 V13.9.0, 5.1.4: Random Access Response reception:
  // When RAP fails, and the MaxNumPreambleAttemptCE counter is reached, the UE resets MaxNumPreambleAttemptCE 
  // and retries in the next CE level, until preambleTransMax is reached
  NS_LOG_FUNCTION (this << contention);
  m_waitingForRaResponse = false;
  m_saraGroupActive = false;
  m_saraGroupSize = 1;
  m_saraTag = 0;
  m_saraDesiredTagSet = false;
  m_saraDesiredTag = 0;
  m_saraWaitingForTag = false;
  //NS_BUILD_DEBUG(std::cout << "Window End" << std::endl);
  // 3GPP 36.321 5.1.4
  ++m_preambleTransmissionCounter;
  ++m_preambleTransmissionCounterCe;
  //fire RA response timeout trace
  m_raResponseTimeoutTrace (m_imsi, contention, m_preambleTransmissionCounter,
                            m_rachConfig.preambleTransMax);
  if (m_preambleTransmissionCounter == m_radioResourceConfig.rachConfigCommon.preambleTransMaxCE)
    {
      NS_LOG_INFO ("RAR timeout, preambleTransMax reached => giving up");
      LogMessage("LteUeMac::RaResponseTimeoutNb,RAR timeout: preambleTransMax reached");

      m_cmacSapUser->NotifyRandomAccessFailed ();
    }
  else
    {
      if (m_preambleTransmissionCounterCe ==
      NbIotRrcSap::ConvertMaxNumPreambleAttemptCE2int (m_CeLevel)) // Max. number of retries in this CE level reached
        {
          m_preambleTransmissionCounterCe = 0;
          NbIotRrcSap::NprachParametersNbR14 tmp = {}; // needed if EDT is enabled

          if (m_CeLevel.coverageEnhancementLevel == m_radioResourceConfig.nprachConfig.nprachParametersList.nprachParametersNb0.coverageEnhancementLevel) // CE0
            {
              // Increase to CE1
              NS_LOG_INFO ("RAR timeout, MaxNumPreambleAttemptCE reached => increasing CE level to CE1");
              LogMessage("LteUeMac::RaResponseTimeoutNb,RAR timeout, MaxNumPreambleAttemptCE reached => increasing CE level to CE1");              
              m_CeLevel = m_radioResourceConfig.nprachConfig.nprachParametersList.nprachParametersNb1;
              m_rachConfigCe = m_radioResourceConfig.rachConfigCommon.rachInfoList.rachInfo2;
              if(m_edt){
                tmp = m_radioResourceConfig.nprachConfigR15.nprachParameterListEdt.nprachParametersNb1;
              }
            }
          else if (m_CeLevel.coverageEnhancementLevel == m_radioResourceConfig.nprachConfig.nprachParametersList.nprachParametersNb1.coverageEnhancementLevel) // CE1
            {
              // Increase to CE2
              NS_LOG_INFO ("RAR timeout, MaxNumPreambleAttemptCE reached => increasing CE level to CE2");
              LogMessage("LteUeMac::RaResponseTimeoutNb,RAR timeout, MaxNumPreambleAttemptCE reached => increasing CE level to CE2");   
              m_CeLevel = m_radioResourceConfig.nprachConfig.nprachParametersList.nprachParametersNb2;
              m_rachConfigCe = m_radioResourceConfig.rachConfigCommon.rachInfoList.rachInfo3;
              if(m_edt){
                tmp = m_radioResourceConfig.nprachConfigR15.nprachParameterListEdt.nprachParametersNb2;
              }
            }
          else if (m_CeLevel.coverageEnhancementLevel == m_radioResourceConfig.nprachConfig.nprachParametersList.nprachParametersNb2.coverageEnhancementLevel) // CE2
            {
              // Can't increase further
              NS_LOG_INFO ("RAR timeout, MaxNumPreambleAttemptCE in CE2 reached => giving up");
              LogMessage("LteUeMac::RaResponseTimeoutNb,RAR timeout, MaxNumPreambleAttemptCE in CE2 reached => giving up");  
              m_cmacSapUser->NotifyRandomAccessFailed ();
              return;
            }

          if(m_edt){
            // Overwrite R13 config with values for R15 EDT provided
            // easiest way to access data not include in NprachParameterNBR14
            m_CeLevel.coverageEnhancementLevel= tmp.coverageEnhancementLevel;
            m_CeLevel.nprachPeriodicity = tmp.nprachPeriodicity; 
            m_CeLevel.nprachStartTime = tmp.nprachStartTime;
            m_CeLevel.nprachSubcarrierOffset = tmp.nprachSubcarrierOffset;
            m_CeLevel.nprachNumSubcarriers = tmp.nprachNumSubcarriers;
            m_CeLevel.nprachSubcarrierMsg3RangeStart= tmp.nprachSubcarrierMsg3RangeStart;
            m_CeLevel.npdcchNumRepetitionsRA = tmp.npdcchNumRepetitionsRA;
            m_CeLevel.npdcchStartSfCssRa = tmp.npdcchStartSfCssRa;
            m_CeLevel.npdcchOffsetRa = tmp.npdcchOffsetRa;

          }
        }
      uint16_t backoffMs = 0;
      if (m_nbRaBackoffEnabled)
        {
          const uint16_t minBackoffMs = std::min<uint16_t> (m_nbRaBackoffMinMs, m_nbRaBackoffMaxMs);
          const uint16_t maxBackoffMs = std::max<uint16_t> (m_nbRaBackoffMinMs, m_nbRaBackoffMaxMs);
          if (minBackoffMs == maxBackoffMs)
            {
              backoffMs = minBackoffMs;
            }
          else
            {
              backoffMs = static_cast<uint16_t> (m_raBackoffUniformVariable->GetInteger (minBackoffMs, maxBackoffMs));
            }
        }
      m_backoffParameter = backoffMs;
      NS_LOG_INFO ("RAR timeout, re-send preamble"
                   << " backoffMs=" << static_cast<uint32_t> (backoffMs));
      NS_LOG_INFO ("[UE][MSG1][RETRY] imsi=" << m_imsi
                  << " preambleTxCounter=" << static_cast<uint32_t> (m_preambleTransmissionCounter)
                  << " ceLevel="
                  << static_cast<uint32_t> (m_CeLevel.coverageEnhancementLevel)
                  << " backoffMs=" << static_cast<uint32_t> (backoffMs));
      LogMessage("LteUeMac::RaResponseTimeoutNb,RAR timeout, re-send preamble");  
      if (backoffMs == 0)
        {
          if (contention)
            {
              RandomlySelectAndSendRaPreambleNb ();
            }
          else
            {
              SendRaPreambleNb (contention);
            }
        }
      else
        {
          if (contention)
            {
              Simulator::Schedule (MilliSeconds (backoffMs),
                                   &LteUeMac::RandomlySelectAndSendRaPreambleNb,
                                   this);
            }
          else
            {
              Simulator::Schedule (MilliSeconds (backoffMs),
                                   &LteUeMac::SendRaPreambleNb,
                                   this,
                                   contention);
            }
        }
    }
}

void
LteUeMac::DoConfigureRach (LteUeCmacSapProvider::RachConfig rc)
{
  NS_LOG_FUNCTION (this);
  m_rachConfig = rc;
  m_rachConfigured = true;
}
void
LteUeMac::DoConfigureRadioResourceConfig (NbIotRrcSap::RadioResourceConfigCommonNb rc)
{
  NS_LOG_FUNCTION (this);
  m_radioResourceConfig = rc;
  m_nprachConfigured = true;
}
void
LteUeMac::DoStartContentionBasedRandomAccessProcedure ()
{
  NS_LOG_FUNCTION (this);

  // 3GPP 36.321 5.1.1
  NS_ASSERT_MSG (m_rachConfigured, "RACH not configured");
  m_preambleTransmissionCounter = 0;
  m_backoffParameter = 0;
  m_saraGroupActive = false;
  m_saraGroupSize = 1;
  m_saraTag = 0;
  m_saraDesiredTagSet = false;
  m_saraDesiredTag = 0;
  m_saraWaitingForTag = false;
  RandomlySelectAndSendRaPreamble ();
}
void
LteUeMac::DoStartRandomAccessProcedureNb (bool edt)
{
  NS_LOG_FUNCTION (this);

  // 3GPP 36.321 5.1.1
  NS_ASSERT_MSG (m_nprachConfigured, "RACH not configured");
  m_preambleTransmissionCounter = 0;
  m_preambleTransmissionCounterCe = 0;
  m_edt = edt;
  m_saraGroupActive = false;
  m_saraGroupSize = 1;
  m_saraTag = 0;
  m_saraDesiredTagSet = false;
  m_saraDesiredTag = 0;
  m_saraWaitingForTag = false;
  // Check CE Level
  double rsrp = m_uePhySapProvider->GetRSRP ();
  //NS_BUILD_DEBUG (std::cout << "RSRP: " << rsrp << "dBm" << std::endl);

  NbIotRrcSap::NprachParametersNbR14 tmp = {}; // needed if EDT is enabled

  if (rsrp <= m_radioResourceConfig.nprachConfig.rsrpThresholdsPrachInfoList.ce2_lowerbound)
    {
      // CE2
      m_CeLevel = m_radioResourceConfig.nprachConfig.nprachParametersList.nprachParametersNb2;
      m_rachConfigCe = m_radioResourceConfig.rachConfigCommon.rachInfoList.rachInfo3;
      if(m_edt){
        tmp = m_radioResourceConfig.nprachConfigR15.nprachParameterListEdt.nprachParametersNb2;
      }
    }
  else if (rsrp <= m_radioResourceConfig.nprachConfig.rsrpThresholdsPrachInfoList.ce1_lowerbound)
    {
      // CE1
      m_CeLevel = m_radioResourceConfig.nprachConfig.nprachParametersList.nprachParametersNb1;
      m_rachConfigCe = m_radioResourceConfig.rachConfigCommon.rachInfoList.rachInfo2;
      if(m_edt){
        tmp = m_radioResourceConfig.nprachConfigR15.nprachParameterListEdt.nprachParametersNb1;
      }
    }
  else if (rsrp > m_radioResourceConfig.nprachConfig.rsrpThresholdsPrachInfoList.ce1_lowerbound)
    {
      // CE0
      m_CeLevel = m_radioResourceConfig.nprachConfig.nprachParametersList.nprachParametersNb0;
      m_rachConfigCe = m_radioResourceConfig.rachConfigCommon.rachInfoList.rachInfo1;
      if(m_edt){
        tmp = m_radioResourceConfig.nprachConfigR15.nprachParameterListEdt.nprachParametersNb0;
      }
    }

  if(m_edt){
    // Overwrite R13 config with values for R15 EDT provided
    // easiest way to access data not include in NprachParameterNBR14
    m_CeLevel.coverageEnhancementLevel= tmp.coverageEnhancementLevel;
    m_CeLevel.nprachPeriodicity = tmp.nprachPeriodicity; 
    m_CeLevel.nprachStartTime = tmp.nprachStartTime;
    m_CeLevel.nprachSubcarrierOffset = tmp.nprachSubcarrierOffset;
    m_CeLevel.nprachNumSubcarriers = tmp.nprachNumSubcarriers;
    m_CeLevel.nprachSubcarrierMsg3RangeStart= tmp.nprachSubcarrierMsg3RangeStart;
    m_CeLevel.npdcchNumRepetitionsRA = tmp.npdcchNumRepetitionsRA;
    m_CeLevel.npdcchStartSfCssRa = tmp.npdcchStartSfCssRa;
    m_CeLevel.npdcchOffsetRa = tmp.npdcchOffsetRa;

  }
  m_backoffParameter = 0;

  if (m_mac_logging)
  {
    LogMessage("StartRandomAccessProcedureNb");
  }

  NS_LOG_INFO ("[UE][CE][SELECT] imsi=" << m_imsi
               << " rsrp=" << rsrp
               << " ceLevel="
               << static_cast<uint32_t> (m_CeLevel.coverageEnhancementLevel)
               << " ceRepetitions="
               << NbIotRrcSap::ConvertNumRepetitionsPerPreambleAttempt2int (m_CeLevel)
               << " maxPreambleAttemptsCe="
               << NbIotRrcSap::ConvertMaxNumPreambleAttemptCE2int (m_CeLevel));

  RandomlySelectAndSendRaPreambleNb ();
}
void
LteUeMac::DoSetRnti (uint16_t rnti)
{
  NS_LOG_FUNCTION (this);
  m_rnti = rnti;
}

void
LteUeMac::DoSetImsi (uint64_t imsi)
{
  NS_LOG_FUNCTION (this);
  m_imsi = imsi;
}

void
LteUeMac::DoStartNonContentionBasedRandomAccessProcedure (uint16_t rnti, uint8_t preambleId,
                                                          uint8_t prachMask)
{
  NS_LOG_FUNCTION (this << rnti << (uint16_t) preambleId << (uint16_t) prachMask);
  NS_ASSERT_MSG (prachMask == 0,
                 "requested PRACH MASK = " << (uint32_t) prachMask
                                           << ", but only PRACH MASK = 0 is supported");
  m_rnti = rnti;
  m_raPreambleId = preambleId;
  m_preambleTransmissionCounter = 0;
  m_preambleTransmissionCounterCe = 0;
  bool contention = false;
  SendRaPreamble (contention);
}

void
LteUeMac::DoAddLc (uint8_t lcId, LteUeCmacSapProvider::LogicalChannelConfig lcConfig,
                   LteMacSapUser *msu)
{
  NS_LOG_FUNCTION (this << " lcId" << (uint32_t) lcId);
  NS_ASSERT_MSG (m_lcInfoMap.find (lcId) == m_lcInfoMap.end (),
                 "cannot add channel because LCID " << (uint16_t) lcId << " is already present");

  LcInfo lcInfo;
  lcInfo.lcConfig = lcConfig;
  lcInfo.macSapUser = msu;
  m_lcInfoMap[lcId] = lcInfo;
}

void
LteUeMac::DoRemoveLc (uint8_t lcId)
{
  NS_LOG_FUNCTION (this << " lcId" << lcId);
  NS_ASSERT_MSG (m_lcInfoMap.find (lcId) != m_lcInfoMap.end (), "could not find LCID " << lcId);
  m_lcInfoMap.erase (lcId);
}

void
LteUeMac::DoReset ()
{
  NS_LOG_FUNCTION (this);
  std::map<uint8_t, LcInfo>::iterator it = m_lcInfoMap.begin ();
  while (it != m_lcInfoMap.end ())
    {
      // don't delete CCCH)
      if (it->first == 0)
        {
          ++it;
        }
      else
        {
          // note: use of postfix operator preserves validity of iterator
          m_lcInfoMap.erase (it++);
        }
    }
  // note: rnti will be assigned by the eNB using RA response message
  m_rnti = 0;
  m_noRaResponseReceivedEvent.Cancel ();
  m_rachConfigured = false;
  m_freshUlBsr = false;
  m_ulBsrReceived.clear ();
}

void
LteUeMac::DoNotifyConnectionSuccessful ()
{
  NS_LOG_FUNCTION (this);
  m_uePhySapProvider->NotifyConnectionSuccessful ();
}

void
LteUeMac::DoReceivePhyPdu (Ptr<Packet> p)
{
  LteRadioBearerTag tag;
  p->RemovePacketTag (tag);
  if (tag.GetRnti () == m_rnti)
    {
      // packet is for the current user
      std::map<uint8_t, LcInfo>::const_iterator it = m_lcInfoMap.find (tag.GetLcid ());
      if (it != m_lcInfoMap.end ())
        {
          LteMacSapUser::ReceivePduParameters rxPduParams;
          rxPduParams.p = p;
          rxPduParams.rnti = m_rnti;
          rxPduParams.lcid = tag.GetLcid ();
          it->second.macSapUser->ReceivePdu (rxPduParams);
          // NB-IoT Specific: Send HARQ at advertised Subframe
          if (m_nextPossibleHarqOpportunity.size () > 0)
            {
              uint32_t currentsubframe = 10 * (m_frameNo - 1) + (m_subframeNo - 1);
              uint32_t subframestillHarqF2 = m_nextPossibleHarqOpportunity[0].second.front()- currentsubframe;
              uint32_t subframestowait = m_nextPossibleHarqOpportunity[0].second.back() - currentsubframe;
              //NS_BUILD_DEBUG (std::cout << "Sending HARQ Response at " << currentsubframe + subframestowait << std::endl);

              Simulator::Schedule (MilliSeconds (subframestowait),
                                   &LteUePhySapProvider::SendHarqAckResponse, m_uePhySapProvider,
                                   true);
              Simulator::Schedule(MilliSeconds(subframestillHarqF2),&LteUeCmacSapUser::NotifyEnergyState, m_cmacSapUser, NbiotEnergyModel::PowerState::RRC_CONNECTED_SENDING_NPUSCH_F2);
              Simulator::Schedule(MilliSeconds(subframestowait+1),&LteUeCmacSapUser::NotifyEnergyState, m_cmacSapUser, NbiotEnergyModel::PowerState::RRC_CONNECTED_IDLE);
              m_nextPossibleHarqOpportunity.clear();
              //NS_BUILD_DEBUG (std::cout << m_rnti << " Got to MSG4-HARQ \n");
            }
        }
      else
        {
          NS_LOG_WARN ("received packet with unknown lcid " << (uint32_t) tag.GetLcid ());
        }
    }
}
void 
LteUeMac::DoSetTransmissionScheduled(bool scheduled){
  m_transmissionScheduled = scheduled;
}

void
LteUeMac::DoReceiveLteControlMessage (Ptr<LteControlMessage> msg)
{
  NS_LOG_FUNCTION (this);
  if (msg->GetMessageType () == LteControlMessage::UL_DCI)
    {
      Ptr<UlDciLteControlMessage> msg2 = DynamicCast<UlDciLteControlMessage> (msg);
      UlDciListElement_s dci = msg2->GetDci ();
      if (dci.m_ndi == 1)
        {
          // New transmission -> empty pkt buffer queue (for deleting eventual pkts not acked )
          Ptr<PacketBurst> pb = CreateObject<PacketBurst> ();
          m_miUlHarqProcessesPacket.at (m_harqProcessId) = pb;
          // Retrieve data from RLC
          std::map<uint8_t, LteMacSapProvider::ReportBufferStatusParameters>::iterator itBsr;
          uint16_t activeLcs = 0;
          uint32_t statusPduMinSize = 0;
          for (itBsr = m_ulBsrReceived.begin (); itBsr != m_ulBsrReceived.end (); itBsr++)
            {
              if (((*itBsr).second.statusPduSize > 0) || ((*itBsr).second.retxQueueSize > 0) ||
                  ((*itBsr).second.txQueueSize > 0))
                {
                  activeLcs++;
                  if (((*itBsr).second.statusPduSize != 0) &&
                      ((*itBsr).second.statusPduSize < statusPduMinSize))
                    {
                      statusPduMinSize = (*itBsr).second.statusPduSize;
                    }
                  if (((*itBsr).second.statusPduSize != 0) && (statusPduMinSize == 0))
                    {
                      statusPduMinSize = (*itBsr).second.statusPduSize;
                    }
                }
            }
          if (activeLcs == 0)
            {
              NS_LOG_ERROR (this << " No active flows for this UL-DCI");
              return;
            }
          std::map<uint8_t, LcInfo>::iterator it;
          uint32_t bytesPerActiveLc = dci.m_tbSize / activeLcs;
          bool statusPduPriority = false;
          if ((statusPduMinSize != 0) && (bytesPerActiveLc < statusPduMinSize))
            {
              // send only the status PDU which has highest priority
              statusPduPriority = true;
              NS_LOG_DEBUG (this << " Reduced resource -> send only Status, b ytes "
                                 << statusPduMinSize);
              if (dci.m_tbSize < statusPduMinSize)
                {
                  NS_FATAL_ERROR ("Insufficient Tx Opportunity for sending a status message");
                }
            }
          NS_LOG_LOGIC (this << " UE " << m_rnti << ": UL-CQI notified TxOpportunity of "
                             << dci.m_tbSize << " => " << bytesPerActiveLc << " bytes per active LC"
                             << " statusPduMinSize " << statusPduMinSize);

          LteMacSapUser::TxOpportunityParameters txOpParams;

          for (it = m_lcInfoMap.begin (); it != m_lcInfoMap.end (); it++)
            {
              itBsr = m_ulBsrReceived.find ((*it).first);
              NS_LOG_DEBUG (this << " Processing LC " << (uint32_t) (*it).first
                                 << " bytesPerActiveLc " << bytesPerActiveLc);
              if ((itBsr != m_ulBsrReceived.end ()) &&
                  (((*itBsr).second.statusPduSize > 0) || ((*itBsr).second.retxQueueSize > 0) ||
                   ((*itBsr).second.txQueueSize > 0)))
                {
                  if ((statusPduPriority) && ((*itBsr).second.statusPduSize == statusPduMinSize))
                    {
                      txOpParams.bytes = (*itBsr).second.statusPduSize;
                      txOpParams.layer = 0;
                      txOpParams.harqId = 0;
                      txOpParams.componentCarrierId = m_componentCarrierId;
                      txOpParams.rnti = m_rnti;
                      txOpParams.lcid = (*it).first;
                      (*it).second.macSapUser->NotifyTxOpportunity (txOpParams);
                      NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << " send  "
                                         << (*itBsr).second.statusPduSize << " status bytes to LC "
                                         << (uint32_t) (*it).first << " statusQueue "
                                         << (*itBsr).second.statusPduSize << " retxQueue"
                                         << (*itBsr).second.retxQueueSize << " txQueue"
                                         << (*itBsr).second.txQueueSize);
                      (*itBsr).second.statusPduSize = 0;
                      break;
                    }
                  else
                    {
                      uint32_t bytesForThisLc = bytesPerActiveLc;
                      NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << " bytes to LC "
                                         << (uint32_t) (*it).first << " statusQueue "
                                         << (*itBsr).second.statusPduSize << " retxQueue"
                                         << (*itBsr).second.retxQueueSize << " txQueue"
                                         << (*itBsr).second.txQueueSize);
                      if (((*itBsr).second.statusPduSize > 0) &&
                          (bytesForThisLc > (*itBsr).second.statusPduSize))
                        {
                          txOpParams.bytes = (*itBsr).second.statusPduSize;
                          txOpParams.layer = 0;
                          txOpParams.harqId = 0;
                          txOpParams.componentCarrierId = m_componentCarrierId;
                          txOpParams.rnti = m_rnti;
                          txOpParams.lcid = (*it).first;
                          (*it).second.macSapUser->NotifyTxOpportunity (txOpParams);
                          bytesForThisLc -= (*itBsr).second.statusPduSize;
                          NS_LOG_DEBUG (this << " serve STATUS " << (*itBsr).second.statusPduSize);
                          (*itBsr).second.statusPduSize = 0;
                        }
                      else
                        {
                          if ((*itBsr).second.statusPduSize > bytesForThisLc)
                            {
                              NS_FATAL_ERROR (
                                  "Insufficient Tx Opportunity for sending a status message");
                            }
                        }

                      if ((bytesForThisLc > 7) // 7 is the min TxOpportunity useful for Rlc
                          && (((*itBsr).second.retxQueueSize > 0) ||
                              ((*itBsr).second.txQueueSize > 0)))
                        {
                          if ((*itBsr).second.retxQueueSize > 0)
                            {
                              NS_LOG_DEBUG (this << " serve retx DATA, bytes " << bytesForThisLc);
                              txOpParams.bytes = bytesForThisLc;
                              txOpParams.layer = 0;
                              txOpParams.harqId = 0;
                              txOpParams.componentCarrierId = m_componentCarrierId;
                              txOpParams.rnti = m_rnti;
                              txOpParams.lcid = (*it).first;
                              (*it).second.macSapUser->NotifyTxOpportunity (txOpParams);
                              if ((*itBsr).second.retxQueueSize >= bytesForThisLc)
                                {
                                  (*itBsr).second.retxQueueSize -= bytesForThisLc;
                                }
                              else
                                {
                                  (*itBsr).second.retxQueueSize = 0;
                                }
                            }
                          else if ((*itBsr).second.txQueueSize > 0)
                            {
                              uint16_t lcid = (*it).first;
                              uint32_t rlcOverhead;
                              if (lcid == 1)
                                {
                                  // for SRB1 (using RLC AM) it's better to
                                  // overestimate RLC overhead rather than
                                  // underestimate it and risk unneeded
                                  // segmentation which increases delay
                                  rlcOverhead = 4;
                                }
                              else
                                {
                                  // minimum RLC overhead due to header
                                  rlcOverhead = 2;
                                }
                              NS_LOG_DEBUG (this << " serve tx DATA, bytes " << bytesForThisLc
                                                 << ", RLC overhead " << rlcOverhead);
                              txOpParams.bytes = bytesForThisLc;
                              txOpParams.layer = 0;
                              txOpParams.harqId = 0;
                              txOpParams.componentCarrierId = m_componentCarrierId;
                              txOpParams.rnti = m_rnti;
                              txOpParams.lcid = (*it).first;
                              (*it).second.macSapUser->NotifyTxOpportunity (txOpParams);
                              if ((*itBsr).second.txQueueSize >= bytesForThisLc - rlcOverhead)
                                {
                                  (*itBsr).second.txQueueSize -= bytesForThisLc - rlcOverhead;
                                }
                              else
                                {
                                  (*itBsr).second.txQueueSize = 0;
                                }
                            }
                        }
                      else
                        {
                          if (((*itBsr).second.retxQueueSize > 0) ||
                              ((*itBsr).second.txQueueSize > 0))
                            {
                              // resend BSR info for updating eNB peer MAC
                              m_freshUlBsr = true;
                            }
                        }
                      NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << "\t new queues "
                                         << (uint32_t) (*it).first << " statusQueue "
                                         << (*itBsr).second.statusPduSize << " retxQueue"
                                         << (*itBsr).second.retxQueueSize << " txQueue"
                                         << (*itBsr).second.txQueueSize);
                    }
                }
            }
        }
      else
        {
          // HARQ retransmission -> retrieve data from HARQ buffer
          NS_LOG_DEBUG (this << " UE MAC RETX HARQ " << (uint16_t) m_harqProcessId);
          Ptr<PacketBurst> pb = m_miUlHarqProcessesPacket.at (m_harqProcessId);
          for (std::list<Ptr<Packet>>::const_iterator j = pb->Begin (); j != pb->End (); ++j)
            {
              Ptr<Packet> pkt = (*j)->Copy ();
              m_uePhySapProvider->SendMacPdu (pkt);
            }
          m_miUlHarqProcessesPacketTimer.at (m_harqProcessId) = HARQ_PERIOD;
        }
    }
  else if (msg->GetMessageType () == LteControlMessage::RAR)
    {
      if (m_waitingForRaResponse)
        {
          Ptr<RarLteControlMessage> rarMsg = DynamicCast<RarLteControlMessage> (msg);
          uint16_t raRnti = rarMsg->GetRaRnti ();
          NS_LOG_LOGIC (this << "got RAR with RA-RNTI " << (uint32_t) raRnti << ", expecting "
                             << (uint32_t) m_raRnti);
          if (raRnti == m_raRnti) // RAR corresponds to TX subframe of preamble
            {
              for (std::list<RarLteControlMessage::Rar>::const_iterator it =
                       rarMsg->RarListBegin ();
                   it != rarMsg->RarListEnd (); ++it)
                {
                  if (it->rapId == m_raPreambleId) // RAR is for me
                    {
                      RecvRaResponse (it->rarPayload);
                      /// \todo RRC generates the RecvRaResponse messaged
                      /// for avoiding holes in transmission at PHY layer
                      /// (which produce erroneous UL CQI evaluation)
                    }
                }
            }
        }
    }
  else if (msg->GetMessageType () == LteControlMessage::RAR_NB)
    {
      if (m_waitingForRaResponse)
        {
          Ptr<RarNbiotControlMessage> rarMsg = DynamicCast<RarNbiotControlMessage> (msg);
          uint16_t raRnti = rarMsg->GetRaRnti ();
          NS_LOG_LOGIC (this << "got RAR with RA-RNTI " << (uint32_t) raRnti << ", expecting "
                             << (uint32_t) m_raRnti);
          if (raRnti == m_raRnti) // RAR corresponds to TX subframe of preamble
            {
              if (m_newSchemaActivated)
                {
                  const uint8_t expectedRapid =
                      static_cast<uint8_t> (NbIotRrcSap::ConvertNprachSubcarrierOffset2int (m_CeLevel) +
                                            m_raPreambleId);
                  const uint32_t localMetaId = NbIotToaUtils::ToaMetaIdFromImsi (m_imsi);
                  const uint16_t localToaBin = NbIotToaUtils::ComputeToaBin (localMetaId, m_toaNumBins);
                  bool hasToaRarForRapid = false;
                  for (std::list<NbIotRrcSap::Rar>::const_iterator it = rarMsg->RarListBegin ();
                       it != rarMsg->RarListEnd (); ++it)
                    {
                      if ((it->rapId == expectedRapid) && it->toaValid)
                        {
                          hasToaRarForRapid = true;
                          break;
                        }
                    }

                  struct ToaMatchCandidate
                  {
                    const NbIotRrcSap::Rar* rar;
                    uint16_t diff;
                  };

                  bool rarAccepted = false;
                  std::vector<ToaMatchCandidate> toaMatchCandidates;
                  for (std::list<NbIotRrcSap::Rar>::const_iterator it = rarMsg->RarListBegin ();
                       it != rarMsg->RarListEnd (); ++it)
                    {
                      if (it->rapId != expectedRapid)
                        {
                          continue;
                        }

                      NS_LOG_INFO ("[UE][MSG2][RX] imsi=" << m_imsi
                                  << " ra-rnti=" << static_cast<uint32_t> (rarMsg->GetRaRnti ())
                                  << " rapid=" << static_cast<uint32_t> (it->rapId)
                                  << " tc-rnti=" << it->cellRnti
                                  << " toaValid=" << (it->toaValid ? "1" : "0")
                                  << " toaBin=" << static_cast<uint32_t> (it->toaBin)
                                  << " virtualId=" << static_cast<uint32_t> (it->virtualId)
                                  << " codebook=" << static_cast<uint32_t> (it->codebookId)
                                  << " localToaBin=" << static_cast<uint32_t> (localToaBin));

                      // Legacy fallback in new schema: no ToA-tagged RAR for this RAPID.
                      if (!hasToaRarForRapid)
                        {
                          NS_LOG_INFO ("[UE][MSG2][ACCEPT] imsi=" << m_imsi
                                      << " reason=legacy-first-match"
                                      << " rapid=" << static_cast<uint32_t> (it->rapId)
                                      << " tc-rnti=" << it->cellRnti
                                      << " codebook=" << static_cast<uint32_t> (it->codebookId)
                                      << " virtualId=" << static_cast<uint32_t> (it->virtualId));
                          if (SaraReport::IsEnabled ())
                            {
                              SaraReport::LogMsg2Select (Simulator::GetContext (), m_imsi, it->rapId,
                                                         it->rarPayload.cellRnti, false, 0, 0,
                                                         0, localToaBin, it->toaBin,
                                                         it->virtualId, it->codebookId,
                                                         1, 0);
                            }
                          RecvRaResponseNb (*it);
                          rarAccepted = true;
                          break;
                        }

                      if (!it->toaValid)
                        {
                          NS_LOG_INFO ("[UE][MSG2][REJECT] imsi=" << m_imsi
                                      << " rapid=" << static_cast<uint32_t> (it->rapId)
                                      << " reason=no-toa");
                          if (SaraReport::IsEnabled ())
                            {
                              SaraReport::LogMsg2Select (Simulator::GetContext (), m_imsi, it->rapId,
                                                         it->rarPayload.cellRnti, false, 0, 0,
                                                         0, localToaBin, it->toaBin,
                                                         it->virtualId, it->codebookId,
                                                         0, 1);
                            }
                          continue;
                        }

                      const uint16_t diff = (localToaBin >= it->toaBin) ? (localToaBin - it->toaBin)
                                                                        : (it->toaBin - localToaBin);
                      if (diff <= m_toaToleranceBins)
                        {
                          ToaMatchCandidate c;
                          c.rar = &(*it);
                          c.diff = diff;
                          toaMatchCandidates.push_back (c);
                        }
                      else
                        {
                          NS_LOG_INFO ("[UE][MSG2][REJECT] imsi=" << m_imsi
                                      << " reason=toa-mismatch"
                                      << " rapid=" << static_cast<uint32_t> (it->rapId)
                                      << " localToaBin=" << static_cast<uint32_t> (localToaBin)
                                      << " rarToaBin=" << static_cast<uint32_t> (it->toaBin)
                                      << " tolerance=" << static_cast<uint32_t> (m_toaToleranceBins));
                          if (SaraReport::IsEnabled ())
                            {
                              SaraReport::LogMsg2Select (Simulator::GetContext (), m_imsi, it->rapId,
                                                         it->rarPayload.cellRnti, false, 0, 0,
                                                         1, localToaBin, it->toaBin,
                                                         it->virtualId, it->codebookId,
                                                         0, 2);
                            }
                        }
                    }

                  if (!rarAccepted && hasToaRarForRapid && !toaMatchCandidates.empty ())
                    {
                      uint16_t bestDiff = toaMatchCandidates.front ().diff;
                      for (std::vector<ToaMatchCandidate>::const_iterator c = toaMatchCandidates.begin ();
                           c != toaMatchCandidates.end (); ++c)
                        {
                          if (c->diff < bestDiff)
                            {
                              bestDiff = c->diff;
                            }
                        }

                      std::vector<const NbIotRrcSap::Rar*> bestMatches;
                      for (std::vector<ToaMatchCandidate>::const_iterator c = toaMatchCandidates.begin ();
                           c != toaMatchCandidates.end (); ++c)
                        {
                          if (c->diff == bestDiff)
                            {
                              bestMatches.push_back (c->rar);
                            }
                        }

                      std::sort (bestMatches.begin (), bestMatches.end (),
                                 [] (const NbIotRrcSap::Rar* a, const NbIotRrcSap::Rar* b)
                                 {
                                   if (a->toaBin != b->toaBin)
                                     {
                                       return a->toaBin < b->toaBin;
                                     }
                                   return a->cellRnti < b->cellRnti;
                                 });

                      const uint32_t tieSeed = localMetaId ^ (static_cast<uint32_t> (expectedRapid) * 2654435761u);
                      const uint32_t tieIndex = bestMatches.empty () ? 0 : (tieSeed % bestMatches.size ());
                      const NbIotRrcSap::Rar* chosen = bestMatches[tieIndex];

                      NS_LOG_INFO ("[UE][MSG2][ACCEPT] imsi=" << m_imsi
                                  << " reason=toa-best-match"
                                  << " rapid=" << static_cast<uint32_t> (chosen->rapId)
                                  << " localToaBin=" << static_cast<uint32_t> (localToaBin)
                                  << " rarToaBin=" << static_cast<uint32_t> (chosen->toaBin)
                                  << " tc-rnti=" << chosen->cellRnti
                                  << " codebook=" << static_cast<uint32_t> (chosen->codebookId)
                                  << " virtualId=" << static_cast<uint32_t> (chosen->virtualId)
                                  << " tieCandidates=" << static_cast<uint32_t> (bestMatches.size ())
                                  << " tieIndex=" << tieIndex);
                      if (SaraReport::IsEnabled ())
                        {
                          SaraReport::LogMsg2Select (Simulator::GetContext (), m_imsi, chosen->rapId,
                                                     chosen->rarPayload.cellRnti, false, 0, 0,
                                                     1, localToaBin, chosen->toaBin,
                                                     chosen->virtualId, chosen->codebookId,
                                                     1, 0);
                        }
                      RecvRaResponseNb (*chosen);
                      rarAccepted = true;
                    }
                }
              else
                {
                  uint8_t myRapid = NbIotRrcSap::ConvertNprachSubcarrierOffset2int (m_CeLevel) +
                                    m_raPreambleId;
                  bool processed = false;
                  bool sawSaraForMyRapid = false;

                  for (std::list<NbIotRrcSap::Rar>::const_iterator it = rarMsg->RarListBegin ();
                       it != rarMsg->RarListEnd (); ++it)
                    {
                      if (it->rapId != myRapid)
                        {
                          continue;
                        }

                      if (it->saraGroup)
                        {
                          sawSaraForMyRapid = true;
                          if (it->saraGroupSize > 0)
                            {
                              if (!m_saraDesiredTagSet)
                                {
                                  Ptr<UniformRandomVariable> rng = CreateObject<UniformRandomVariable> ();
                                  m_saraDesiredTag = rng->GetInteger (0, it->saraGroupSize - 1);
                                  m_saraDesiredTagSet = true;
                                }

                              if (it->saraTag == m_saraDesiredTag)
                                {
                                  m_saraGroupActive = true;
                                  m_saraGroupSize = it->saraGroupSize;
                                  m_saraTag = it->saraTag;
                                  m_saraWaitingForTag = false;

                                  NS_LOG_INFO ("[UE][RAR][SELECT] node=" << Simulator::GetContext ()
                                               << " imsi=" << m_imsi
                                               << " RAPID=" << (uint32_t) myRapid
                                               << " group=" << (uint32_t) m_saraGroupSize
                                               << " tag=" << (uint32_t) m_saraTag);
                                  if (SaraReport::IsEnabled ())
                                    {
                                      SaraReport::LogMsg2Select (Simulator::GetContext (), m_imsi, it->rapId,
                                                                 it->rarPayload.cellRnti, true,
                                                                 it->saraGroupSize, it->saraTag,
                                                                 -1, -1, -1, -1, -1,
                                                                 1, 0);
                                    }

                                  RecvRaResponseNb (*it);
                                  processed = true;
                                  break;
                                }
                              else if (SaraReport::IsEnabled ())
                                {
                                  SaraReport::LogMsg2Select (Simulator::GetContext (), m_imsi, it->rapId,
                                                             it->rarPayload.cellRnti, true,
                                                             it->saraGroupSize, it->saraTag,
                                                             -1, -1, -1, -1, -1,
                                                             0, 3);
                                }
                            }
                        }
                    }

                  if (processed)
                    {
                      return;
                    }

                  if (sawSaraForMyRapid)
                    {
                      m_saraWaitingForTag = true;
                      if (SaraReport::IsEnabled ())
                        {
                          SaraReport::LogMsg2Select (Simulator::GetContext (), m_imsi, myRapid,
                                                     0, true, m_saraGroupSize, m_saraDesiredTag,
                                                     -1, -1, -1, -1, -1,
                                                     0, 4);
                        }
                      return;
                    }

                  for (std::list<NbIotRrcSap::Rar>::const_iterator it = rarMsg->RarListBegin ();
                       it != rarMsg->RarListEnd (); ++it)
                    {
                      if (it->rapId == myRapid)
                        {
                          m_saraGroupActive = false;
                          m_saraGroupSize = 1;
                          m_saraTag = 0;
                          m_saraDesiredTagSet = false;
                          m_saraDesiredTag = 0;
                          m_saraWaitingForTag = false;

                          NS_LOG_INFO ("[UE][RAR][SELECT] node=" << Simulator::GetContext ()
                                       << " imsi=" << m_imsi
                                       << " RAPID=" << (uint32_t) myRapid);
                          if (SaraReport::IsEnabled ())
                            {
                              SaraReport::LogMsg2Select (Simulator::GetContext (), m_imsi, it->rapId,
                                                         it->rarPayload.cellRnti, false, 0, 0,
                                                         -1, -1, -1, -1, -1,
                                                         1, 0);
                            }
                          RecvRaResponseNb (*it);
                          break;
                        }
                    }
                }
            }
        }
    }
  else if (msg->GetMessageType () == LteControlMessage::DL_DCI_NB){
      Ptr<DlDciN1NbiotControlMessage> msg2 = DynamicCast<DlDciN1NbiotControlMessage> (msg);
      NbIotRrcSap::DciN1 dci = msg2->GetDci ();
      //Handle Energy State Dci Reception
      m_cmacSapUser->NotifyEnergyState(NbiotEnergyModel::PowerState::RRC_CONNECTED_IDLE);

      uint32_t subframesTillNpdschBegin = dci.npdschOpportunity.front() - (10*(m_frameNo-1)+m_subframeNo-1);
      uint32_t subframesTillNpdschEnd = dci.npdschOpportunity.back() - (10 * (m_frameNo - 1) + m_subframeNo - 1);
      m_transmissionScheduled = true;
      Simulator::Schedule(MilliSeconds(subframesTillNpdschBegin), &LteUeCmacSapUser::NotifyEnergyState, m_cmacSapUser, NbiotEnergyModel::PowerState::RRC_CONNECTED_RECEIVING_NPDSCH);
      Simulator::Schedule(MilliSeconds(subframesTillNpdschEnd+1), &LteUeCmacSapUser::NotifyEnergyState, m_cmacSapUser, NbiotEnergyModel::PowerState::RRC_CONNECTED_IDLE);

      // Liberg p. 286 "After the device completes its NPUSCH Format 2 transmission, it is not required to monitor NPDCCH search space for 3 ms."
      if (m_nextPossibleHarqOpportunity.size() > 0){
        uint32_t subframesTransmissionEnd = m_nextPossibleHarqOpportunity[0].second.back() - (10*(m_frameNo-1)+m_subframeNo-1);
        Simulator::Schedule(MilliSeconds(subframesTransmissionEnd+3),&LteUeMac::DoSetTransmissionScheduled, this,false); // Transmission done, ready to listen to new NPDCCH
      }else{
        Simulator::Schedule(MilliSeconds(subframesTillNpdschEnd+3), &LteUeMac::DoSetTransmissionScheduled, this, false);
      }


  }
  else if (msg->GetMessageType () == LteControlMessage::UL_DCI_NB)
    {
      Ptr<UlDciN0NbiotControlMessage> msg2 = DynamicCast<UlDciN0NbiotControlMessage> (msg);
      NbIotRrcSap::DciN0 dci = msg2->GetDci ();
      const uint32_t npuschOppCount =
          dci.npuschOpportunity.size () > 0
              ? static_cast<uint32_t> (dci.npuschOpportunity[0].second.size ())
              : 0;
      const uint32_t npuschOppFirst =
          npuschOppCount > 0
              ? static_cast<uint32_t> (dci.npuschOpportunity[0].second.front ())
              : 0;
      const uint32_t npuschOppLast =
          npuschOppCount > 0
              ? static_cast<uint32_t> (dci.npuschOpportunity[0].second.back ())
              : 0;
      NS_LOG_INFO ("[UE][UL-DCI][RX] imsi=" << m_imsi
                   << " rnti=" << m_rnti
                   << " tbs=" << dci.tbs
                   << " npuschOppCount=" << npuschOppCount
                   << " npuschOppFirst=" << npuschOppFirst
                   << " npuschOppLast=" << npuschOppLast
                   << " ndi=" << (dci.NDI ? "1" : "0"));

      m_cmacSapUser->NotifyEnergyState(NbiotEnergyModel::PowerState::RRC_CONNECTED_IDLE);

      uint32_t subframesTillNpusch = dci.npuschOpportunity[0].second.front() - (10*(m_frameNo-1)+m_subframeNo-1);
      uint32_t subframes = *(dci.npuschOpportunity[0].second.end () - 1) -
          (10 * (m_frameNo - 1) + m_subframeNo - 1);

      m_transmissionScheduled = true;
      Simulator::Schedule(MilliSeconds(subframesTillNpusch), &LteUeCmacSapUser::NotifyEnergyState, m_cmacSapUser, NbiotEnergyModel::PowerState::RRC_CONNECTED_SENDING_NPUSCH);
      Simulator::Schedule(MilliSeconds(subframes+1), &LteUeCmacSapUser::NotifyEnergyState, m_cmacSapUser, NbiotEnergyModel::PowerState::RRC_CONNECTED_IDLE);


      // Liberg p. 283 "After the device completes its NPUSCH transmission, there is at least a 3-ms gap to allow the device to switch from transmission mode to reception mode and be ready for monitoring the next NPDCCH search space candidate."
      uint32_t subframesTransmissionEnd = dci.npuschOpportunity[0].second.back() - (10*(m_frameNo-1)+m_subframeNo-1);
      Simulator::Schedule(MilliSeconds(subframesTransmissionEnd+3),&LteUeMac::DoSetTransmissionScheduled, this,false); // Transmission done, ready to listen to new NPDCCH

      if (dci.NDI)
        {
          // New transmission -> empty pkt buffer queue (for deleting eventual pkts not acked )
          Ptr<PacketBurst> pb = CreateObject<PacketBurst> ();
          m_miUlHarqProcessesPacket.at (m_harqProcessId) = pb;
          // Retrieve data from RLC
          std::map<uint8_t, LteMacSapProvider::ReportBufferStatusParameters>::iterator itBsr;
          std::vector<uint8_t> activeLcs;
          for (itBsr = m_ulBsrReceived.begin (); itBsr != m_ulBsrReceived.end (); itBsr++)
          {
            if (((*itBsr).second.statusPduSize > 0) || ((*itBsr).second.retxQueueSize > 0) ||
                ((*itBsr).second.txQueueSize > 0))
              {
                if(m_nextIsMsg5){
                  // We might get a bigger TxOp as the size of the MSG5, so we don't want user data transmitted there
                  if(itBsr->first >2){
                    continue;
                  }
                }
                activeLcs.push_back(itBsr->first);
              }
          }
          if(m_nextIsMsg5){
            m_nextIsMsg5 = false;
          }
          LteMacSapUser::TxOpportunityParameters txOpParams;
          // Prioritise SRBs over DataBs
          uint64_t bytesforallLc = dci.tbs/8;
          for(std::vector<uint8_t>::iterator lcit = activeLcs.begin(); lcit != activeLcs.end(); ++lcit){
            std::map<uint8_t, LteMacSapProvider::ReportBufferStatusParameters>::iterator bsr = m_ulBsrReceived.find((*lcit));
            std::map<uint8_t, LcInfo>::iterator lcidIt = m_lcInfoMap.find (bsr->second.lcid);
            if ((bsr->second.statusPduSize > 0) &&
                    (bytesforallLc > bsr->second.statusPduSize))
              {
                txOpParams.bytes = bsr->second.statusPduSize;
                txOpParams.layer = 0;
                txOpParams.harqId = 0;
                txOpParams.componentCarrierId = m_componentCarrierId;
                txOpParams.rnti = bsr->second.rnti;
                txOpParams.lcid = bsr->second.lcid;
                //Simulator::Schedule (MilliSeconds (subframes), &LteMacSapUser::NotifyTxOpportunity,
                //    (*lcidIt).second.macSapUser, txOpParams);
                NS_LOG_INFO ("[UE][NPUSCH][TXOP] imsi=" << m_imsi
                             << " lcid=" << static_cast<uint32_t> (txOpParams.lcid)
                             << " bytes=" << txOpParams.bytes
                             << " fromULDCI=1");
                (*lcidIt).second.macSapUser->NotifyTxOpportunityNb(txOpParams,subframes);
                bytesforallLc -= bsr->second.statusPduSize;
                bsr->second.statusPduSize = 0;
              }
            else
              {
                if (bsr->second.statusPduSize > bytesforallLc)
                  {
                    //NS_FATAL_ERROR (
                    //    "Insufficient Tx Opportunity for sending a status message");
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
                    //Simulator::Schedule (MilliSeconds (subframes), &LteMacSapUser::NotifyTxOpportunity,
                    //  (*lcidIt).second.macSapUser, txOpParams);
                    NS_LOG_INFO ("[UE][NPUSCH][TXOP] imsi=" << m_imsi
                                 << " lcid=" << static_cast<uint32_t> (txOpParams.lcid)
                                 << " bytes=" << txOpParams.bytes
                                 << " fromULDCI=1");
                    (*lcidIt).second.macSapUser->NotifyTxOpportunityNb(txOpParams,subframes);
                  }
                else if (bsr->second.txQueueSize > 0)
                  {
                    uint16_t lcid = bsr->second.lcid;
                    uint32_t rlcOverhead;
                    if (lcid == 1 || lcid == 3)
                      {
                        // for SRB1 (using RLC AM) it's better to
                        // overestimate RLC overhead rather than
                        // underestimate it and risk unneeded
                        // segmentation which increases delay
                        rlcOverhead = 4;
                      }
                    else
                      {
                        // minimum RLC overhead due to header
                        rlcOverhead = 2;
                      }
                    NS_LOG_DEBUG (this << " serve tx DATA, bytes " << bytesforallLc 
                                        << ", RLC overhead " << rlcOverhead);
                    if(bsr->second.txQueueSize > bytesforallLc){
                      txOpParams.bytes = bytesforallLc;
                      bsr->second.txQueueSize -= bytesforallLc-rlcOverhead;
                      bytesforallLc = 0;
                    }else{
                      if(bsr->second.txQueueSize +4 < 7){
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

                    //Simulator::Schedule (MilliSeconds (subframes), &LteMacSapUser::NotifyTxOpportunity,
                    //  (*lcidIt).second.macSapUser, txOpParams);
                    NS_LOG_INFO ("[UE][NPUSCH][TXOP] imsi=" << m_imsi
                                 << " lcid=" << static_cast<uint32_t> (txOpParams.lcid)
                                 << " bytes=" << txOpParams.bytes
                                 << " fromULDCI=1");
                    (*lcidIt).second.macSapUser->NotifyTxOpportunityNb(txOpParams,subframes);
                    
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
          //uint16_t activeLcs = 0;
          //uint32_t statusPduMinSize = 0;
          //for (itBsr = m_ulBsrReceived.begin (); itBsr != m_ulBsrReceived.end (); itBsr++)
          //  {
          //    if (((*itBsr).second.statusPduSize > 0) || ((*itBsr).second.retxQueueSize > 0) ||
          //        ((*itBsr).second.txQueueSize > 0))
          //      {
          //        activeLcs++;
          //        if (((*itBsr).second.statusPduSize != 0) &&
          //            ((*itBsr).second.statusPduSize < statusPduMinSize))
          //          {
          //            statusPduMinSize = (*itBsr).second.statusPduSize;
          //          }
          //        if (((*itBsr).second.statusPduSize != 0) && (statusPduMinSize == 0))
          //          {
          //            statusPduMinSize = (*itBsr).second.statusPduSize;
          //          }
          //      }
          //  }
          //if (activeLcs == 0)
          //  {
          //    NS_LOG_ERROR (this << " No active flows for this UL-DCI");
          //    return;
          //  }
          //std::map<uint8_t, LcInfo>::iterator it;
          //uint32_t bytesPerActiveLc = (dci.tbs/8)/ activeLcs;
          //bool statusPduPriority = false;
          //if ((statusPduMinSize != 0) && (bytesPerActiveLc < statusPduMinSize))
          //  {
          //    // send only the status PDU which has highest priority
          //    statusPduPriority = true;
          //    NS_LOG_DEBUG (this << " Reduced resource -> send only Status, b ytes "
          //                       << statusPduMinSize);
          //    if (dci.tbs/8< statusPduMinSize)
          //      {
          //        NS_FATAL_ERROR ("Insufficient Tx Opportunity for sending a status message");
          //      }
          //  }
          //NS_LOG_LOGIC (this << " UE " << m_rnti << ": UL-CQI notified TxOpportunity of "
          //                   << dci.tbs << " => " << bytesPerActiveLc << " bytes per active LC"
          //                   << " statusPduMinSize " << statusPduMinSize);

          //LteMacSapUser::TxOpportunityParameters txOpParams;

          //for (it = m_lcInfoMap.begin (); it != m_lcInfoMap.end (); it++)
          //  {
          //    itBsr = m_ulBsrReceived.find ((*it).first);
          //    NS_LOG_DEBUG (this << " Processing LC " << (uint32_t) (*it).first
          //                       << " bytesPerActiveLc " << bytesPerActiveLc);
          //    if ((itBsr != m_ulBsrReceived.end ()) &&
          //        (((*itBsr).second.statusPduSize > 0) || ((*itBsr).second.retxQueueSize > 0) ||
          //         ((*itBsr).second.txQueueSize > 0)))
          //      {
          //        if ((statusPduPriority) && ((*itBsr).second.statusPduSize == statusPduMinSize))
          //          {
          //            txOpParams.bytes = (*itBsr).second.statusPduSize;
          //            txOpParams.layer = 0;
          //            txOpParams.harqId = 0;
          //            txOpParams.componentCarrierId = m_componentCarrierId;
          //            txOpParams.rnti = m_rnti;
          //            txOpParams.lcid = (*it).first;
          //            Simulator::Schedule (MilliSeconds (subframes), &LteMacSapUser::NotifyTxOpportunity,
          //                    it->second.macSapUser, txOpParams);
          //            NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << " send  "
          //                               << (*itBsr).second.statusPduSize << " status bytes to LC "
          //                               << (uint32_t) (*it).first << " statusQueue "
          //                               << (*itBsr).second.statusPduSize << " retxQueue"
          //                               << (*itBsr).second.retxQueueSize << " txQueue"
          //                               << (*itBsr).second.txQueueSize);
          //            (*itBsr).second.statusPduSize = 0;
          //            break;
          //          }
          //        else
          //          {
          //            uint32_t bytesForThisLc = bytesPerActiveLc;
          //            NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << " bytes to LC "
          //                               << (uint32_t) (*it).first << " statusQueue "
          //                               << (*itBsr).second.statusPduSize << " retxQueue"
          //                               << (*itBsr).second.retxQueueSize << " txQueue"
          //                               << (*itBsr).second.txQueueSize);
          //            if (((*itBsr).second.statusPduSize > 0) &&
          //                (bytesForThisLc > (*itBsr).second.statusPduSize))
          //              {
          //                txOpParams.bytes = (*itBsr).second.statusPduSize;
          //                txOpParams.layer = 0;
          //                txOpParams.harqId = 0;
          //                txOpParams.componentCarrierId = m_componentCarrierId;
          //                txOpParams.rnti = m_rnti;
          //                txOpParams.lcid = (*it).first;
          //                Simulator::Schedule (MilliSeconds (subframes), &LteMacSapUser::NotifyTxOpportunity,
          //                    it->second.macSapUser, txOpParams);
          //                bytesForThisLc -= (*itBsr).second.statusPduSize;
          //                NS_LOG_DEBUG (this << " serve STATUS " << (*itBsr).second.statusPduSize);
          //                (*itBsr).second.statusPduSize = 0;
          //              }
          //            else
          //              {
          //                if ((*itBsr).second.statusPduSize > bytesForThisLc)
          //                  {
          //                    NS_FATAL_ERROR (
          //                        "Insufficient Tx Opportunity for sending a status message");
          //                  }
          //              }

          //            if ((bytesForThisLc > 7) // 7 is the min TxOpportunity useful for Rlc
          //                && (((*itBsr).second.retxQueueSize > 0) ||
          //                    ((*itBsr).second.txQueueSize > 0)))
          //              {
          //                if ((*itBsr).second.retxQueueSize > 0)
          //                  {
          //                    NS_LOG_DEBUG (this << " serve retx DATA, bytes " << bytesForThisLc);
          //                    txOpParams.bytes = bytesForThisLc;
          //                    txOpParams.layer = 0;
          //                    txOpParams.harqId = 0;
          //                    txOpParams.componentCarrierId = m_componentCarrierId;
          //                    txOpParams.rnti = m_rnti;
          //                    txOpParams.lcid = (*it).first;
          //                    Simulator::Schedule (MilliSeconds (subframes), &LteMacSapUser::NotifyTxOpportunity,
          //                      it->second.macSapUser, txOpParams);
          //                    if ((*itBsr).second.retxQueueSize >= bytesForThisLc)
          //                      {
          //                        (*itBsr).second.retxQueueSize -= bytesForThisLc;
          //                      }
          //                    else
          //                      {
          //                        (*itBsr).second.retxQueueSize = 0;
          //                      }
          //                  }
          //                else if ((*itBsr).second.txQueueSize > 0)
          //                  {
          //                    uint16_t lcid = (*it).first;
          //                    uint32_t rlcOverhead;
          //                    if (lcid == 1)
          //                      {
          //                        // for SRB1 (using RLC AM) it's better to
          //                        // overestimate RLC overhead rather than
          //                        // underestimate it and risk unneeded
          //                        // segmentation which increases delay
          //                        rlcOverhead = 4;
          //                      }
          //                    else
          //                      {
          //                        // minimum RLC overhead due to header
          //                        rlcOverhead = 2;
          //                      }
          //                    NS_LOG_DEBUG (this << " serve tx DATA, bytes " << bytesForThisLc
          //                                       << ", RLC overhead " << rlcOverhead);
          //                    txOpParams.bytes = bytesForThisLc;
          //                    txOpParams.layer = 0;
          //                    txOpParams.harqId = 0;
          //                    txOpParams.componentCarrierId = m_componentCarrierId;
          //                    txOpParams.rnti = m_rnti;
          //                    txOpParams.lcid = (*it).first;

          //                    Simulator::Schedule (MilliSeconds (subframes), &LteMacSapUser::NotifyTxOpportunity,
          //                      it->second.macSapUser, txOpParams);
          //                    if ((*itBsr).second.txQueueSize >= bytesForThisLc - rlcOverhead)
          //                      {
          //                        (*itBsr).second.txQueueSize -= bytesForThisLc - rlcOverhead;
          //                      }
          //                    else
          //                      {
          //                        (*itBsr).second.txQueueSize = 0;
          //                      }
          //                  }
          //              }
          //            else
          //              {
          //                if (((*itBsr).second.retxQueueSize > 0) ||
          //                    ((*itBsr).second.txQueueSize > 0))
          //                  {
          //                    // resend BSR info for updating eNB peer MAC
          //                    m_freshUlBsr = true;
          //                  }
          //              }
          //            NS_LOG_LOGIC (this << "\t" << bytesPerActiveLc << "\t new queues "
          //                               << (uint32_t) (*it).first << " statusQueue "
          //                               << (*itBsr).second.statusPduSize << " retxQueue"
          //                               << (*itBsr).second.retxQueueSize << " txQueue"
          //                               << (*itBsr).second.txQueueSize);
          //          }
          //      }
          //  }
        }
      else
        {
          // HARQ retransmission -> retrieve data from HARQ buffer
          NS_LOG_DEBUG (this << " UE MAC RETX HARQ " << (uint16_t) m_harqProcessId);
          Ptr<PacketBurst> pb = m_miUlHarqProcessesPacket.at (m_harqProcessId);
          for (std::list<Ptr<Packet>>::const_iterator j = pb->Begin (); j != pb->End (); ++j)
            {
              Ptr<Packet> pkt = (*j)->Copy ();
              Simulator::Schedule (MilliSeconds (subframes), &LteUePhySapProvider::SendMacPdu ,
                m_uePhySapProvider, pkt);
              //m_uePhySapProvider->SendMacPdu (pkt);
            }
          m_miUlHarqProcessesPacketTimer.at (m_harqProcessId) = HARQ_PERIOD;
        }
    }
  else
    {
      NS_LOG_WARN (this << " LteControlMessage not recognized");
    }
}
void
LteUeMac::DoNotifyAboutHarqOpportunity (std::vector<std::pair<uint64_t, std::vector<uint64_t>>> subframes)
{
  m_nextPossibleHarqOpportunity = subframes;
}

void
LteUeMac::RefreshHarqProcessesPacketBuffer (void)
{
  NS_LOG_FUNCTION (this);

  for (uint16_t i = 0; i < m_miUlHarqProcessesPacketTimer.size (); i++)
    {
      if (m_miUlHarqProcessesPacketTimer.at (i) == 0)
        {
          if (m_miUlHarqProcessesPacket.at (i)->GetSize () > 0)
            {
              // timer expired: drop packets in buffer for this process
              NS_LOG_INFO (this << " HARQ Proc Id " << i << " packets buffer expired");
              Ptr<PacketBurst> emptyPb = CreateObject<PacketBurst> ();
              m_miUlHarqProcessesPacket.at (i) = emptyPb;
            }
        }
      else
        {
          m_miUlHarqProcessesPacketTimer.at (i)--;
        }
    }
}

void
LteUeMac::DoSubframeIndication (uint32_t frameNo, uint32_t subframeNo)
{
  NS_LOG_FUNCTION (this);
  m_frameNo = frameNo;
  m_subframeNo = subframeNo;
  //RefreshHarqProcessesPacketBuffer ();
  //
  if(m_edrx && GetBufferSizeComplete() == 0 && !m_transmissionScheduled){
    m_listenToSearchSpaces = false;
    m_cmacSapUser->NotifyEnergyState(NbiotEnergyModel::PowerState::RRC_SUSPENDED_EDRX);
    m_edrx = false;
    // TODO Activate Paging Occasion listening
  }
  if(m_psm && GetBufferSizeComplete() == 0 && !m_transmissionScheduled){
    m_listenToSearchSpaces = false;
    m_cmacSapUser->NotifyEnergyState(NbiotEnergyModel::PowerState::RRC_SUSPENDED_PSM);
    // TODO Activate Paging Occasion listening
    m_psm = false;
  }
  if(m_listenToSearchSpaces){
    // Energy Model Start Receiving on my SearchSpaceBegin
    uint32_t searchSpacePeriodicity = NbIotRrcSap::ConvertNpdcchNumRepetitionsRa2int (m_CeLevel) *
                                      NbIotRrcSap::ConvertNpdcchStartSfCssRa2double (m_CeLevel);
    uint32_t searchSpaceConditionLeftSide =
        (10 * (m_frameNo - 1) + (m_subframeNo - 1)) % searchSpacePeriodicity;
    uint32_t searchSpaceConditionRightSide =
        NbIotRrcSap::ConvertNpdcchOffsetRa2double (m_CeLevel) * searchSpacePeriodicity;

    if (searchSpaceConditionLeftSide == searchSpaceConditionRightSide) 
      {
        m_inSearchSpace=true;
        m_subframesInSearchSpace = 0;
      }
    if (m_inSearchSpace){ 
      if(!m_transmissionScheduled && m_cmacSapUser->GetEnergyState() == NbiotEnergyModel::PowerState::RRC_CONNECTED_IDLE){
        // We just moved from another state into IDLE
        // According to Liberg p.286 we still have to monitor the rest of the NPDCCH
        // Offset like the 3ms after NPUSCH F2 schould be handled by the m_transmissionScheduled flag
        m_cmacSapUser->NotifyEnergyState(NbiotEnergyModel::PowerState::RRC_CONNECTED_RECEIVING_NPDCCH);
      }
      if (((m_subframeNo-1) != 0) && ((m_subframeNo-1) != 5) && !((m_subframeNo-1) == 9 && ((m_frameNo-1) % 2) == 1)) // Current Subframe is not NPBCH, NPSS and NSSS, and SI #TODO add SI
        {
          m_subframesInSearchSpace++; 
        }
      if(m_subframesInSearchSpace == NbIotRrcSap::ConvertNpdcchNumRepetitionsRa2int (m_CeLevel)){
        m_inSearchSpace = false;
        m_subframesInSearchSpace = 0;
        if(m_cmacSapUser->GetEnergyState() == NbiotEnergyModel::PowerState::RRC_CONNECTED_RECEIVING_NPDCCH){
          m_cmacSapUser->NotifyEnergyState(NbiotEnergyModel::PowerState::RRC_CONNECTED_IDLE); // Device listened to the whole SearchSpace without Dci scheduled
        }
      }
    }
  }
  if ((Simulator::Now () >= m_bsrLast + m_bsrPeriodicity) && (m_freshUlBsr == true))
    {
      if (m_componentCarrierId == 0)
        {
          //Send BSR through primary carrier
          SendReportBufferStatus ();
        }
      m_bsrLast = Simulator::Now ();
      m_freshUlBsr = false;
    }
  m_harqProcessId = (m_harqProcessId + 1) % HARQ_PERIOD;
}

int64_t
LteUeMac::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_raPreambleUniformVariable->SetStream (stream);
  m_raBackoffUniformVariable->SetStream (stream + 1);
  return 2;
}
void 
LteUeMac::DoNotifyEdrx(){
  m_edrx = true;
}

void 
LteUeMac::DoNotifyPsm(){
  m_psm = true;
}

NbIotRrcSap::NprachParametersNb::CoverageEnhancementLevel 
LteUeMac::DoGetCoverageEnhancementLevel(){
  return m_CeLevel.coverageEnhancementLevel;
}

void LteUeMac::DoSetMsg5Buffer(uint32_t buffersize){
  m_msg5Buffer = buffersize;
}

void LteUeMac::SetLogDir(std::string dirname){
  m_logdir = dirname;
  m_mac_logging = true;
}

void LteUeMac::LogMessage(std::string msg){
  std::string logfile_path = m_logdir+"MAC.log";
  std::ofstream logfile;
  logfile.open(logfile_path, std::ios_base::app);
  logfile <<  m_imsi << "," << msg << "," << Simulator::Now().GetMilliSeconds() << "\n";
  logfile.close();
}

} // namespace ns3
