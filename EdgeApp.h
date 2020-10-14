#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/traced-callback.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/node.h"
#include "ns3/address.h"
#include "ns3/simple-wireless-tdma-module.h"
#include "ns3/traced-callback.h"
#include "EdgeTag.h"

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <vector>
using namespace ns3;

NS_LOG_COMPONENT_DEFINE("EdgeApp");
class EdgeApp : public OnOffApplication{
public:
    static TypeId GetTypeId(void);
    EdgeApp();
    virtual ~EdgeApp();
    void ReportOnTime();
    double CalculateLatency();
    void UpdateMacPro(NodeContainer nodes,NetDeviceContainer devices,int index,double txpDistance);
    
    u_int16_t m_rti; //Report time interval;
    u_int32_t m_avelatency;//Average latency;
    bool m_usingtdma;// using tdma protocal?
    bool m_change; // is change?
    TracedCallback<Ptr<const Packet>> m_rxTrace;

    TracedCallback<Ptr<const Packet>,const Address&,const Address &> m_rxTraceWithAddress;
private:
    // virtual void StartApplication(void);
    // virtual void StopApplication(void);
    void HandleRead (Ptr<Socket> socket);
};





NS_OBJECT_ENSURE_REGISTERED(EdgeApp);
// Edge function

TypeId 
EdgeApp::GetTypeId(void){
    static TypeId tid = TypeId("ns3::EdgeApp")
        .SetParent<OnOffApplication>()
        .SetGroupName("Applications")
        .AddConstructor<EdgeApp>()
        .AddAttribute("ReportTimeIntervalve",
                    "the intervalue of report the infomation ",
                    UintegerValue(32),
                    MakeUintegerAccessor(&EdgeApp::m_rti),
                    MakeUintegerChecker<uint16_t > (1,60))
        .AddAttribute("TDMA",
                    "Using TDMA?",
                    BooleanValue(false),
                    MakeBooleanAccessor(&EdgeApp::m_usingtdma),
                    MakeBooleanChecker())
        .AddTraceSource ("Rx", "A packet has been received",
                     MakeTraceSourceAccessor (&EdgeApp::m_rxTrace),
                     "ns3::Packet::TracedCallback")
        .AddTraceSource ("RxWithAddresses", "A packet has been received",
                     MakeTraceSourceAccessor (&EdgeApp::m_rxTraceWithAddress),
                     "ns3::Packet::TwoAddressTracedCallback")
    ;
    return tid;
}

EdgeApp::EdgeApp()
    : m_rti (10),
      m_avelatency(0),
      m_usingtdma(false),
      m_change (false)

{
    NS_LOG_FUNCTION(this);
}

EdgeApp::~EdgeApp(){
    NS_LOG_FUNCTION(this);
}
void EdgeApp::ReportOnTime(){

}

double EdgeApp::CalculateLatency(){

}
void 
Node::DoDispose(){
    NS_LOG_FUNCTION (this);
    m_deviceAdditionListeners.clear ();
    m_handlers.clear ();
    for (std::vector<Ptr<NetDevice> >::iterator i = m_devices.begin ();i != m_devices.end (); i++)
    {
       Ptr<NetDevice> device = *i;
       device->Dispose ();
       *i = 0;
    }
    m_devices.clear ();
}
void EdgeApp::UpdateMacPro(OnOffHelper helper,int index){
    if(m_change){
        
        if(m_usingtdma){
            
        }else{
            
            m_usingtdma = false;
        }
    }
}



void EdgeApp::HandleRead (Ptr<Socket> socket){
    NS_LOG_FUNCTION (this << socket);
    Ptr<Packet> packet;
    Address from;
    Address localAddress;
    while ((packet = socket->RecvFrom(from)))
    {
        socket->GetSockName(localAddress);
        m_rxTrace (packet);
        m_rxTraceWithAddress(packet,from,localAddress);
        if (packet->GetSize () > 0){
            EdgeTag tag;
            packet->PeekPacketTag(tag);
            if (tag.GetTagValue() == 0) {
                NS_LOG_INFO("it is a data packet" + tag.GetTagValue());
            }else{
                NS_LOG_INFO("it is a control packet" + tag.GetTagValue());
                SeqTsSizeHeader seqTs;
                packet->RemoveHeader(seqTs);
                uint32_t currentSequenceNumber = seqTs.GetSeq();
                uint32_t isChange = seqTs.GetSize ();
                if (isChange > 0)
                    m_change = true;
                else
                {
                    m_change = false;
                }
            }
        }
    } 
}


