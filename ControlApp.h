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
#include "EdgeApp.h"
#include "TimeHeader.h"


#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <vector>
using namespace ns3;

class ControlApp : public Application{
public:
    static  TypeId GetTypeId(void);
    ControlApp();
    virtual ~ControlApp();
   
private:
    std::vector<std::vector<uint32_t> >  m_qtable;
    std::vector<std::vector<uint32_t> >  m_metrics;
    uint32_t m_apnum;
    uint32_t m_edgenum;
    uint32_t m_allnum;
    std::vector<Ptr<Socket>> m_sockets;
    std::vector<Address>     m_childs;
    Address m_local;
    EventId  m_updataEvent;

    virtual void StartApplication(void);
    virtual void StopApplication(void);
    void HandleRead (Ptr<Socket> socket);
    void Setup(std::vector<Ptr<Socket>> sockets,std::vector<Ptr<Socket>> childs,uint32_t m_apnum,uint32_t m_edgenum,uint32_t m_allnum,Address m_local);
    void Q_Learning(); 
};
TypeId
ControlApp::GetTypeId(void){
    static TypeId tid = TypeId("ns3::ControlApp")
        .SetParent<Application>()
        .SetGroupName("Application")
        .AddConstructor<ControlApp>()
        .AddAttribute("local",
                    "local Address",
                    AddressValue(),
                    MakeAddressAccessor(&ControlApp::m_local),
                    MakeAddressChecker())
    ;
    return tid;
}
ControlApp::ControlApp()
    : m_qtable(),
      m_metrics(),
      m_apnum(0),
      m_allnum(0), 
      m_sockets(),
      m_childs(),
      m_local(),
      m_updataEvent()
{

}
ControlApp::~ControlApp(){

}
void 
ControlApp::Setup(std::vector<Ptr<Socket>> sockets,std::vector<Ptr<Socket>> childs,uint32_t apnum,uint32_t edgenum,uint32_t allnum,Address local)
{
    m_local = local;
    m_sockets.assign(sockets.begin(),sockets.end());
    m_childs.assign(childs.begin(),childs.end());
    m_apnum = apnum;
    m_edgenum = edgenum;
    m_allnum = allnum;
}

void
ControlApp::StartApplication(void){
    for(int i = 0; i < m_sockets.size();i++){
        if(!m_local.IsInvalid()){
            m_sockets[i]->Bind(m_local);
        }else
        {
            m_sockets[i]->Bind();
        }
        m_sockets[i]->Connect(m_childs[i]);
        m_sockets[i]->SetRecvCallback(MakeCallback(&ControlApp::HandleRead,this));       
    }
}
void
ControlApp::StopApplication(void) {

}
void
ControlApp::HandleRead(Ptr<Socket> socket)
{
    Ptr<Packet> packet;
    Address from;
    Address localAddress;
    while((packet = socket->RecvFrom(from)))
    {  
        if(packet->GetSize() > 0){
            EdgeTag tag;
            packet->PeekPacketTag(tag);
            if(tag.GetTagValue() == 0){

            }else if(tag.GetTagValue() == 1){

            }else if(tag.GetTagValue() == 2){

            }else if(tag.GetTagValue() == 3){

            }
        }
        else
        {

        }


    }
}