#include "nb-iot-traffic-helper.h"

#include "ns3/double.h"
#include "ns3/fatal-error.h"
#include "ns3/log.h"
#include "ns3/random-variable-stream.h"

#include <algorithm>
#include <cctype>
#include <limits>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("NbIotTrafficHelper");

std::string
NbIotTrafficHelper::ToLower (const std::string& input)
{
  std::string out = input;
  std::transform (out.begin (), out.end (), out.begin (),
                  [] (unsigned char c) { return static_cast<char> (std::tolower (c)); });
  return out;
}

uint32_t
NbIotTrafficHelper::ResolvePopulation (const std::string& preset, uint32_t customPopulation)
{
  const std::string p = ToLower (preset);
  if (p == "avg")
    {
      return 52500;
    }
  if (p == "high")
    {
      return 200000;
    }
  if (p == "ultra")
    {
      return 500000;
    }
  if (p == "custom")
    {
      if (customPopulation == 0)
        {
          NS_FATAL_ERROR ("populationPreset=custom requiere customPopulation>0");
        }
      return customPopulation;
    }
  NS_FATAL_ERROR ("populationPreset invalido='" << preset << "'. Use avg|high|ultra|custom");
  return 0;
}

double
NbIotTrafficHelper::ResolvePeriodSeconds (const std::string& profile, double customPeriodSeconds)
{
  const std::string p = ToLower (profile);
  if (p == "2h")
    {
      return 2.0 * 3600.0;
    }
  if (p == "10m")
    {
      return 10.0 * 60.0;
    }
  if (p == "custom")
    {
      if (customPeriodSeconds <= 0.0)
        {
          NS_FATAL_ERROR ("trafficProfile=custom requiere customPeriodSeconds>0");
        }
      return customPeriodSeconds;
    }
  NS_FATAL_ERROR ("trafficProfile invalido='" << profile << "'. Use 2h|10m|custom");
  return 0.0;
}

NbIotTrafficHelper::Result
NbIotTrafficHelper::BuildSchedule (const Config& cfg, double horizonSeconds)
{
  if (horizonSeconds <= 0.0)
    {
      NS_FATAL_ERROR ("horizonSeconds debe ser > 0");
    }

  Result result;
  result.population = ResolvePopulation (cfg.populationPreset, cfg.customPopulation);
  result.periodSeconds = ResolvePeriodSeconds (cfg.trafficProfile, cfg.customPeriodSeconds);
  result.lambdaPerSecond = static_cast<double> (result.population) / result.periodSeconds;
  result.arrivalTimesSeconds.clear ();

  if (result.lambdaPerSecond <= 0.0)
    {
      return result;
    }

  uint32_t maxArrivals = cfg.maxArrivals;
  const uint32_t unlimited = std::numeric_limits<uint32_t>::max ();
  if (maxArrivals == 0)
    {
      maxArrivals = unlimited;
    }

  Ptr<ExponentialRandomVariable> interArrival = CreateObject<ExponentialRandomVariable> ();
  interArrival->SetAttribute ("Mean", DoubleValue (1.0 / result.lambdaPerSecond));

  double t = 0.0;
  while (result.arrivalTimesSeconds.size () < maxArrivals)
    {
      t += interArrival->GetValue ();
      if (t > horizonSeconds)
        {
          break;
        }
      result.arrivalTimesSeconds.push_back (t);
    }

  return result;
}

} // namespace ns3
