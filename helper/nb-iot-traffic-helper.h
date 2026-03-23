#ifndef NB_IOT_TRAFFIC_HELPER_H
#define NB_IOT_TRAFFIC_HELPER_H

#include <cstdint>
#include <string>
#include <vector>

namespace ns3
{

/**
 * Helper to build NB-IoT access arrival schedules from virtual-population
 * traffic models without touching MAC/RRC random-access logic.
 */
class NbIotTrafficHelper
{
public:
  struct Config
  {
    std::string populationPreset;   //!< avg|high|ultra|custom
    std::string trafficProfile;     //!< 2h|10m|custom
    uint32_t customPopulation;      //!< used when populationPreset=custom
    double customPeriodSeconds;     //!< used when trafficProfile=custom
    uint32_t maxArrivals;           //!< 0: unlimited
  };

  struct Result
  {
    uint32_t population;
    double periodSeconds;
    double lambdaPerSecond;
    std::vector<double> arrivalTimesSeconds;
  };

  /**
   * Build schedule in [0, horizonSeconds] according to Config.
   */
  static Result BuildSchedule (const Config& cfg, double horizonSeconds);

private:
  static uint32_t ResolvePopulation (const std::string& preset, uint32_t customPopulation);
  static double ResolvePeriodSeconds (const std::string& profile, double customPeriodSeconds);
  static std::string ToLower (const std::string& input);
};

} // namespace ns3

#endif // NB_IOT_TRAFFIC_HELPER_H
