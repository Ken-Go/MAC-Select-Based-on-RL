#ifndef CONTROL1_APP_H
#define CONTROL1_APP_H
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
#include "EdgeTag.h"
#include "TimeHeader.h"

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <vector>
using namespace ns3;

namespace ns3 {

    class Control1App : public Application
    {
    public:
        static TypeId GetTypeId(void);
        Control1App();
        virtual ~Control1App();
        void Setup(uint32_t childnum,Address local,uint32_t sendPort,uint32_t receivePort,Address peer,uint32_t peerPort,Ipv4InterfaceContainer children);
    private:
        uint32_t m_periods;
        std::vector<bool> m_reports;
        std::vector<uint32_t> m_rti;
        Ptr<Socket> m_sendSocket;
        Ptr<Socket> m_receiveSocket;
        Address m_local;
        uint32_t m_sendPort;
        uint32_t m_receivePort;
        Address m_peer;
        uint32_t m_peerPort;

        Ipv4InterfaceContainer m_children;
        uint32_t m_childnum;
        EventId m_periodBroad;
        
        
        void broadcast();
        void ScheduleBroadcast();
        virtual void StartApplication(void);
        virtual void StopApplication(void);
        void HandleRead(Ptr<Socket> socket);
        void SendControl(uint32_t addindex);
    };
}
#endif
