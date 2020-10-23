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
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "TimeHeader.h"
#include <iostream>
#include <cmath>
#include <fstream>
#include <string>
#include <cassert>
#include <vector>
using namespace ns3;


class EdgeApp : public Application{
public:
    static TypeId GetTypeId(void);
    EdgeApp();
    virtual ~EdgeApp();
    void Setup(Ipv4InterfaceContainer interfaces,uint32_t interfaceIndex ,Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate,uint32_t childnum);
    void CalculateRTT(uint64_t startTime,uint64_t endTime);
    bool GetUsingTdma();
    TracedCallback<Ptr<const Packet>> m_rxTrace;
    TracedCallback<Ptr<const Packet>,const Address&,const Address &> m_rxTraceWithAddress;
private:
    virtual void StartApplication(void);
    virtual void StopApplication(void);
    void ScheduleTx (void);
    void ScheduleRe (void);
    void SendPacket (void);
 
    void HandleRead (Ptr<Socket> socket);
    void ReportOnTime();
    void UpdateMacPro();
    Ptr<Socket>     m_socket;
    Address         m_peer;
    Address         m_local;
    EventId         m_sendEvent;
    EventId         m_reEvent;
    bool            m_running;
    uint32_t        m_packetsSent;
    uint32_t        m_packetSize;
    uint32_t        m_nPackets;
    DataRate        m_dataRate;

    uint16_t m_rti; //Report time interval;
    uint64_t m_avelatency;//Average latency;
    uint32_t m_count; // count of packets
    uint64_t m_latencyNow;
    
    bool m_usingtdma;// using tdma protocal?
    bool m_change; // is change?
    bool m_report;
    uint32_t m_metrxType; // 1:latency 2:throughput 3:latency and throughput
    uint32_t m_childnum;
    Ipv4InterfaceContainer m_interfaces;
    uint32_t m_interfaceIndex;
};





NS_OBJECT_ENSURE_REGISTERED(EdgeApp);
// Edge function

TypeId 
EdgeApp::GetTypeId(void){
    static TypeId tid = TypeId("ns3::EdgeApp")
        .SetParent<Application>()
        .SetGroupName("Applications")
        .AddConstructor<EdgeApp>()
        .AddAttribute("ReportTimeIntervalve",
                    "the intervalue of report the infomation ",
                    UintegerValue(1),
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
        .AddAttribute("MetricType",
                    " 1:latency 2:throughput 3:latency and throughput",
                    UintegerValue(1),
                    MakeUintegerAccessor(&EdgeApp::m_metrxType),
                    MakeUintegerChecker<uint32_t > ())
        .AddAttribute("Local",
                    "Local Address",
                    AddressValue(),
                    MakeAddressAccessor(&EdgeApp::m_local),
                    MakeAddressChecker())
        .AddAttribute("DateRate",
                    "the data rate in on state",
                    DataRateValue (DataRate("500kb/s")),
                    MakeDataRateAccessor(&EdgeApp::m_dataRate),
                    MakeDataRateChecker())
        .AddAttribute ("Remote", "The address of the destination",
                    AddressValue (),
                    MakeAddressAccessor (&EdgeApp::m_peer),
                    MakeAddressChecker ())  
        .AddAttribute ("PacketSize", "The size of packets sent in on state",
                    UintegerValue (512),
                    MakeUintegerAccessor (&EdgeApp::m_packetSize),
                    MakeUintegerChecker<uint32_t> (1))
        .AddAttribute ("Report", "need report to AP?",
                    BooleanValue (false),
                    MakeBooleanAccessor (&EdgeApp::m_report),
                    MakeBooleanChecker())
    ;
    return tid;
}

EdgeApp::EdgeApp()
    : m_socket(0),
      m_peer(),
      m_local(),
      m_sendEvent(),
      m_reEvent(),
      m_running(false),
      m_packetsSent(0),
      m_packetSize(0),
      m_nPackets(0),
      m_dataRate(0),
      m_rti (10),
      m_avelatency(0),
      m_count(0),
      m_latencyNow(0),
      m_usingtdma(false),
      m_change (false),
      m_report(false),
      m_metrxType(1),
      m_childnum(0),
      m_interfaces(),
      m_interfaceIndex(0)
{
    // ReportOnTime();
}
EdgeApp::~EdgeApp(){
  
}
 void
 EdgeApp::Setup (Ipv4InterfaceContainer interfaces,uint32_t interfaceIndex,Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate,uint32_t childnum)
 {
   m_interfaces = interfaces;
   m_interfaceIndex = interfaceIndex;
   m_socket = socket;
   m_peer = address;
   m_packetSize = packetSize;
   m_nPackets = nPackets;
   m_dataRate = dataRate;
   m_childnum = childnum;
 }
 
void EdgeApp::StartApplication(void){
    m_running = true;
    m_packetsSent = 0;
    if(!m_local.IsInvalid())
        m_socket->Bind(m_local);
    else
        m_socket->Bind();
    m_socket->Connect (m_peer);
    
    if(m_report)
        ReportOnTime();
    SendPacket ();
   
}
void EdgeApp::StopApplication(void){
    m_running = false;
    if(m_sendEvent.IsRunning ()){
        Simulator::Cancel(m_sendEvent);
    }
    if(m_socket){
        m_socket->Close();
    }
    m_socket->SetRecvCallback(MakeCallback (&EdgeApp::HandleRead,this));
}
void EdgeApp::SendPacket(void){
    Ptr<Packet> packet = Create<Packet> (m_packetSize);
    std::cout<<"send packet at "<<Simulator::Now().GetSeconds()<<std::endl;
    // if((uint64_t)Simulator::Now().GetSeconds() %  m_rti == 0){
    //     SeqTsSizeHeader header;
    //     header.SetSeq(m_avelatency);
    //     packet->AddHeader(header);
    // }
    // set data tag
    EdgeTag tag;
    tag.SetTagValue(0);
    packet->AddPacketTag(tag);
    // set time header now
    TimeHeader header;
    header.SetData(Simulator::Now().GetMilliSeconds());
    packet->AddHeader(header);
    m_socket->Send(packet);
    if(++m_packetsSent < m_nPackets){
        ScheduleTx ();
    }
}
void
EdgeApp::ScheduleTx(void){
    if (m_running){
        Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate())));
        m_sendEvent = Simulator::Schedule (tNext,&EdgeApp::SendPacket,this);
    }
}

void EdgeApp::ScheduleRe(void){
    Time tNext (Seconds (m_rti));
    m_reEvent = Simulator::Schedule(tNext,&EdgeApp::ReportOnTime,this);
}

void EdgeApp::ReportOnTime(){
    Ptr<Packet> packet = Create<Packet> (0);
    //add tag: metrics
    EdgeTag tag;
    tag.SetTagValue(1);
    packet->AddPacketTag(tag);
    
    SeqTsSizeHeader  nodeNum;
    nodeNum.SetSeq(m_childnum);
    //add metrics header
    SeqTsSizeHeader metrics;
    metrics.SetSeq(m_avelatency / m_count);
    //add suingtdma header
    SeqTsSizeHeader usingtdma;
    if(m_usingtdma)
        usingtdma.SetSeq(1);
    else
        usingtdma.SetSeq(0);
    packet->AddHeader(nodeNum);
    packet->AddHeader(metrics);
    packet->AddHeader(usingtdma);
    m_socket->Send(packet);
    if((uint32_t)Simulator::Now().GetSeconds() % m_rti == 0 || Simulator::Now().GetSeconds() == 0){
        // scheduleRe();
        std::cout<<"the time is "<<Simulator::Now().GetSeconds()<<" s"<<std::endl;
        ScheduleRe ();
    }
}

void EdgeApp::CalculateRTT(uint64_t startTime,uint64_t endTime){
    m_latencyNow = endTime - startTime;    
}


bool
EdgeApp::GetUsingTdma(){
    return m_usingtdma;
}

void EdgeApp::UpdateMacPro(){
    if(m_change){
        if(m_usingtdma){
            this->SetAttribute("Local",AddressValue(InetSocketAddress(m_interfaces.GetAddress(1))));
            m_socket->Bind(m_local);
            m_usingtdma = false;
        }else{
            this->SetAttribute("Local",AddressValue(InetSocketAddress(m_interfaces.GetAddress(0))));
            m_socket->Bind(m_local);
            m_usingtdma = true;
        }
    }
}




void EdgeApp::HandleRead (Ptr<Socket> socket){
    // NS_LOG_FUNCTION (this << socket);
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
                // NS_LOG_INFO("it is a data packet" + tag.GetTagValue());
                //ischange
                SeqTsSizeHeader seqTs;
                packet->RemoveHeader(seqTs);
                uint32_t firstL = seqTs.GetSeq();
                std::cout<<firstL<<std::endl;
                //reciv time
                TimeHeader header;
                packet->RemoveHeader(header);
                Time time(Simulator::Now());
                CalculateRTT(header.GetData(),time.GetMilliSeconds());
                m_count++;
                m_avelatency = m_avelatency + m_latencyNow;
            }else if(tag.GetTagValue() == 1){  // uplink metrics packet
              
            }else if (tag.GetTagValue() == 2 ){ //download control packet
                // NS_LOG_INFO("it is a control packet" + tag.GetTagValue());
                SeqTsSizeHeader action;
                packet->RemoveHeader(action);
                uint32_t isChange = action.GetSeq();
                
                if (isChange == 0 && m_usingtdma == false){
                    m_change = true;
                    UpdateMacPro();
                }
                else if (isChange == 0 && m_usingtdma == true)
                {
                    m_change = false;
                }else if(isChange == 1 && m_usingtdma == false){
                    m_change = false;
                }else if(isChange == 1 && m_usingtdma == true){
                    m_change == true;
                    UpdateMacPro();
                }
            }else if(tag.GetTagValue() == 3){   // download metrics packet
                
            }
        }
    } 
}


