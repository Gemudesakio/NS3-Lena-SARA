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

  static void LogMsg1 (uint32_t nodeId, uint64_t imsi, uint8_t preambleId, uint16_t raRnti);
  static void LogMsg2Select (uint32_t nodeId, uint64_t imsi, uint8_t rapid,
                             uint16_t tcRnti, bool sara, uint8_t groupSize, uint8_t tag);
  static void LogMsg3Ue (uint32_t nodeId, uint64_t imsi, uint16_t tcRnti, uint8_t lcid,
                         bool sara, uint8_t codebook, uint8_t dmrs);
  static void LogMsg3EnbRx (uint16_t tcRnti, uint8_t lcid, bool sara, uint8_t codebook);
  static void LogMsg3Separated (uint16_t tcRnti, uint8_t codebook, bool accepted);
  static void LogMsg4Enb (uint16_t tcRnti, uint64_t imsi, uint16_t cRnti, bool sara);
  static void LogMsg4Ue (uint32_t nodeId, uint64_t imsi, uint16_t tcRnti, uint16_t cRnti, bool sara);
  static void LogMsg5Tx (uint32_t nodeId, uint64_t imsi, uint16_t cRnti);
  static void LogMsg5Enb (uint64_t imsi, uint16_t cRnti);

private:
  static double Now ();
};

} // namespace ns3

#endif // SARA_REPORT_H
