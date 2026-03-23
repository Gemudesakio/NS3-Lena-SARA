#include "sara-msg3-group-tag.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SaraMsg3GroupTag");
NS_OBJECT_ENSURE_REGISTERED (SaraMsg3GroupTag);

SaraMsg3GroupTag::SaraMsg3GroupTag ()
  : m_tempRnti (0),
    m_windowEnd (0),
    m_isLast (0)
{
}

SaraMsg3GroupTag::SaraMsg3GroupTag (uint16_t tempRnti, uint64_t windowEnd, bool isLast)
  : m_tempRnti (tempRnti),
    m_windowEnd (windowEnd),
    m_isLast (isLast ? 1 : 0)
{
}

TypeId
SaraMsg3GroupTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SaraMsg3GroupTag")
    .SetParent<Tag> ()
    .AddConstructor<SaraMsg3GroupTag> ();
  return tid;
}

TypeId
SaraMsg3GroupTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
SaraMsg3GroupTag::Set (uint16_t tempRnti, uint64_t windowEnd, bool isLast)
{
  m_tempRnti = tempRnti;
  m_windowEnd = windowEnd;
  m_isLast = isLast ? 1 : 0;
}

void
SaraMsg3GroupTag::Get (uint16_t &tempRnti, uint64_t &windowEnd, bool &isLast) const
{
  tempRnti = m_tempRnti;
  windowEnd = m_windowEnd;
  isLast = (m_isLast != 0);
}

uint32_t
SaraMsg3GroupTag::GetSerializedSize (void) const
{
  return 11;
}

void
SaraMsg3GroupTag::Serialize (TagBuffer i) const
{
  i.WriteU16 (m_tempRnti);
  i.WriteU64 (m_windowEnd);
  i.WriteU8 (m_isLast);
}

void
SaraMsg3GroupTag::Deserialize (TagBuffer i)
{
  m_tempRnti = i.ReadU16 ();
  m_windowEnd = i.ReadU64 ();
  m_isLast = i.ReadU8 ();
}

void
SaraMsg3GroupTag::Print (std::ostream &os) const
{
  os << "tempRnti=" << m_tempRnti
     << " windowEnd=" << m_windowEnd
     << " isLast=" << static_cast<uint32_t> (m_isLast);
}

} // namespace ns3
