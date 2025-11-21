#ifndef SARA_UL_ID_TAG_H               // (1)
#define SARA_UL_ID_TAG_H               // (2)

#include "ns3/tag.h"                   // (3)
#include "ns3/uinteger.h"              // (4)
#include "ns3/type-id.h"               // (5)
#include <stdint.h>                    // (6)

namespace ns3 {                        // (7)

class SaraUlIdTag : public Tag         // (8)
{
public:                                // (9)
  SaraUlIdTag ();                      // (10)
  SaraUlIdTag (uint8_t cd, uint8_t dmrs); // (11)

  void Set (uint8_t cd, uint8_t dmrs); // (12)
  void Get (uint8_t &cd, uint8_t &dmrs) const; // (13)

  static TypeId GetTypeId (void);      // (14)
  virtual TypeId GetInstanceTypeId (void) const; // (15)

  virtual uint32_t GetSerializedSize (void) const; // (16)
  virtual void Serialize (TagBuffer i) const;      // (17)
  virtual void Deserialize (TagBuffer i);          // (18)
  virtual void Print (std::ostream &os) const;     // (19)

private:                               // (20)
  uint8_t m_cd;                        // (21)
  uint8_t m_dmrs;                      // (22)
};

} // namespace ns3                    // (23)

#endif // SARA_UL_ID_TAG_H           // (24)

