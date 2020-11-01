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
    void Setup(Ptr<Socket> socket,Address peer,Ipv4InterfaceContainer childrenAddress,Address local,Address fatherControlL,Address fatherControl,uint32_t childnum,uint32_t index);
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
    uint16_t m_port;
    Address m_peer;

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
