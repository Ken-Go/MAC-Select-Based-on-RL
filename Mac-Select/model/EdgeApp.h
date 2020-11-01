#ifndef EDGE_APP_H
#define EDGE_APP_H
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


namespace ns3 {


class EdgeApp : public Application{
public:
    static TypeId GetTypeId(void);
    EdgeApp();
    virtual ~EdgeApp();
    void Setup(Ptr<NetDevice> BindDevice,Address local,uint32_t fatherIndex,Ptr<Socket> socket, Address peeraddress, uint32_t packetSize, uint32_t nPackets, DataRate dataRate,uint32_t childnum);
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
    uint32_t m_childIndex;
    Ipv4InterfaceContainer m_interfaces;
    uint32_t m_interfaceIndex;
    uint32_t m_fatherIndex;
    uint16_t m_port;
    Ptr<NetDevice> m_BindDevice;
};

}
#endif