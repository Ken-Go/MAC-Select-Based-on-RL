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
#include "ns3/traced-callback.h"
#include "packet-loss-counter.h"

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <vector>
using namespace ns3;


class SenseApp : public UdpServer{
public:
    void SenseEnv();
    void CalculateDelay();
private:
    virtual void StartApplication(void);
    virtual void StopApplication(void);
    void HandleRead (Ptr<Socket> socket);
};

class ControlApp : public UdpServer{
public:
    std::vector<std::vector<double> >  q_table;
    void Q_Learning();
private:
    virtual void StartApplication(void);
    virtual void StopApplication(void);
    void HandleRead (Ptr<Socket> socket);
};

class EdgeApp : public OnOffApplication{
public:
    static TypeId GetTypeId(void);
    EdgeApp();
    virtual ~EdgeApp();
    void ReportOnTime();
    double CalculateLatency();
    void UpdateMacPro();
    
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


//EdgeTag: "0": data packet, "1":control packet
class EdgeTag : public Tag
{
public:
    static TypeId GetTypeId (void);
    virtual TypeId GetInstanceTypeId (void) const;
    virtual uint32_t GetSerializedSize (void) const;
    virtual void Serialize (TagBuffer i) const;
    virtual void Deserialize (TagBuffer i);
    virtual void Print (std::ostream &os) const;
 
    // these are our accessors to our tag structure
    void SetTagValue (uint8_t value);
    uint8_t GetTagValue (void) const;
private:
    uint8_t m_edgeTag;
};



 //namespace ns3
