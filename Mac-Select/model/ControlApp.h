#ifndef CONTROL_APP_H
#define CONTROL_APP_H
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

namespace ns3 {

class QLearn{
public:
    QLearn();
    uint32_t last_metrics;
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
    uint32_t choose_action_random();
    uint32_t argmax(uint32_t line);
    bool is_Line_Zero(uint32_t line);
    double Reward();
    void UpdateQtable();
    void get_env_feedback(uint32_t state, double reward);
    void changeState();
};
QLearn::QLearn()
{ 
    srand(time(0));
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

uint32_t QLearn::choose_action_random(){
    uint32_t action_name;
    int index = rand()%m_qtable[now_state].size();
    action_name = Action[index];
    take_action = action_name;
    next_state = action_name;
    return action_name;
}

uint32_t QLearn::choose_action()
{
    uint32_t action_name;
    //can't use srand(time(0)), it causes rand() create the same number;
    //srand(time(0));
    double random = rand()%(10000)/(float)(10000);
    if(random > epsilon || is_Line_Zero(now_state)){
        int index = rand()%m_qtable[now_state].size();
        action_name = Action[index];
    }else{
        action_name = Action[argmax(now_state)];
    }
    
    take_action = action_name;
    next_state = action_name;
    return action_name;
}

void QLearn::get_env_feedback(uint32_t state, double reward){
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
    void Setup(uint32_t apnum,uint32_t edgeOfAp,Address local,uint32_t sendPort,uint32_t receivePort,Address peer,uint32_t peerPort,Ipv4InterfaceContainer children);
private:
    std::vector<std::vector<QLearn*> >  m_qlearns;
    std::vector<std::vector<uint32_t> >  m_metrics;
    std::vector<std::vector<uint32_t> > m_states;
    std::vector<std::vector<uint32_t> > m_lastMetrics;
    std::vector<std::vector<uint32_t> > m_WeekUp;
    uint32_t m_apnum;
    uint32_t m_edgenum;
    uint32_t m_allnum;
    
    Ptr<Socket> m_sendSocket;
    Ptr<Socket> m_receiveSocket;
    Ipv4InterfaceContainer     m_children;
    Address m_local;
    Address m_peer;
    uint32_t m_sendPort;
    uint32_t m_receivePort;
    uint32_t m_peerPort;
    

    EventId  m_updataEvent;
    EventId  m_reportTimeEvent;
    uint32_t m_count;
    

    virtual void StartApplication(void);
    virtual void StopApplication(void);
    void ReportTime();
    void HandleRead (Ptr<Socket> socket);
    void SendControl(Ptr<Socket> socket,uint32_t apIndex);
    void SendControl(Ptr<Socket> socket,uint32_t apIndex,Address from);
};

}
#endif
