#ifndef SARA_MSG3_GROUP_TAG_H
#define SARA_MSG3_GROUP_TAG_H

#include "ns3/tag.h"
#include "ns3/type-id.h"
#include <stdint.h>

namespace ns3 {

class SaraMsg3GroupTag : public Tag
{
public:
  SaraMsg3GroupTag ();
  SaraMsg3GroupTag (uint16_t tempRnti, uint64_t windowEnd, bool isLast);

  void Set (uint16_t tempRnti, uint64_t windowEnd, bool isLast);
  void Get (uint16_t &tempRnti, uint64_t &windowEnd, bool &isLast) const;

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

private:
  uint16_t m_tempRnti;
  uint64_t m_windowEnd;
  uint8_t m_isLast;
};

} // namespace ns3

#endif // SARA_MSG3_GROUP_TAG_H
