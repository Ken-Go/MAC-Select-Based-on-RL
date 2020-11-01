#ifndef TIME_HEADER_H
#define TIME_HEADER_H
#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/header.h"
#include "ns3/timer.h"
#include <iostream>

namespace ns3 {


class TimeHeader : public Header 
{
public:
  TimeHeader();
  virtual ~TimeHeader();

  void SetData (uint64_t time);
  uint64_t GetData (void) const;

  static TypeId GetTypeId(void);
  virtual TypeId GetInstanceTypeId(void) const;
  virtual void Print(std::ostream &os) const;
  virtual void Serialize(Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual uint32_t GetSerializedSize (void) const;
private:
  uint64_t m_time;  
};

}
#endif
