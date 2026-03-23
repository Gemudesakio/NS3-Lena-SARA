#include "nb-iot-msg3-imsi-tag.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NbIotMsg3ImsiTag");
NS_OBJECT_ENSURE_REGISTERED (NbIotMsg3ImsiTag);

NbIotMsg3ImsiTag::NbIotMsg3ImsiTag ()
  : m_imsi (0)
{
}

NbIotMsg3ImsiTag::NbIotMsg3ImsiTag (uint64_t imsi)
  : m_imsi (imsi)
{
}

TypeId
NbIotMsg3ImsiTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NbIotMsg3ImsiTag")
    .SetParent<Tag> ()
    .AddConstructor<NbIotMsg3ImsiTag> ();
  return tid;
}

TypeId
NbIotMsg3ImsiTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
NbIotMsg3ImsiTag::SetImsi (uint64_t imsi)
{
  m_imsi = imsi;
}

uint64_t
NbIotMsg3ImsiTag::GetImsi () const
{
  return m_imsi;
}

uint32_t
NbIotMsg3ImsiTag::GetSerializedSize (void) const
{
  return 8;
}

void
NbIotMsg3ImsiTag::Serialize (TagBuffer i) const
{
  i.WriteU64 (m_imsi);
}

void
NbIotMsg3ImsiTag::Deserialize (TagBuffer i)
{
  m_imsi = i.ReadU64 ();
}

void
NbIotMsg3ImsiTag::Print (std::ostream &os) const
{
  os << "imsi=" << m_imsi;
}

} // namespace ns3
