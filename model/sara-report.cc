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
static std::ofstream g_msg4Enb;
static std::ofstream g_msg4Ue;
static std::ofstream g_msg5Tx;
static std::ofstream g_msg5Enb;
static std::ofstream g_transition;
static std::ofstream g_summary;

static std::vector<Msg1Event> g_msg1Events;
static std::vector<Msg4Event> g_msg4Events;

static std::map<uint64_t, uint32_t> g_imsiToNode;

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

void
SaraReport::Enable (const std::string &prefix)
{
  if (g_enabled)
    {
      return;
    }
  g_enabled = true;
  g_prefix = prefix;

  g_msg1.open ((g_prefix + "_msg1.csv").c_str ());
  g_msg2.open ((g_prefix + "_msg2.csv").c_str ());
  g_msg3Ue.open ((g_prefix + "_msg3_ue.csv").c_str ());
  g_msg3Enb.open ((g_prefix + "_msg3_enb.csv").c_str ());
  g_msg3Sep.open ((g_prefix + "_msg3_sep.csv").c_str ());
  g_msg4Enb.open ((g_prefix + "_msg4_enb.csv").c_str ());
  g_msg4Ue.open ((g_prefix + "_msg4_ue.csv").c_str ());
  g_msg5Tx.open ((g_prefix + "_msg5_tx.csv").c_str ());
  g_msg5Enb.open ((g_prefix + "_msg5_enb.csv").c_str ());
  g_transition.open ((g_prefix + "_transition.csv").c_str ());
  g_summary.open ((g_prefix + "_summary.csv").c_str ());

  g_msg1 << "time_s,node,imsi,ra_rnti,preamble\n";
  g_msg2 << "time_s,node,imsi,rapid,tc_rnti,sara,group_size,tag\n";
  g_msg3Ue << "time_s,node,imsi,tc_rnti,lcid,sara,codebook,dmrs\n";
  g_msg3Enb << "time_s,tc_rnti,lcid,sara,codebook\n";
  g_msg3Sep << "time_s,tc_rnti,codebook,accepted\n";
  g_msg4Enb << "time_s,tc_rnti,imsi,c_rnti,sara\n";
  g_msg4Ue << "time_s,node,imsi,tc_rnti,c_rnti,sara\n";
  g_msg5Tx << "time_s,node,imsi,c_rnti\n";
  g_msg5Enb << "time_s,imsi,c_rnti\n";
  g_transition << "imsi,node,tc_rnti,c_rnti,msg4_time_s\n";
  g_summary << "metric,value\n";

  g_msg1 << std::fixed << std::setprecision (6);
  g_msg2 << std::fixed << std::setprecision (6);
  g_msg3Ue << std::fixed << std::setprecision (6);
  g_msg3Enb << std::fixed << std::setprecision (6);
  g_msg3Sep << std::fixed << std::setprecision (6);
  g_msg4Enb << std::fixed << std::setprecision (6);
  g_msg4Ue << std::fixed << std::setprecision (6);
  g_msg5Tx << std::fixed << std::setprecision (6);
  g_msg5Enb << std::fixed << std::setprecision (6);
  g_transition << std::fixed << std::setprecision (6);
  g_summary << std::fixed << std::setprecision (6);
}

void
SaraReport::LogMsg1 (uint32_t nodeId, uint64_t imsi, uint8_t preambleId, uint16_t raRnti)
{
  if (!g_enabled)
    {
      return;
    }
  const double t = Now ();
  g_msg1 << t << "," << nodeId << "," << imsi << "," << raRnti << "," << (uint32_t) preambleId << "\n";
  g_msg1Events.push_back (Msg1Event {t, nodeId, imsi, raRnti, preambleId});
  g_imsiToNode[imsi] = nodeId;
}

void
SaraReport::LogMsg2Select (uint32_t nodeId, uint64_t imsi, uint8_t rapid,
                           uint16_t tcRnti, bool sara, uint8_t groupSize, uint8_t tag)
{
  if (!g_enabled)
    {
      return;
    }
  g_msg2 << Now () << "," << nodeId << "," << imsi << "," << (uint32_t) rapid << ","
         << tcRnti << "," << (sara ? 1 : 0) << "," << (uint32_t) groupSize << ","
         << (uint32_t) tag << "\n";
  g_imsiToNode[imsi] = nodeId;
}

void
SaraReport::LogMsg3Ue (uint32_t nodeId, uint64_t imsi, uint16_t tcRnti, uint8_t lcid,
                       bool sara, uint8_t codebook, uint8_t dmrs)
{
  if (!g_enabled)
    {
      return;
    }
  g_msg3Ue << Now () << "," << nodeId << "," << imsi << "," << tcRnti << ","
           << (uint32_t) lcid << "," << (sara ? 1 : 0) << ","
           << (uint32_t) codebook << "," << (uint32_t) dmrs << "\n";
  g_imsiToNode[imsi] = nodeId;
}

void
SaraReport::LogMsg3EnbRx (uint16_t tcRnti, uint8_t lcid, bool sara, uint8_t codebook)
{
  if (!g_enabled)
    {
      return;
    }
  g_msg3Enb << Now () << "," << tcRnti << "," << (uint32_t) lcid << ","
            << (sara ? 1 : 0) << "," << (uint32_t) codebook << "\n";
}

void
SaraReport::LogMsg3Separated (uint16_t tcRnti, uint8_t codebook, bool accepted)
{
  if (!g_enabled)
    {
      return;
    }
  g_msg3Sep << Now () << "," << tcRnti << "," << (uint32_t) codebook << ","
            << (accepted ? 1 : 0) << "\n";
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
      g_transition << e.imsi << "," << nodeId << "," << e.tcRnti << ","
                   << e.cRnti << "," << e.t << "\n";
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
  double sumLatency = 0.0;
  double sumLatencyFirst = 0.0;
  uint32_t successCount = 0;
  uint32_t firstAttemptSuccess = 0;
  uint32_t firstAttemptLatencyCount = 0;
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

      sumLatency += (t4 - chosen.t);
      ++successCount;
      if (attemptsBefore <= 1)
        {
          ++firstAttemptSuccess;
          sumLatencyFirst += (t4 - chosen.t);
          ++firstAttemptLatencyCount;
        }
      raRntiSuccessCount[chosen.raRnti]++;
    }

  double avgLatency = 0.0;
  if (successCount > 0)
    {
      avgLatency = sumLatency / successCount;
    }
  double avgLatencyFirst = 0.0;
  if (firstAttemptLatencyCount > 0)
    {
      avgLatencyFirst = sumLatencyFirst / firstAttemptLatencyCount;
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
  g_summary << "ues_unicos_exito," << uniqueUesSuccess << "\n";
  g_summary << "eficiencia_acceso," << accessEfficiency << "\n";
  g_summary << "probabilidad_exito," << successProbability << "\n";
  g_summary << "tasa_exito_primer_intento," << firstAttemptRate << "\n";
  g_summary << "latencia_acceso_promedio_total_s," << avgLatency << "\n";
  g_summary << "latencia_acceso_promedio_primer_intento_s," << avgLatencyFirst << "\n";
  g_summary << "tasa_colision_preambulo," << collisionRate << "\n";
  g_summary << "preambulos_colisionados," << collidingPreambles << "\n";
  g_summary << "ues_colisionados," << collidingUes << "\n";
  g_summary << "capacidad_nora," << noraCapacity << "\n";

  g_msg1.close ();
  g_msg2.close ();
  g_msg3Ue.close ();
  g_msg3Enb.close ();
  g_msg3Sep.close ();
  g_msg4Enb.close ();
  g_msg4Ue.close ();
  g_msg5Tx.close ();
  g_msg5Enb.close ();
  g_transition.close ();
  g_summary.close ();

  g_enabled = false;
}

} // namespace ns3
