 #ifndef EDGE_TAG_H
 #define EDGE_TAG_H
 #include "ns3/tag.h"
 #include "ns3/packet.h"
 #include "ns3/uinteger.h"
 #include <iostream>

namespace ns3 {

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


 
}
#endif
