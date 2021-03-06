#include "EdgeTag.h"

namespace ns3 {
    NS_OBJECT_ENSURE_REGISTERED(EdgeTag);
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
}
