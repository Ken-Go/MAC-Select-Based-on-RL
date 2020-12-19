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
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv4-interface.h"
#include "ns3/udp-l4-protocol.h"
#include "EdgeTag.h"
#include "TimeHeader.h"
#include "PacketInfo.h"
#include <iostream>
#include <cmath>
#include <fstream>
#include <string>
#include <sstream>
#include <cassert>
#include <vector>


namespace ns3 {

class EdgeApp : public Application{
public:
    static TypeId GetTypeId(void);
    EdgeApp();
    virtual ~EdgeApp();
    void Setup(uint32_t fatherIndex,uint32_t index,Ipv4InterfaceContainer locals, uint32_t localPort,uint32_t receivePort,Ipv4InterfaceContainer peers,uint32_t peerPort, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);
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
    void SendStart(void);
    void SendEnd(void);
 
    void HandleRead (Ptr<Socket> socket);
    void ReportOnTime();
    void UpdateMacPro();
    Ptr<Socket>     m_sendSocket;
    Ptr<Socket>     m_csmaSocket;
    Ptr<Socket>     m_tdmaSocket;
    Ptr<Socket>     m_receiveSocket;
    Address         m_peer;
    Address         m_local;
    Ipv4InterfaceContainer m_locals;
    Ipv4InterfaceContainer m_peers;
    uint32_t        m_localPort;
    uint32_t        m_receivePort;
    uint32_t        m_peerPort;

    EventId         m_sendEvent;
    EventId         m_reEvent;
   
    bool            m_running;
    uint32_t        m_packetsSent;
    uint32_t        m_packetSize;
    uint32_t        m_nPackets;
    DataRate        m_dataRate;

    uint16_t m_rti;                 //Report time interval;
    uint64_t m_avelatency;          //Average latency;
    uint32_t m_count;               // count of packets
    uint64_t m_latencyNow;
    
    bool m_usingtdma;               // using tdma protocal?
    bool m_change;                  // is change?
    bool m_report;                  //repoet?
    bool m_sendStart;               //send Start;
    uint32_t m_metrxType;           // 1:latency 2:throughput 3:latency and throughput
    uint32_t m_childIndex;          //edge index
    uint32_t m_fatherIndex;         //Ap index
    uint16_t m_port;                // port to listen packet
    std::vector<uint32_t> m_rtt;    // remember m_rtt time
    std::ofstream m_outStream;
    uint32_t m_packetSeq;
    std::vector<PacketInfo>  m_PacketInfos;

};
}
#endif