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
#include "ns3/random-variable-stream.h"
#include "ns3/address.h"
#include "EdgeApp.h"
#include "TimeHeader.h"

#include <time.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <vector>
using namespace ns3;

class QLearn{
public:
    QLearn(); 
    uint32_t states;
    uint32_t actions;
    uint32_t now_state;
    uint32_t take_action;
    double m_reward;
    uint32_t next_state;
    std::vector<uint32_t> Action;
    double epsilon; // greedy police
    double lambda;  // discount factor
    double alpha;   //learning rate
    std::vector<std::vector<double> >  m_qtable;
    uint32_t choose_action();
    uint32_t argmax(uint32_t line);
    bool is_Line_Zero(uint32_t line);
    double Reward();
    void UpdateQtable();
    double get_env_feedback(uint32_t state, double reward);
    void changeState();
};
QLearn::QLearn()
{ 
    actions = 2;
    states = 2;
    Action.push_back(0);
    Action.push_back(1);
    m_qtable.resize(states);
    for(uint32_t i =0;i < states;i++){
        m_qtable[i].resize(actions);
    }
    for(uint32_t i = 0; i <states;i++){
        for(uint32_t j = 0; j < actions;j++){
            m_qtable[i][j] = 0.0;
        }
    }
    epsilon = 0.9;
    alpha = 0.1;
    lambda = 0.9;
}

bool QLearn::is_Line_Zero(uint32_t line){
    uint32_t size = m_qtable[line].size();
    for(uint32_t i; i < size;i++){
        if(m_qtable[line][i] != 0)
            return false;
    }
    return true;
}

uint32_t QLearn::argmax(uint32_t line){
    uint32_t index = 0;
    for(uint32_t i = 1; i < m_qtable[line].size();i++){
        if(m_qtable[line][i] > m_qtable[line][index] ){
            index = i;
        }
    }
    return index;
}

uint32_t QLearn::choose_action()
{

    uint32_t action_name;
    srand(time(0));
    double random = rand()%(10000)/(float)(10000);
    if(random > epsilon || is_Line_Zero(now_state)){
        int index = rand()%m_qtable[now_state].size();
        action_name = Action[index];
    }else{
        action_name = Action[argmax(now_state)];
    }
    take_action = action_name;
    next_state = action_name;
}

double QLearn::get_env_feedback(uint32_t state, double reward){
    m_reward = reward;
    now_state = state;
}
void
QLearn::UpdateQtable(){
    double q_pre = m_qtable[now_state][take_action];
    double q_target = m_reward + lambda * m_qtable[next_state][argmax(next_state)];
    m_qtable[now_state][take_action] += alpha * (q_target - q_pre);
}
void 
QLearn::changeState(){
    now_state = next_state;
}



class ControlApp : public Application{
public:
    static  TypeId GetTypeId(void);
    ControlApp();
    virtual ~ControlApp();
   
private:
    std::vector<std::vector<QLearn*> >  m_qlearns;
    std::vector<std::vector<uint32_t> >  m_metrics;
    std::vector<std::vector<uint32_t> > m_states;
    uint32_t m_apnum;
    uint32_t m_edgenum;
    uint32_t m_allnum;
    
    std::vector<Ptr<Socket>> m_sockets;
    std::vector<Address>     m_childs;
    Address m_local;
    EventId  m_updataEvent;
    double e;

    virtual void StartApplication(void);
    virtual void StopApplication(void);
    void HandleRead (Ptr<Socket> socket);
    void SendControl(Ptr<Socket> socket,uint32_t apIndex,std::vector<uint32_t> state);
    void SendControl(Ptr<Socket> socket,uint32_t apIndex,std::vector<uint32_t> state,Address from);
    void Setup(std::vector<Ptr<Socket>> sockets,std::vector<Ptr<Socket>> childs,uint32_t m_apnum,uint32_t m_edgenum,uint32_t m_allnum,Address m_local);

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
    : m_qlearns(),
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
    m_qlearns.resize(m_apnum);
    for(uint32_t i =0;i<m_apnum;i++){
        m_qlearns[i].resize(edgenum);
    }
    for(uint32_t i = 0; i < m_apnum;i++){
        for(uint32_t j = 0; j < m_edgenum;j++){
            m_states[i][j] = 0;
            QLearn* q = new QLearn();
            m_qlearns[i][j] = q;
        }
    }
}
void
ControlApp::SendControl(Ptr<Socket> socket,uint32_t apIndex,std::vector<uint32_t> state){
    Ptr<Packet> packet = Create<Packet>(0);
    EdgeTag tag;
    tag.SetTagValue(3);
    packet->AddPacketTag(tag);
    SeqTsHeader nums;
    nums.SetSeq(m_edgenum);
    packet->AddHeader(nums);
    std::vector<SeqTsSizeHeader> headers(m_edgenum);
    for(uint32_t i; i < m_edgenum;i++){
        m_qlearns[apIndex][i]->get_env_feedback(state[i],0);
        headers[i].SetSeq(m_qlearns[apIndex][i]->choose_action());
        packet->AddHeader(headers[i]);
    }
    socket->Send(packet,0);
}
void
ControlApp::SendControl(Ptr<Socket> socket,uint32_t apIndex,std::vector<uint32_t> state,Address from){
    Ptr<Packet> packet = Create<Packet>(0);
    EdgeTag tag;
    tag.SetTagValue(3);
    packet->AddPacketTag(tag);
    SeqTsHeader nums;
    nums.SetSeq(m_edgenum);
    packet->AddHeader(nums);
    std::vector<SeqTsSizeHeader> headers(m_edgenum);
    for(uint32_t i; i < m_edgenum;i++){
        m_qlearns[apIndex][i]->get_env_feedback(state[i],0);
        headers[i].SetSeq(m_qlearns[apIndex][i]->choose_action());
        packet->AddHeader(headers[i]);
    }
    socket->SendTo(packet,0,from);
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
    for(uint32_t i = 0; i < m_sockets.size();i++){
        SendControl(m_sockets[i],i,m_states[i]);
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
            if(tag.GetTagValue() == 0){     //data  packet
                
            }else if(tag.GetTagValue() == 1){   //uplink metrics packet
                SeqTsSizeHeader index;
                packet->RemoveHeader(index);
                uint32_t mindex  = index.GetSeq(); 
                SeqTsSizeHeader edgenum;
                packet->RemoveHeader(edgenum);
                m_edgenum = edgenum.GetSeq();
                std::vector<SeqTsSizeHeader> headers(m_edgenum),headers_usingtdma(m_edgenum);
                for(uint32_t i = 0; i < m_edgenum;i++){
                    packet->RemoveHeader(headers[i]);
                    m_metrics[mindex][i] = headers[i].GetSeq();        
                }
                for(uint32_t i = 0; i < m_edgenum;i++){
                    packet->RemoveHeader(headers_usingtdma[i]);
                    m_states[mindex][i] = headers_usingtdma[i].GetSeq();        
                }
                for(uint32_t i =0; i < m_edgenum;i++){
                    m_qlearns[mindex][i]->get_env_feedback(m_states[mindex][i],m_metrics[mindex][i]);
                    m_qlearns[mindex][i]->UpdateQtable();
                    m_qlearns[mindex][i]->changeState();
                    SendControl(socket,mindex,m_states[mindex],from);
                }
            }else if(tag.GetTagValue() == 2){   //download control packet

            }else if(tag.GetTagValue() == 3){   //download metrics packet

            }
        }
    }
}
