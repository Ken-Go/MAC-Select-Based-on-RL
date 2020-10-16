 #include "ns3/tag.h"
 #include "ns3/packet.h"
 #include "ns3/uinteger.h"
 #include <iostream>

 using namespace ns3;

//EdgeTag: "0": data packet, "1": uplink matrics packet,"2":control packet,"3":download matrics packet
class EdgeTag : public Tag
{
public:
    static TypeId GetTypeId (void);
    virtual TypeId GetInstanceTypeId (void) const;
    virtual uint32_t GetSerializedSize (void) const;
    virtual void Serialize (TagBuffer i) const;
    virtual void Deserialize (TagBuffer i);
    virtual void Print (std::ostream &os) const;
 
    // these are our accessors to our tag structure
    void SetTagValue (uint8_t value);
    uint8_t GetTagValue (void) const;
private:
    uint8_t m_edgeTag;
};
TypeId
EdgeTag::GetTypeId(void)
{
    static TypeId tid = TypeId ("ns3::EdgeTag")
                        .SetParent<Tag>()
                        .AddConstructor<EdgeTag>()
                        .AddAttribute("EdgeTagValue",
                                    "EdgeTag: '0': data packet, '1':control packet",
                                    EmptyAttributeValue(),
                                    MakeUintegerAccessor(&EdgeTag::GetTagValue),
                                    MakeUintegerChecker<uint8_t>())
    ;
    return tid;
}
TypeId
EdgeTag::GetInstanceTypeId(void) const {
    return GetTypeId();
}
uint32_t
EdgeTag::GetSerializedSize(void) const {
    return 1;
}

void 
EdgeTag::Serialize(TagBuffer i) const 
{
    i.WriteU8(m_edgeTag);
}
void 
EdgeTag::Deserialize(TagBuffer i){
    m_edgeTag = i.ReadU8();
}
 void 
 EdgeTag::Print (std::ostream &os) const
 {
   os << "v=" << (uint32_t)m_edgeTag;
 }
 void 
 EdgeTag::SetTagValue (uint8_t value)
 {
   m_edgeTag = value;
 }
 uint8_t 
 EdgeTag::GetTagValue (void) const
 {
   return m_edgeTag;
 }