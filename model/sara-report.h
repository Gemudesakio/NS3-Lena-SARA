#ifndef SARA_REPORT_H
#define SARA_REPORT_H

#include <cstdint>
#include <string>

namespace ns3 {

class SaraReport
{
public:
  static void Enable (const std::string &prefix);
  static void Finalize ();
  static bool IsEnabled ();

  // Summary helpers (usable even after Finalize()).
  static uint32_t GetMsg4UeAcceptedCount ();
  static uint32_t GetMsg4UeAcceptedUniqueCount ();

  // Extra debug visibility for Msg4 delivery chain.
  // "Air" here means the PDU reached eNB MAC (i.e., got a Tx opportunity).
  static void LogMsg4AirEnb (uint16_t tcRnti, uint64_t imsi, uint16_t cRnti, bool sara, uint32_t bytes);
  // Msg4 reception at UE RRC protocol (after lower layers delivered the PDU).
  // decision: 1 accepted/passed upward, 0 dropped.
  // dropReason: 0 none, 1 state_mismatch, 2 imsi_mismatch.
  static void LogMsg4RxUe (uint32_t nodeId, uint64_t ueImsi, uint16_t ueState, uint16_t tcRnti,
                           uint64_t msgImsi, uint16_t cRnti, uint8_t decision, uint8_t dropReason);
  // UE PHY/MAC-level visibility: whether a Msg4 PDU was processed by SpectrumPhy.
  // event: 0 delivered OK, 1 dropped (no expected TB), 2 dropped (TB corrupt).
  static void LogMsg4PhyUe (uint32_t nodeId, uint16_t tcRnti, uint8_t lcid, uint32_t pktUid,
                            uint64_t msgImsi, uint16_t cRnti, uint8_t event);
  // UE-side DCI reception and the scheduling of AddNbiotExpectedTb().
  static void LogDlDciNbUe (uint32_t nodeId, uint16_t rnti, uint64_t currSf, uint64_t npdschFirst,
                            uint64_t npdschLast, uint64_t waitMs);
  static void LogExpectedTbAddUe (uint32_t nodeId, uint16_t rnti);

  static void LogMsg1 (uint32_t nodeId, uint64_t imsi, uint8_t preambleId, uint16_t raRnti,
                       int32_t ceLevel = -1, int32_t rapid = -1);
  static void LogMsg2Select (uint32_t nodeId, uint64_t imsi, uint8_t rapid,
                             uint16_t tcRnti, bool sara, uint8_t groupSize, uint8_t tag,
                             int32_t toaValid = -1, int32_t toaBinUe = -1, int32_t toaBinRar = -1,
                             int32_t virtualId = -1, int32_t codebookId = -1,
                             int32_t decision = -1, int32_t dropReason = -1);
  static void LogMsg3Ue (uint32_t nodeId, uint64_t imsi, uint16_t tcRnti, uint8_t lcid,
                         bool sara, uint8_t codebook, uint8_t dmrs,
                         int32_t virtualId = -1, int32_t physicalCarrier = -1,
                         int32_t decision = 1, int32_t dropReason = 0);
  static void LogMsg3EnbRx (uint16_t tcRnti, uint64_t imsi, uint8_t lcid, bool sara, uint8_t codebook,
                            int32_t virtualId = -1, int32_t physicalCarrier = -1,
                            int32_t decision = 1, int32_t dropReason = 0);
  static void LogMsg3Separated (uint16_t tcRnti, uint64_t imsi, uint8_t codebook, bool accepted);
  static void LogMsg3ToRrc (uint16_t tcRnti, uint64_t imsi, uint8_t lcid, bool sara, uint8_t codebook,
                            int32_t virtualId = -1, int32_t physicalCarrier = -1);
  // Forward status at eNB MAC when attempting to hand Msg3 to RLC.
  // decision: 1 forwarded, 0 dropped.
  // reason: 0 ok, 1 unknown-rnti, 2 missing-lcid.
  static void LogMsg3Forward (uint16_t tcRnti, uint64_t imsi, uint8_t lcid, bool sara, uint8_t codebook,
                              uint8_t decision, uint8_t reason);
  // Context lifecycle trace for eNB-side RNTI debugging.
  static void LogEnbContextEvent (const std::string &layer, const std::string &event,
                                  uint16_t rnti, uint64_t imsi = 0);
  static void LogMsg4Enb (uint16_t tcRnti, uint64_t imsi, uint16_t cRnti, bool sara);
  static void LogMsg4Ue (uint32_t nodeId, uint64_t imsi, uint16_t tcRnti, uint16_t cRnti, bool sara);
  static void LogMsg5Tx (uint32_t nodeId, uint64_t imsi, uint16_t cRnti);
  static void LogMsg5Enb (uint64_t imsi, uint16_t cRnti);

private:
  static double Now ();
};

} // namespace ns3

#endif // SARA_REPORT_H
