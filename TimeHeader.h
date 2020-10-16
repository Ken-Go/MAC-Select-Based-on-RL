 #include "ns3/ptr.h"
 #include "ns3/packet.h"
 #include "ns3/header.h"
 #include "ns3/timer.h"
 #include <iostream>

 using namespace ns3;
 
 
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
TimeHeader::TimeHeader ()
 {
   // we must provide a public default constructor, 
   // implicit or explicit, but never private.
 }
 TimeHeader::~TimeHeader ()
 {
 }
 TypeId
TimeHeader::GetTypeId (void)
 {
   static TypeId tid = TypeId ("ns3:TimeHeader")
     .SetParent<Header> ()
     .AddConstructor<TimeHeader> ()
   ;
   return tid;
 }
 TypeId
TimeHeader::GetInstanceTypeId (void) const
 {
   return GetTypeId ();
 }
 
 void
TimeHeader::Print (std::ostream &os) const
 {
   // This method is invoked by the packet printing
   // routines to print the content of my header.
   //os << "data=" << m_data << std::endl;
   os << "data=" << m_time;
 }
 uint32_t
TimeHeader::GetSerializedSize (void) const
 {
   // we reserve 2 bytes for our header.
   return 2;
 }
 void
TimeHeader::Serialize (Buffer::Iterator start) const
 {
   // we can serialize two bytes at the start of the buffer.
   // we write them in network byte order.
   start.WriteU64(m_time);
}
 uint32_t
TimeHeader::Deserialize (Buffer::Iterator start)
 {
   // we can deserialize two bytes from the start of the buffer.
   // we read them in network byte order and store them
   // in host byte order.
   m_time = start.ReadU64 ();
 
   // we return the number of bytes effectively read.
   return 2;
 }
 
 void 
TimeHeader::SetData (uint64_t data)
 {
   m_time = data;
 }
 uint64_t 
TimeHeader::GetData (void) const
 {
   return m_time;
 }