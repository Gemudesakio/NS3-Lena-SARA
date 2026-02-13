/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2026
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 */

#include "nb-iot-scma-msg3-tag.h"
#include "ns3/uinteger.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (NbIotScmaMsg3Tag);

TypeId
NbIotScmaMsg3Tag::GetTypeId (void)
{
  static TypeId tid =
      TypeId ("ns3::NbIotScmaMsg3Tag")
          .SetParent<Tag> ()
          .SetGroupName ("Lte")
          .AddConstructor<NbIotScmaMsg3Tag> ()
          .AddAttribute ("tcRnti",
                         "Temporary C-RNTI used in Msg3.",
                         UintegerValue (0),
                         MakeUintegerAccessor (&NbIotScmaMsg3Tag::GetTcRnti),
                         MakeUintegerChecker<uint16_t> ())
          .AddAttribute ("virtualId",
                         "Virtual SCMA carrier id assigned by scheduler.",
                         UintegerValue (0),
                         MakeUintegerAccessor (&NbIotScmaMsg3Tag::GetVirtualId),
                         MakeUintegerChecker<uint16_t> ())
          .AddAttribute ("codebookId",
                         "SCMA codebook id assigned by scheduler.",
                         UintegerValue (0),
                         MakeUintegerAccessor (&NbIotScmaMsg3Tag::GetCodebookId),
                         MakeUintegerChecker<uint8_t> ())
          .AddAttribute ("physicalCarrier",
                         "Physical UL carrier where Msg3 is transmitted.",
                         UintegerValue (0),
                         MakeUintegerAccessor (&NbIotScmaMsg3Tag::GetPhysicalCarrier),
                         MakeUintegerChecker<uint8_t> ());
  return tid;
}

TypeId
NbIotScmaMsg3Tag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

NbIotScmaMsg3Tag::NbIotScmaMsg3Tag ()
    : m_tcRnti (0),
      m_virtualId (0),
      m_codebookId (0),
      m_physicalCarrier (0)
{
}

void
NbIotScmaMsg3Tag::SetTcRnti (uint16_t tcRnti)
{
  m_tcRnti = tcRnti;
}

uint16_t
NbIotScmaMsg3Tag::GetTcRnti () const
{
  return m_tcRnti;
}

void
NbIotScmaMsg3Tag::SetVirtualId (uint16_t virtualId)
{
  m_virtualId = virtualId;
}

uint16_t
NbIotScmaMsg3Tag::GetVirtualId () const
{
  return m_virtualId;
}

void
NbIotScmaMsg3Tag::SetCodebookId (uint8_t codebookId)
{
  m_codebookId = codebookId;
}

uint8_t
NbIotScmaMsg3Tag::GetCodebookId () const
{
  return m_codebookId;
}

void
NbIotScmaMsg3Tag::SetPhysicalCarrier (uint8_t physicalCarrier)
{
  m_physicalCarrier = physicalCarrier;
}

uint8_t
NbIotScmaMsg3Tag::GetPhysicalCarrier () const
{
  return m_physicalCarrier;
}

uint32_t
NbIotScmaMsg3Tag::GetSerializedSize () const
{
  return 6;
}

void
NbIotScmaMsg3Tag::Serialize (TagBuffer i) const
{
  i.WriteU16 (m_tcRnti);
  i.WriteU16 (m_virtualId);
  i.WriteU8 (m_codebookId);
  i.WriteU8 (m_physicalCarrier);
}

void
NbIotScmaMsg3Tag::Deserialize (TagBuffer i)
{
  m_tcRnti = i.ReadU16 ();
  m_virtualId = i.ReadU16 ();
  m_codebookId = i.ReadU8 ();
  m_physicalCarrier = i.ReadU8 ();
}

void
NbIotScmaMsg3Tag::Print (std::ostream& os) const
{
  os << "tcRnti=" << m_tcRnti << " virtualId=" << m_virtualId
     << " codebookId=" << static_cast<uint32_t> (m_codebookId)
     << " physicalCarrier=" << static_cast<uint32_t> (m_physicalCarrier);
}

} // namespace ns3
