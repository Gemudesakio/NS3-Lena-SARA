#include "sara-report.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <map>
#include <set>
#include <utility>
#include <vector>

#include "ns3/simulator.h"

namespace ns3 {

namespace
{
struct Msg1Event
{
  double t;
  uint32_t node;
  uint64_t imsi;
  uint16_t raRnti;
  uint8_t preamble;
  int32_t ceLevel;
  int32_t rapid;
};

struct Msg4Event
{
  double t;
  uint32_t node;
  uint64_t imsi;
  uint16_t tcRnti;
  uint16_t cRnti;
  bool sara;
};
} // namespace

static bool g_enabled = false;
static std::string g_prefix;

static std::ofstream g_msg1;
static std::ofstream g_msg2;
static std::ofstream g_msg3Ue;
static std::ofstream g_msg3Enb;
static std::ofstream g_msg3Sep;
static std::ofstream g_msg3Rrc;
static std::ofstream g_msg3Fwd;
static std::ofstream g_ctxEnb;
static std::ofstream g_msg4Enb;
static std::ofstream g_msg4Ue;
static std::ofstream g_msg4AirEnb;
static std::ofstream g_msg4RxUe;
static std::ofstream g_msg4PhyUe;
static std::ofstream g_dciNbUe;
static std::ofstream g_expectedTbUe;
static std::ofstream g_msg5Tx;
static std::ofstream g_msg5Enb;
static std::ofstream g_transition;
static std::ofstream g_summary;

static std::vector<Msg1Event> g_msg1Events;
static std::vector<Msg4Event> g_msg4Events;

static std::map<uint64_t, uint32_t> g_imsiToNode;
static std::set<uint64_t> g_msg2OkUniqueImsi;
static std::set<uint64_t> g_msg3RrcUniqueImsi;
static std::set<uint64_t> g_msg4TxUniqueImsi;
static std::set<uint64_t> g_msg5CompleteUniqueImsi;

double
SaraReport::Now ()
{
  return Simulator::Now ().GetSeconds ();
}

bool
SaraReport::IsEnabled ()
{
  return g_enabled;
}

uint32_t
SaraReport::GetMsg4UeAcceptedCount ()
{
  return static_cast<uint32_t> (g_msg4Events.size ());
}

uint32_t
SaraReport::GetMsg4UeAcceptedUniqueCount ()
{
  std::set<uint64_t> imsis;
  for (const Msg4Event &e : g_msg4Events)
    {
      imsis.insert (e.imsi);
    }
  return static_cast<uint32_t> (imsis.size ());
}

void
SaraReport::Enable (const std::string &prefix)
{
  if (g_enabled)
    {
      return;
    }
  g_enabled = true;
  g_prefix = prefix;

  g_msg1Events.clear ();
  g_msg4Events.clear ();
  g_imsiToNode.clear ();
  g_msg2OkUniqueImsi.clear ();
  g_msg3RrcUniqueImsi.clear ();
  g_msg4TxUniqueImsi.clear ();
  g_msg5CompleteUniqueImsi.clear ();

  g_msg1.open ((g_prefix + "_msg1.csv").c_str ());
  g_msg2.open ((g_prefix + "_msg2.csv").c_str ());
  g_msg3Ue.open ((g_prefix + "_msg3_ue.csv").c_str ());
  g_msg3Enb.open ((g_prefix + "_msg3_enb.csv").c_str ());
  g_msg3Sep.open ((g_prefix + "_msg3_sep.csv").c_str ());
  g_msg3Rrc.open ((g_prefix + "_msg3_rrc.csv").c_str ());
  g_msg3Fwd.open ((g_prefix + "_msg3_fwd.csv").c_str ());
  g_ctxEnb.open ((g_prefix + "_ctx_enb.csv").c_str ());
  g_msg4Enb.open ((g_prefix + "_msg4_enb.csv").c_str ());
  g_msg4Ue.open ((g_prefix + "_msg4_ue.csv").c_str ());
  g_msg4AirEnb.open ((g_prefix + "_msg4_air_enb.csv").c_str ());
  g_msg4RxUe.open ((g_prefix + "_msg4_rx_ue.csv").c_str ());
  g_msg4PhyUe.open ((g_prefix + "_msg4_phy_ue.csv").c_str ());
  g_dciNbUe.open ((g_prefix + "_dci_nb_ue.csv").c_str ());
  g_expectedTbUe.open ((g_prefix + "_expected_tb_ue.csv").c_str ());
  g_msg5Tx.open ((g_prefix + "_msg5_tx.csv").c_str ());
  g_msg5Enb.open ((g_prefix + "_msg5_enb.csv").c_str ());
  g_transition.open ((g_prefix + "_transition.csv").c_str ());
  g_summary.open ((g_prefix + "_summary.csv").c_str ());

  g_msg1 << "time_s,node,imsi,ra_rnti,preamble,ce_level,rapid\n";
  g_msg2 << "time_s,node,imsi,rapid,tc_rnti,sara,group_size,tag,toa_valid,toa_bin_ue,toa_bin_rar,virtual_id,codebook_id,decision,drop_reason\n";
  g_msg3Ue << "time_s,node,imsi,tc_rnti,lcid,sara,codebook,dmrs,virtual_id,physical_carrier,decision,drop_reason\n";
  g_msg3Enb << "time_s,tc_rnti,imsi,lcid,sara,codebook,virtual_id,physical_carrier,decision,drop_reason\n";
  g_msg3Sep << "time_s,tc_rnti,imsi,codebook,accepted\n";
  g_msg3Rrc << "time_s,tc_rnti,imsi,lcid,sara,codebook,virtual_id,physical_carrier\n";
  g_msg3Fwd << "time_s,tc_rnti,imsi,lcid,sara,codebook,decision,reason\n";
  g_ctxEnb << "time_s,layer,event,rnti,imsi\n";
  g_msg4Enb << "time_s,tc_rnti,imsi,c_rnti,sara\n";
  g_msg4Ue << "time_s,node,imsi,tc_rnti,c_rnti,sara\n";
  g_msg4AirEnb << "time_s,tc_rnti,imsi,c_rnti,sara,bytes\n";
  g_msg4RxUe << "time_s,node,ue_imsi,ue_state,tc_rnti,msg_imsi,c_rnti,decision,drop_reason\n";
  g_msg4PhyUe << "time_s,node,tc_rnti,lcid,pkt_uid,msg_imsi,c_rnti,event\n";
  g_dciNbUe << "time_s,node,rnti,curr_sf,npdsch_first,npdsch_last,wait_ms\n";
  g_expectedTbUe << "time_s,node,rnti\n";
  g_msg5Tx << "time_s,node,imsi,c_rnti\n";
  g_msg5Enb << "time_s,imsi,c_rnti\n";
  g_transition << "imsi,node,tc_rnti,c_rnti,msg4_time_s,latencia_real_s,latencia_instantanea_s\n";
  g_summary << "metric,value\n";

  g_msg1 << std::fixed << std::setprecision (6);
  g_msg2 << std::fixed << std::setprecision (6);
  g_msg3Ue << std::fixed << std::setprecision (6);
  g_msg3Enb << std::fixed << std::setprecision (6);
  g_msg3Sep << std::fixed << std::setprecision (6);
  g_msg3Rrc << std::fixed << std::setprecision (6);
  g_msg3Fwd << std::fixed << std::setprecision (6);
  g_ctxEnb << std::fixed << std::setprecision (6);
  g_msg4Enb << std::fixed << std::setprecision (6);
  g_msg4Ue << std::fixed << std::setprecision (6);
  g_msg4AirEnb << std::fixed << std::setprecision (6);
  g_msg4RxUe << std::fixed << std::setprecision (6);
  g_msg4PhyUe << std::fixed << std::setprecision (6);
  g_dciNbUe << std::fixed << std::setprecision (6);
  g_expectedTbUe << std::fixed << std::setprecision (6);
  g_msg5Tx << std::fixed << std::setprecision (6);
  g_msg5Enb << std::fixed << std::setprecision (6);
  g_transition << std::fixed << std::setprecision (6);
  g_summary << std::fixed << std::setprecision (6);
}

void
SaraReport::LogMsg1 (uint32_t nodeId, uint64_t imsi, uint8_t preambleId, uint16_t raRnti,
                     int32_t ceLevel, int32_t rapid)
{
  if (!g_enabled)
    {
      return;
    }
  const double t = Now ();
  g_msg1 << t << "," << nodeId << "," << imsi << "," << raRnti << ","
         << (uint32_t) preambleId << "," << ceLevel << "," << rapid << "\n";
  g_msg1Events.push_back (Msg1Event {t, nodeId, imsi, raRnti, preambleId, ceLevel, rapid});
  g_imsiToNode[imsi] = nodeId;
}

void
SaraReport::LogMsg2Select (uint32_t nodeId, uint64_t imsi, uint8_t rapid,
                           uint16_t tcRnti, bool sara, uint8_t groupSize, uint8_t tag,
                           int32_t toaValid, int32_t toaBinUe, int32_t toaBinRar,
                           int32_t virtualId, int32_t codebookId, int32_t decision,
                           int32_t dropReason)
{
  if (!g_enabled)
    {
      return;
    }
  g_msg2 << Now () << "," << nodeId << "," << imsi << "," << (uint32_t) rapid << ","
         << tcRnti << "," << (sara ? 1 : 0) << "," << (uint32_t) groupSize << ","
         << (uint32_t) tag << "," << toaValid << "," << toaBinUe << ","
         << toaBinRar << "," << virtualId << "," << codebookId << ","
         << decision << "," << dropReason << "\n";
  if ((decision == 1) && (imsi != 0))
    {
      g_msg2OkUniqueImsi.insert (imsi);
    }
  g_imsiToNode[imsi] = nodeId;
}

void
SaraReport::LogMsg3Ue (uint32_t nodeId, uint64_t imsi, uint16_t tcRnti, uint8_t lcid,
                       bool sara, uint8_t codebook, uint8_t dmrs,
                       int32_t virtualId, int32_t physicalCarrier,
                       int32_t decision, int32_t dropReason)
{
  if (!g_enabled)
    {
      return;
    }
  g_msg3Ue << Now () << "," << nodeId << "," << imsi << "," << tcRnti << ","
           << (uint32_t) lcid << "," << (sara ? 1 : 0) << ","
           << (uint32_t) codebook << "," << (uint32_t) dmrs << ","
           << virtualId << "," << physicalCarrier << ","
           << decision << "," << dropReason << "\n";
  g_imsiToNode[imsi] = nodeId;
}

void
SaraReport::LogMsg3EnbRx (uint16_t tcRnti, uint64_t imsi, uint8_t lcid, bool sara, uint8_t codebook,
                          int32_t virtualId, int32_t physicalCarrier,
                          int32_t decision, int32_t dropReason)
{
  if (!g_enabled)
    {
      return;
    }
  g_msg3Enb << Now () << "," << tcRnti << "," << imsi << "," << (uint32_t) lcid << ","
            << (sara ? 1 : 0) << "," << (uint32_t) codebook << ","
            << virtualId << "," << physicalCarrier << ","
            << decision << "," << dropReason << "\n";
}

void
SaraReport::LogMsg3Separated (uint16_t tcRnti, uint64_t imsi, uint8_t codebook, bool accepted)
{
  if (!g_enabled)
    {
      return;
    }
  g_msg3Sep << Now () << "," << tcRnti << "," << imsi << "," << (uint32_t) codebook << ","
            << (accepted ? 1 : 0) << "\n";
}

void
SaraReport::LogMsg3ToRrc (uint16_t tcRnti, uint64_t imsi, uint8_t lcid, bool sara, uint8_t codebook,
                          int32_t virtualId, int32_t physicalCarrier)
{
  if (!g_enabled)
    {
      return;
    }
  g_msg3Rrc << Now () << "," << tcRnti << "," << imsi << "," << (uint32_t) lcid << ","
            << (sara ? 1 : 0) << "," << (uint32_t) codebook << ","
            << virtualId << "," << physicalCarrier << "\n";
  if (imsi != 0)
    {
      g_msg3RrcUniqueImsi.insert (imsi);
    }
}

void
SaraReport::LogMsg3Forward (uint16_t tcRnti, uint64_t imsi, uint8_t lcid, bool sara, uint8_t codebook,
                            uint8_t decision, uint8_t reason)
{
  if (!g_enabled)
    {
      return;
    }
  g_msg3Fwd << Now () << "," << tcRnti << "," << imsi << "," << static_cast<uint32_t> (lcid) << ","
            << (sara ? 1 : 0) << "," << static_cast<uint32_t> (codebook) << ","
            << static_cast<uint32_t> (decision) << "," << static_cast<uint32_t> (reason) << "\n";
}

void
SaraReport::LogEnbContextEvent (const std::string &layer, const std::string &event,
                                uint16_t rnti, uint64_t imsi)
{
  if (!g_enabled)
    {
      return;
    }
  g_ctxEnb << Now () << "," << layer << "," << event << "," << rnti << "," << imsi << "\n";
}

void
SaraReport::LogMsg4Enb (uint16_t tcRnti, uint64_t imsi, uint16_t cRnti, bool sara)
{
  if (!g_enabled)
    {
      return;
    }
  g_msg4Enb << Now () << "," << tcRnti << "," << imsi << "," << cRnti << ","
            << (sara ? 1 : 0) << "\n";
  if (imsi != 0)
    {
      g_msg4TxUniqueImsi.insert (imsi);
    }
}

void
SaraReport::LogMsg4Ue (uint32_t nodeId, uint64_t imsi, uint16_t tcRnti, uint16_t cRnti, bool sara)
{
  if (!g_enabled)
    {
      return;
    }
  const double t = Now ();
  g_msg4Ue << t << "," << nodeId << "," << imsi << "," << tcRnti << "," << cRnti << ","
           << (sara ? 1 : 0) << "\n";
  g_msg4Events.push_back (Msg4Event {t, nodeId, imsi, tcRnti, cRnti, sara});
  g_imsiToNode[imsi] = nodeId;
}

void
SaraReport::LogMsg4AirEnb (uint16_t tcRnti, uint64_t imsi, uint16_t cRnti, bool sara, uint32_t bytes)
{
  if (!g_enabled)
    {
      return;
    }
  g_msg4AirEnb << Now () << "," << tcRnti << "," << imsi << "," << cRnti << ","
               << (sara ? 1 : 0) << "," << bytes << "\n";
}

void
SaraReport::LogMsg4RxUe (uint32_t nodeId, uint64_t ueImsi, uint16_t ueState, uint16_t tcRnti,
                         uint64_t msgImsi, uint16_t cRnti, uint8_t decision, uint8_t dropReason)
{
  if (!g_enabled)
    {
      return;
    }
  g_msg4RxUe << Now () << "," << nodeId << "," << ueImsi << "," << ueState << ","
             << tcRnti << "," << msgImsi << "," << cRnti << ","
             << static_cast<uint32_t> (decision) << "," << static_cast<uint32_t> (dropReason) << "\n";
}

void
SaraReport::LogMsg4PhyUe (uint32_t nodeId, uint16_t tcRnti, uint8_t lcid, uint32_t pktUid,
                          uint64_t msgImsi, uint16_t cRnti, uint8_t event)
{
  if (!g_enabled)
    {
      return;
    }
  g_msg4PhyUe << Now () << "," << nodeId << "," << tcRnti << "," << (uint32_t) lcid << ","
              << pktUid << "," << msgImsi << "," << cRnti << "," << (uint32_t) event << "\n";
}

void
SaraReport::LogDlDciNbUe (uint32_t nodeId, uint16_t rnti, uint64_t currSf, uint64_t npdschFirst,
                          uint64_t npdschLast, uint64_t waitMs)
{
  if (!g_enabled)
    {
      return;
    }
  g_dciNbUe << Now () << "," << nodeId << "," << rnti << "," << currSf << ","
            << npdschFirst << "," << npdschLast << "," << waitMs << "\n";
}

void
SaraReport::LogExpectedTbAddUe (uint32_t nodeId, uint16_t rnti)
{
  if (!g_enabled)
    {
      return;
    }
  g_expectedTbUe << Now () << "," << nodeId << "," << rnti << "\n";
}

void
SaraReport::LogMsg5Tx (uint32_t nodeId, uint64_t imsi, uint16_t cRnti)
{
  if (!g_enabled)
    {
      return;
    }
  g_msg5Tx << Now () << "," << nodeId << "," << imsi << "," << cRnti << "\n";
  g_imsiToNode[imsi] = nodeId;
}

void
SaraReport::LogMsg5Enb (uint64_t imsi, uint16_t cRnti)
{
  if (!g_enabled)
    {
      return;
    }
  g_msg5Enb << Now () << "," << imsi << "," << cRnti << "\n";
  if (imsi != 0)
    {
      g_msg5CompleteUniqueImsi.insert (imsi);
    }
}

void
SaraReport::Finalize ()
{
  if (!g_enabled)
    {
      return;
    }

  // Build per-IMSI lists
  std::map<uint64_t, std::vector<Msg1Event> > msg1ByImsi;
  std::map<uint64_t, std::vector<Msg4Event> > msg4ByImsi;
  for (const Msg1Event &e : g_msg1Events)
    {
      msg1ByImsi[e.imsi].push_back (e);
    }
  for (const Msg4Event &e : g_msg4Events)
    {
      msg4ByImsi[e.imsi].push_back (e);
    }

  // Transition table: earliest Msg4 per IMSI
  for (std::map<uint64_t, std::vector<Msg4Event> >::iterator it = msg4ByImsi.begin ();
       it != msg4ByImsi.end (); ++it)
    {
      std::vector<Msg4Event> &v = it->second;
      std::sort (v.begin (), v.end (),
                 [] (const Msg4Event &a, const Msg4Event &b) { return a.t < b.t; });
      const Msg4Event &e = v.front ();
      uint32_t nodeId = 0;
      std::map<uint64_t, uint32_t>::const_iterator nIt = g_imsiToNode.find (e.imsi);
      if (nIt != g_imsiToNode.end ())
        {
          nodeId = nIt->second;
        }
      double latencyReal = 0.0;
      double latencyInstant = 0.0;
      std::vector<Msg1Event> &v1 = msg1ByImsi[e.imsi];
      if (!v1.empty ())
        {
          std::sort (v1.begin (), v1.end (),
                     [] (const Msg1Event &a, const Msg1Event &b) { return a.t < b.t; });
          Msg1Event first = v1.front ();
          Msg1Event lastBefore = v1.front ();
          for (const Msg1Event &m1 : v1)
            {
              if (m1.t <= e.t)
                {
                  lastBefore = m1;
                }
            }
          latencyReal = e.t - first.t;
          latencyInstant = e.t - lastBefore.t;
        }
      g_transition << e.imsi << "," << nodeId << "," << e.tcRnti << ","
                   << e.cRnti << "," << e.t << "," << latencyReal << ","
                   << latencyInstant << "\n";
    }

  const uint32_t totalAttempts = static_cast<uint32_t> (g_msg1Events.size ());
  const uint32_t uniqueUesAttempted = static_cast<uint32_t> (msg1ByImsi.size ());
  const uint32_t uniqueUesSuccess = static_cast<uint32_t> (msg4ByImsi.size ());

  // Preamble collisions
  std::map<std::pair<uint16_t, uint8_t>, uint32_t> preambleCount;
  for (const Msg1Event &e : g_msg1Events)
    {
      preambleCount[std::make_pair (e.raRnti, e.preamble)]++;
    }
  uint32_t collidingPreambles = 0;
  uint32_t collidingUes = 0;
  for (std::map<std::pair<uint16_t, uint8_t>, uint32_t>::const_iterator it = preambleCount.begin ();
       it != preambleCount.end (); ++it)
    {
      if (it->second > 1)
        {
          ++collidingPreambles;
          collidingUes += it->second;
        }
    }

  // Latency + first attempt success
  double sumLatencyInstant = 0.0;
  double sumLatencyReal = 0.0;
  uint32_t successCount = 0;
  uint32_t firstAttemptSuccess = 0;
  std::map<uint16_t, uint32_t> raRntiSuccessCount;

  for (std::map<uint64_t, std::vector<Msg4Event> >::iterator it = msg4ByImsi.begin ();
       it != msg4ByImsi.end (); ++it)
    {
      const uint64_t imsi = it->first;
      std::vector<Msg4Event> &v4 = it->second;
      std::sort (v4.begin (), v4.end (),
                 [] (const Msg4Event &a, const Msg4Event &b) { return a.t < b.t; });
      const double t4 = v4.front ().t;

      std::vector<Msg1Event> &v1 = msg1ByImsi[imsi];
      if (v1.empty ())
        {
          continue;
        }
      std::sort (v1.begin (), v1.end (),
                 [] (const Msg1Event &a, const Msg1Event &b) { return a.t < b.t; });

      Msg1Event chosen = v1.front ();
      uint32_t attemptsBefore = 0;
      for (const Msg1Event &e : v1)
        {
          if (e.t <= t4)
            {
              chosen = e;
              ++attemptsBefore;
            }
        }

      sumLatencyInstant += (t4 - chosen.t);
      sumLatencyReal += (t4 - v1.front ().t);
      ++successCount;
      if (attemptsBefore <= 1)
        {
          ++firstAttemptSuccess;
        }
      raRntiSuccessCount[chosen.raRnti]++;
    }

  double avgLatencyInstant = 0.0;
  double avgLatencyReal = 0.0;
  if (successCount > 0)
    {
      avgLatencyInstant = sumLatencyInstant / successCount;
      avgLatencyReal = sumLatencyReal / successCount;
    }

  double accessEfficiency = 0.0;
  if (totalAttempts > 0)
    {
      accessEfficiency = static_cast<double> (successCount) / totalAttempts;
    }

  double successProbability = 0.0;
  if (uniqueUesAttempted > 0)
    {
      successProbability = static_cast<double> (uniqueUesSuccess) / uniqueUesAttempted;
    }

  double firstAttemptRate = 0.0;
  if (uniqueUesSuccess > 0)
    {
      firstAttemptRate = static_cast<double> (firstAttemptSuccess) / uniqueUesSuccess;
    }

  double collisionRate = 0.0;
  if (totalAttempts > 0)
    {
      collisionRate = static_cast<double> (collidingUes) / totalAttempts;
    }

  uint32_t noraCapacity = 0;
  for (std::map<uint16_t, uint32_t>::const_iterator it = raRntiSuccessCount.begin ();
       it != raRntiSuccessCount.end (); ++it)
    {
      noraCapacity = std::max (noraCapacity, it->second);
    }

  g_summary << "total_intentos," << totalAttempts << "\n";
  g_summary << "ues_unicos_intento," << uniqueUesAttempted << "\n";
  g_summary << "msg2_ok_ues_unicos," << g_msg2OkUniqueImsi.size () << "\n";
  g_summary << "msg3_rrc_ues_unicos," << g_msg3RrcUniqueImsi.size () << "\n";
  g_summary << "msg4_tx_ues_unicos," << g_msg4TxUniqueImsi.size () << "\n";
  g_summary << "msg5_complete_ues_unicos," << g_msg5CompleteUniqueImsi.size () << "\n";
  g_summary << "eficiencia_acceso," << accessEfficiency << "\n";
  g_summary << "probabilidad_exito," << successProbability << "\n";
  g_summary << "tasa_exito_primer_intento," << firstAttemptRate << "\n";
  g_summary << "latencia_acceso_promedio_real_s," << avgLatencyReal << "\n";
  g_summary << "latencia_acceso_promedio_instantanea_s," << avgLatencyInstant << "\n";
  g_summary << "tasa_colision_preambulo," << collisionRate << "\n";
  g_summary << "preambulos_colisionados," << collidingPreambles << "\n";
  g_summary << "ues_colisionados," << collidingUes << "\n";
  g_summary << "capacidad_nora," << noraCapacity << "\n";

  g_msg1.close ();
  g_msg2.close ();
  g_msg3Ue.close ();
  g_msg3Enb.close ();
  g_msg3Sep.close ();
  g_msg3Rrc.close ();
  g_msg3Fwd.close ();
  g_ctxEnb.close ();
  g_msg4Enb.close ();
  g_msg4Ue.close ();
  g_msg4AirEnb.close ();
  g_msg4RxUe.close ();
  g_msg4PhyUe.close ();
  g_dciNbUe.close ();
  g_expectedTbUe.close ();
  g_msg5Tx.close ();
  g_msg5Enb.close ();
  g_transition.close ();
  g_summary.close ();

  g_enabled = false;
}

} // namespace ns3
