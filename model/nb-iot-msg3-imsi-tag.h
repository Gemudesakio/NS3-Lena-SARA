#ifndef NB_IOT_MSG3_IMSI_TAG_H
#define NB_IOT_MSG3_IMSI_TAG_H

#include "ns3/tag.h"
#include "ns3/type-id.h"
#include <stdint.h>

namespace ns3 {

class NbIotMsg3ImsiTag : public Tag
{
public:
  NbIotMsg3ImsiTag ();
  explicit NbIotMsg3ImsiTag (uint64_t imsi);

  void SetImsi (uint64_t imsi);
  uint64_t GetImsi () const;

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

private:
  uint64_t m_imsi;
};

} // namespace ns3

#endif // NB_IOT_MSG3_IMSI_TAG_H
