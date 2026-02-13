/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2026
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 */

#ifndef NBIOT_SCMA_MSG3_TAG_H
#define NBIOT_SCMA_MSG3_TAG_H

#include "ns3/tag.h"

namespace ns3 {

/**
 * Tag attached to Msg3 PDUs in NewSchema mode.
 * It carries logical SCMA metadata used by the simulator internals.
 */
class NbIotScmaMsg3Tag : public Tag
{
public:
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  NbIotScmaMsg3Tag ();

  void SetTcRnti (uint16_t tcRnti);
  uint16_t GetTcRnti () const;

  void SetVirtualId (uint16_t virtualId);
  uint16_t GetVirtualId () const;

  void SetCodebookId (uint8_t codebookId);
  uint8_t GetCodebookId () const;

  void SetPhysicalCarrier (uint8_t physicalCarrier);
  uint8_t GetPhysicalCarrier () const;

  virtual uint32_t GetSerializedSize () const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream& os) const;

private:
  uint16_t m_tcRnti;
  uint16_t m_virtualId;
  uint8_t m_codebookId;
  uint8_t m_physicalCarrier;
};

} // namespace ns3

#endif // NBIOT_SCMA_MSG3_TAG_H
