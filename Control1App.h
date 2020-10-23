#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/traced-callback.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "EdgeTag.h"
#include "TimeHeader.h"

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <vector>
using namespace ns3;

class Control1App : public Application
{
public:
    static TypeId GetTypeId(void);
    Control1App();
    virtual ~Control1App();
    void Setup(Ptr<Socket> sockets,std::vector<Ipv4InterfaceContainer> children,Address local,uint32_t m_childnum);
private:
    uint32_t m_periods;
    std::vector<bool> m_reports;
    std::vector<uint32_t> m_rti;
    Ptr<Socket> m_sockets;
    Address m_local;
    std::vector<Ipv4InterfaceContainer> m_childs;
    uint32_t m_childnum;
    EventId m_periodBroad;
    void broadcast();
    void ScheduleBroadcast();
    virtual void StartApplication(void);
    virtual void StopApplication(void);
    void HandleRead(Ptr<Socket> socket);
    
    void SendControl(uint32_t addindex);
};
TypeId
Control1App::GetTypeId(void){
    static TypeId tid = TypeId("ns::Control1App")
        .SetParent<Application>()
        .SetGroupName("Application")
        .AddConstructor<Control1App>()
        .AddAttribute("Local",
                    "local address",
                    AddressValue(),
                    MakeAddressAccessor(&Control1App::m_local),
                    MakeAddressChecker())
    ;
    return tid;
}
Control1App::Control1App()
    : m_periods(),
      m_reports(),
      m_rti(),
      m_sockets(),
      m_local(),
      m_childs(),
      m_childnum(0)
{
    
}

Control1App::~Control1App(){

}
void 
Control1App::Setup(Ptr<Socket> sockets,std::vector<Ipv4InterfaceContainer> children,Address local,uint32_t childnum){
    m_sockets = sockets;
    m_childs.assign(children.begin(),children.end());
    m_local = local;
    m_childnum = childnum;
    m_periods = 20;
    m_reports.resize(m_childnum,true);
    m_rti.resize(m_childnum,5);
}

void
Control1App::StartApplication(void){
    if(!m_local.IsInvalid()){
        m_sockets->Bind(m_local);
    }else{
        m_sockets->Bind();
    }
    broadcast();
    m_sockets->SetRecvCallback(MakeCallback(&Control1App::HandleRead,this));
}
void
Control1App::StopApplication(void){}

void
Control1App::HandleRead(Ptr<Socket> socket)
{
    Ptr<Packet> packet;
    Address from;
    Address localAddress;
    while((packet = socket->RecvFrom(from))){
        EdgeTag tag;
        packet->PeekPacketTag(tag);
        if(tag.GetTagValue() == 0) //data packet
        {
            SeqTsSizeHeader index;
            packet->RemoveHeader(index);
            SendControl(index.GetSeq());
        }else if (tag.GetTagValue() == 1) //up metrics packet
        {
        }else if (tag.GetTagValue() == 2)   //download control packet
        {
        }else if (tag.GetTagValue() == 3)   //download metrics packet
        {
        }
    }
}
void
Control1App::ScheduleBroadcast(){
    Time tNext (Seconds (m_periods));
    m_periodBroad = Simulator::Schedule(tNext,&Control1App::broadcast,this);
}
void 
Control1App::broadcast()
{
    for(uint32_t i =0; i < m_childnum;i++){
        SendControl(i);
    }
    ScheduleBroadcast();
}
void 
Control1App::SendControl(uint32_t addindex)
{
    Ptr<Packet> packet = Create<Packet>(0);
    EdgeTag tag;
    tag.SetTagValue(2);
    packet->AddPacketTag(tag);

    SeqTsSizeHeader rti;
    rti.SetSeq(m_rti[addindex]);
    SeqTsSizeHeader report;
    if(m_reports[addindex] == true)
        report.SetSeq(1);
    else
    {
        report.SetSeq(0);
    }
    packet->AddHeader(rti);
    packet->AddHeader(report);
    m_sockets->SendTo(packet,0,m_childs[addindex].GetAddress(0));
}