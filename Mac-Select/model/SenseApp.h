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

namespace ns3 {
class SenseApp : public Application{
public:
    static TypeId GetTypeId(void);
    SenseApp();
    void Setup(uint32_t childnum,uint32_t index,Address local,uint32_t localPort,uint32_t receivePort,Address peer,uint32_t peerPort,Ipv4InterfaceContainer childrenAddress,Address fatherControlL,Address fatherControl);
    virtual ~SenseApp();
    void SenseEnv();
private:
    Ptr<Socket>                  m_sendSocket;
    Ptr<Socket>                  m_receiveSocket;
    Ipv4InterfaceContainer       m_childs;
    
    Address         m_fatherControlL;
    Address         m_fatherControl;
    Address         m_local;
    Address         m_peer;
    uint32_t        m_localPort;
    uint32_t        m_receivePort;
    uint32_t        m_peerPort;        

    EventId         m_sendEvent;
    EventId         m_updateEvent;
    EventId         m_temp;
    
    uint32_t m_index;            //the Ap node index
    uint32_t m_childNum;         //the child number of this Ap node 
    uint32_t m_tolerate;         //tolerate to receive the data report from child(millseconds)
    uint32_t m_info;             // i have no idea about this one****       
    uint64_t m_upTime;          //compute the uplink time (millseconds)
  
    //data matrix
    std::vector<uint64_t> m_metrics;    //the metrics of nodes which is belong to this ap node.
    std::vector<bool> m_usingtdmas;     //all the node is using tdma? 
    std::vector<bool> m_updates;        //is update?
    std::vector<uint32_t> m_actions;    //remember the action 
    
    //app configure set
    uint32_t m_rti;                     // report time interval
    uint32_t m_period;                  // the period of update the control info
    bool     m_report;                  // need report?

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
}
#endif
