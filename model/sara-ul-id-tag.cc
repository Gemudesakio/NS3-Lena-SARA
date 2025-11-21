#include "sara-ul-id-tag.h"            // (1)
#include "ns3/log.h"                   // (2)

namespace ns3 {                        // (3)

NS_LOG_COMPONENT_DEFINE ("SaraUlIdTag");     // (4)
NS_OBJECT_ENSURE_REGISTERED (SaraUlIdTag);  // (5)

SaraUlIdTag::SaraUlIdTag ()           // (6)
  : m_cd (0),                         // (7)
    m_dmrs (0)                        // (8)
{
}                                      // (9)

SaraUlIdTag::SaraUlIdTag (uint8_t cd, uint8_t dmrs) // (10)
  : m_cd (cd),                         // (11)
    m_dmrs (dmrs)                      // (12)
{
}                                      // (13)

TypeId                                      // (14)
SaraUlIdTag::GetTypeId (void)               // (15)
{
  static TypeId tid = TypeId ("ns3::SaraUlIdTag")  // (16)
    .SetParent<Tag> ()                             // (17)
    .AddConstructor<SaraUlIdTag> ();               // (18)
  return tid;                                      // (19)
}

TypeId                                      // (20)
SaraUlIdTag::GetInstanceTypeId (void) const // (21)
{
  return GetTypeId ();                     // (22)
}

void                                        // (23)
SaraUlIdTag::Set (uint8_t cd, uint8_t dmrs) // (24)
{
  m_cd   = cd;                              // (25)
  m_dmrs = dmrs;                            // (26)
}

void                                        // (27)
SaraUlIdTag::Get (uint8_t &cd, uint8_t &dmrs) const // (28)
{
  cd   = m_cd;                              // (29)
  dmrs = m_dmrs;                            // (30)
}

uint32_t                                    // (31)
SaraUlIdTag::GetSerializedSize (void) const // (32)
{
  return 2;                                 // (33)
}

void                                        // (34)
SaraUlIdTag::Serialize (TagBuffer i) const  // (35)
{
  i.WriteU8 (m_cd);                         // (36)
  i.WriteU8 (m_dmrs);                       // (37)
}

void                                        // (38)
SaraUlIdTag::Deserialize (TagBuffer i)      // (39)
{
  m_cd   = i.ReadU8 ();                     // (40)
  m_dmrs = i.ReadU8 ();                     // (41)
}

void                                        // (42)
SaraUlIdTag::Print (std::ostream &os) const // (43)
{
  os << "cd="   << static_cast<uint32_t> (m_cd)   // (44)
     << " dmrs=" << static_cast<uint32_t> (m_dmrs); // (45)
}

} // namespace ns3                         // (46)

