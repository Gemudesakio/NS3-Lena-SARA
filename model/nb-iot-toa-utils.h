/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Internal helper utilities for ToA abstraction in NB-IoT random access.
 */

#ifndef NB_IOT_TOA_UTILS_H
#define NB_IOT_TOA_UTILS_H

#include <algorithm>
#include <cstdint>

namespace ns3
{
namespace NbIotToaUtils
{

inline uint32_t
ToaMetaIdFromImsi (uint64_t imsi)
{
  // Keep deterministic behavior even if IMSI exceeds 32-bit.
  const uint32_t hi = static_cast<uint32_t> (imsi >> 32);
  const uint32_t lo = static_cast<uint32_t> (imsi & 0xffffffffu);
  return hi ^ lo;
}

inline uint16_t
ComputeToaBin (uint32_t metaId, uint16_t numBins)
{
  const uint32_t bins = std::max<uint32_t> (1, numBins);
  uint32_t x = metaId;
  // Simple deterministic mixer (same on UE/eNB).
  x ^= (x << 13);
  x ^= (x >> 17);
  x ^= (x << 5);
  return static_cast<uint16_t> (x % bins);
}

inline bool
MatchToa (uint16_t ueToaBin, uint16_t rarToaBin, uint16_t toleranceBins)
{
  const uint16_t diff = (ueToaBin >= rarToaBin) ? (ueToaBin - rarToaBin)
                                                : (rarToaBin - ueToaBin);
  return diff <= toleranceBins;
}

} // namespace NbIotToaUtils
} // namespace ns3

#endif // NB_IOT_TOA_UTILS_H
