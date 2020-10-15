#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/traced-callback.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
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
    virtual ~SenseApp();
    void SenseEnv();
    void CalculateDelay();
private:
    Ptr<Socket>     m_socket;
    Address         m_peer;
    Address         m_local;
    EventId         m_sendEvent;
    
    std::vector<uint64_t> m_metrics;    //the metrics of nodes which is belong to this ap node.
    std::vector<bool> m_usingtdmas;     //all the node is using tdma? 
    uint32_t m_rti;                     // report time interval
    uint32_t m_period;                  // the period of update the control info
    bool     m_report;
    virtual void StartApplicaition(void);
    virtual void StopApplication(void);
    void HandleRead (Ptr<Socket> socket);
    void SendMetrics();
    void Update();
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
        .AddAttribute ("Remote", "The address of the destination",
                    AddressValue (),
                    MakeAddressAccessor (&SenseApp::m_peer),
                    MakeAddressChecker ()) 
    ;
    return tid;
}

SenseApp::SenseApp()
    : m_socket(0),
      m_peer(),
      m_local(),
      m_sendEvent(),
      m_metrics(),
      m_usingtdmas(),
      m_rti(2),
      m_period(10),
      m_report(true)
{

}
SenseApp::~SenseApp(){

}
void
SenseApp::Setup(){
    
}
