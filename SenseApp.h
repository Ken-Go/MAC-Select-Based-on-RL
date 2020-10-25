#ifndef SENSE_APP_H
#define SENSE_APP_H
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/traced-callback.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "EdgeTag.h"
#include "TimeHeader.h"
#include "ns3/traced-callback.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace ns3;

class SenseApp : public Application{
public:
    static TypeId GetTypeId(void);
    SenseApp();
    void Setup(Ptr<Socket> socket,Ipv4InterfaceContainer childrenAddress,Address local,Address fatherControlL,Address fatherControl,uint32_t childnum,uint32_t index);
    virtual ~SenseApp();
    void SenseEnv();
private:
    Ptr<Socket>                  m_socket;
    Ipv4InterfaceContainer       m_childs;
    Address         m_fatherControlL;
    Address         m_fatherControl;
    Address         m_local;
    EventId         m_sendEvent;
    EventId         m_updateEvent;
    EventId         m_temp;
    
    std::vector<uint64_t> m_metrics;    //the metrics of nodes which is belong to this ap node.
    std::vector<bool> m_usingtdmas;     //all the node is using tdma? 
    uint32_t m_rti;                     // report time interval
    uint32_t m_period;                  // the period of update the control info
    bool     m_report;
    uint32_t m_childNum;
    uint32_t m_info;                    
    uint64_t m_upTime;                  //millseconds
    std::vector<bool> m_updates;
    uint32_t m_tolerate;
    uint32_t m_index;
    std::vector<uint32_t> m_actions;

    virtual void StartApplication(void);
    virtual void StopApplication(void);
    void ScheduleSend(void);
    void ScheduleUpdate(void);
    void HandleRead (Ptr<Socket> socket);
    void ResetUpdate();
    void SendMetrics();
    void Update();
    bool collectAll();
    void SendToChild(Ptr<Socket> socket,uint32_t index,Address To,uint32_t Tag_Value);
};
NS_OBJECT_ENSURE_REGISTERED(SenseApp);

TypeId
SenseApp::GetTypeId(void){
    static TypeId tid = TypeId("ns3::SenseApp")
        .SetParent<Application>()
        .SetGroupName("Applicaition")
        .AddConstructor<SenseApp>()
        // .AddAttribute()
        .AddAttribute("Local",
                    "Local Address",
                    AddressValue(),
                    MakeAddressAccessor(&SenseApp::m_local),
                    MakeAddressChecker())
        // .AddAttribute ("Remote", "The address of the destination",
        //             AddressValue (),
        //             MakeAddressAccessor (&SenseApp::m_peer),
        //             MakeAddressChecker ()) 
    ;
    return tid;
}

SenseApp::SenseApp()
    : m_socket(),
      m_childs(),
      m_fatherControlL(),
      m_fatherControl(),
      m_local(),
      m_sendEvent(),
      m_updateEvent(),
      m_temp(),
      m_metrics(),
      m_usingtdmas(),
      m_rti(5),
      m_period(10),        // report period
      m_report(true),   //is report
      m_childNum(0),    //child nums 
      m_info(0),
      m_upTime(0),
      m_updates(),         //all node metrics is update?
      m_tolerate(1),
      m_index(0),
      m_actions()
{

}
SenseApp::~SenseApp(){

}
void
SenseApp::Setup(Ptr<Socket> socket,Ipv4InterfaceContainer childrenAddress,Address local,Address fatherControlL,Address fatherControl,uint32_t childnum,uint32_t index){
    m_local = local;
    m_socket = socket;
    m_childs = childrenAddress;
    m_childNum = childnum;
    m_updates.resize(childnum);
    for(uint32_t i; i < childnum ;i++){
        m_updates[i] = false;
    }
    m_fatherControl = fatherControl;
    m_fatherControlL = fatherControlL;
    m_index = index;
    m_actions.resize(m_childNum,0);
    m_usingtdmas.resize(m_childNum,false);
    m_metrics.resize(m_childNum,0);
    
};

void
SenseApp::StartApplication(void){
    if(!m_local.IsInvalid()){
       m_socket->Bind(m_local);
    }else{
       m_socket->Bind();
    }
    m_socket->SetRecvCallback(MakeCallback (&SenseApp::HandleRead,this));
    std::cout<<"start SenseApp"<<std::endl;
    // Update();
    SendMetrics();
}

void
SenseApp::StopApplication(void){

}

void
SenseApp::SendToChild(Ptr<Socket> socket,uint32_t index,Address To,uint32_t tag_value){
    Ptr<Packet> packet = Create<Packet>(0);
    EdgeTag tag;
    tag.SetTagValue(tag_value);
    packet->AddPacketTag(tag);
    SeqTsSizeHeader action;
    action.SetSeq(m_actions[index]);
    packet->AddHeader(action);
    socket->SendTo(packet,0,To);
}

void
SenseApp::HandleRead(Ptr<Socket> socket)
{
    std::cout<<"???"<<std::endl;
    Ptr<Packet> packet;
    Address from;
    Address localAddress;
    while((packet = socket->RecvFrom(from)))
    {   
        
        if(packet->GetSize() > 0)
        {
            EdgeTag tag;
            packet->PeekPacketTag(tag);
            if(tag.GetTagValue() == 0) // data packet
            {
                std::cout<<"receive tag == 0 ,data packet;"<<std::endl;
                TimeHeader header;
                packet->RemoveHeader(header);
               
                Time time(Simulator::Now());
                m_upTime = time.GetMilliSeconds()-header.GetData();

                Ptr<Packet> pac = Create<Packet> (0);
                EdgeTag tag;
                tag.SetTagValue(0);
                pac->AddPacketTag(tag);
                SeqTsSizeHeader header1;
                header1.SetSeq(m_upTime);
                TimeHeader header2;
                header2.SetData(Simulator::Now().GetMilliSeconds());
                pac->AddHeader(header1);
                pac->AddHeader(header2);
                socket->SendTo(packet,0,from);
            }else if(tag.GetTagValue() == 1) //uplink metrics packet
            {
                std::cout<<"receive tag == 1 ,up metrics packet;"<<std::endl;
                SeqTsSizeHeader nodeNum,metrics,usingtdma;
                packet->RemoveHeader(nodeNum);
                packet->RemoveHeader(metrics);
                packet->RemoveHeader(usingtdma);
                uint32_t node = nodeNum.GetSeq();
                m_metrics[node] = metrics.GetSeq();
                m_usingtdmas[node] = usingtdma.GetSeq();
                m_updates[node] = true;
            }else if(tag.GetTagValue() == 2) //download control packet
            {
                
            }else if(tag.GetTagValue() == 3) //download metrics packet
            {
                std::cout<<"receive tag == 3 ,download metrics packet;"<<std::endl;
                SeqTsSizeHeader nums;
                packet->RemoveHeader(nums);
                uint32_t m_nums= nums.GetSeq();
                std::vector<SeqTsSizeHeader> headers(m_nums);
                for(uint32_t i =0;i < m_nums;i++){
                    packet->RemoveHeader(headers[i]);
                    m_actions[i] = headers[i].GetSeq();
                    SendToChild(m_socket,i,m_childs.GetAddress(i),2);
                }            
            }
        }

    }
}

// void
// SenseApp::Update()
// {
//     Ptr<Packet> packet = Create<Packet>(0);
//     EdgeTag tag;
//     tag.SetTagValue(0);
//     m_socket->SendTo(packet,0,m_father);
// }

// void
// SenseApp::ScheduleUpdate(void)
// {
//     Time tNext (Seconds (m_period));
//     m_updateEvent = Simulator::Schedule(tNext,&SenseApp::Update,this);
// }
void
SenseApp::ScheduleSend(void)
{
    Time tNext(Seconds (m_rti));
    m_sendEvent = Simulator::Schedule (tNext,&SenseApp::SendMetrics,this);
}
bool 
SenseApp::collectAll(){
    for(uint32_t i = 0;i < m_childNum;i++){
        if(m_updates[i] == false)
            return false;
    }
    return true;
}
void 
SenseApp::ResetUpdate(){
    for(uint32_t i = 0;i < m_childNum;i++){
        m_updates[i] = false;
    }
}
void
SenseApp::SendMetrics()
{
    std::cout<<"SenseApp send Metrics"<<std::endl;
    if(collectAll()){
        std::cout<<"SenseApp Collect All"<<std::endl;
        if(m_temp.IsRunning())
            Simulator::Cancel(m_temp);
        Ptr<Packet> pac = Create<Packet>(0);
        EdgeTag tag;
        tag.SetTagValue(1);
        pac->AddPacketTag(tag);
        SeqTsSizeHeader index;
        index.SetSeq(m_index);
        pac->AddHeader(index);
        SeqTsSizeHeader nums;
        nums.SetSeq(m_childNum);
        pac->AddHeader(nums);
        std::vector<SeqTsSizeHeader> headers(m_childNum);
        std::vector<SeqTsSizeHeader> headers_usingtdma(m_childNum);
        for(uint32_t i = 0;i < m_childNum;i++){
            headers[i].SetSeq(m_metrics[i]);  
            pac->AddHeader(headers[i]);
        }
        for(uint32_t i = 0;i < m_childNum;i++){
            headers_usingtdma[i].SetSeq(m_usingtdmas[i]);
            pac->AddHeader(headers_usingtdma[i]);
        }
        m_socket->SendTo(pac,0,m_fatherControl);
        ResetUpdate();
        ScheduleSend();
    }else{
        std::cout<<"SenseApp next sendmetrics"<<std::endl;
        Time tNext(Seconds (m_tolerate));
        m_temp = Simulator::Schedule(tNext,&SenseApp::SendMetrics,this);
    }
}
#endif
