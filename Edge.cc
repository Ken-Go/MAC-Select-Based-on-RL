#include "Edge.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Edge");

NS_OBJECT_ENSURE_REGISTERED(EdgeApp);
// Edge function
TypeId 
EdgeApp::GetTypeId(void){
    static TypeId tid = TypeId("ns3::EdgeApp")
        .SetParent<OnOffApplication>()
        .SetGroupName("Applications")
        .AddConstructor<EdgeApp>()
        .AddAttribute("ReportTimeIntervalve",
                    "the intervalue of report the infomation ",
                    UintegerValue(32),
                    MakeUintegerAccessor(&EdgeApp::m_rti),
                    MakeUintegerChecker<uint16_t > (1,60))
        .AddAttribute("TDMA",
                    "Using TDMA?",
                    BooleanValue(false),
                    MakeBooleanAccessor(&EdgeApp::m_usingtdma),
                    MakeBooleanChecker())
        .AddTraceSource ("Rx", "A packet has been received",
                     MakeTraceSourceAccessor (&EdgeApp::m_rxTrace),
                     "ns3::Packet::TracedCallback")
        .AddTraceSource ("RxWithAddresses", "A packet has been received",
                     MakeTraceSourceAccessor (&EdgeApp::m_rxTraceWithAddress),
                     "ns3::Packet::TwoAddressTracedCallback")
    ;
    return tid;
}

EdgeApp::EdgeApp()
    : m_rti (10),
      m_avelatency(0),
      m_usingtdma(false),
      m_change (false)

{
    NS_LOG_FUNCTION(this);
}

EdgeApp::~EdgeApp(){
    NS_LOG_FUNCTION(this);
}
void EdgeApp::ReportOnTime(){

}

double EdgeApp::CalculateLatency(){

}

void EdgeApp::UpdateMacPro(){

}

void EdgeApp::HandleRead (Ptr<Socket> socket){
    NS_LOG_FUNCTION (this << socket);
    Ptr<Packet> packet;
    Address from;
    Address localAddress;
    while ((packet = socket->RecvFrom(from)))
    {
        socket->GetSockName(localAddress);
        m_rxTrace (packet);
        m_rxTraceWithAddress(packet,from,localAddress);
        if (packet->GetSize () > 0){
            SeqTsSizeHeader seqTs;
            packet->RemoveHeader(seqTs);
            uint32_t currentSequenceNumber = seqTs.GetSeq();
            uint32_t isChange = seqTs.GetSize ();
            if (isChange > 0)
                m_change = true;
            else
            {
                m_change = false;
            }
            
            if (InetSocketAddress::IsMatchingType (from)){
                NS_LOG_INFO ("TraceDelay: RX " << packet->GetSize () <<
                           " bytes from "<< InetSocketAddress::ConvertFrom (from).GetIpv4 () <<
                           " Sequence Number: " << currentSequenceNumber <<
                           " Uid: " << packet->GetUid () <<
                           " TXtime: " << seqTs.GetTs () <<
                           " RXtime: " << Simulator::Now () <<
                           " Delay: " << Simulator::Now () - seqTs.GetTs ());
            }
            else if (Inet6SocketAddress::IsMatchingType(from) > 0){
                NS_LOG_INFO ("TraceDelay: RX " << packet->GetSize () <<
                           " bytes from "<< Inet6SocketAddress::ConvertFrom (from).GetIpv6 () <<
                           " Sequence Number: " << currentSequenceNumber <<
                           " Uid: " << packet->GetUid () <<
                           " TXtime: " << seqTs.GetTs () <<
                           " RXtime: " << Simulator::Now () <<
                           " Delay: " << Simulator::Now () - seqTs.GetTs ());
            }
        }
    } 
}


int 
main(int argc, char *argv[])
{
    uint32_t nodeofAp = 3;
    uint32_t Ap = 3;
    // uint32_t control = 1;
    NodeContainer wifiApNodes;
    wifiApNodes.Create(Ap);
    NodeContainer allStaNodes;
    
    std::vector<NodeContainer> wifiStaNodes(Ap);
    for(uint32_t i =0 ; i< wifiStaNodes.size();i++){
        wifiStaNodes[i].Create(nodeofAp);
        for(uint32_t j =0;j<nodeofAp;j++){
            allStaNodes.Add(wifiStaNodes[i].Get(j));
        }
    }
    

    NodeContainer ControlNode;
    ControlNode.Create(1);
    
    NodeContainer routers;
    routers.Create(2);

    std::vector<NodeContainer> nodeLinkList(5); 
    nodeLinkList[0] = NodeContainer(wifiApNodes.Get(0),routers.Get(0));
    nodeLinkList[1] = NodeContainer(wifiApNodes.Get(1),routers.Get(0));
    nodeLinkList[2] = NodeContainer(wifiApNodes.Get(2),routers.Get(1));
    nodeLinkList[3] = NodeContainer(ControlNode.Get(0),routers.Get(0));
    nodeLinkList[4] = NodeContainer(ControlNode.Get(0),routers.Get(1));
    /*
    nodeLinkList[0].Add(wifiApNodes.Get(0));
    nodeLinkList[0].Add(routers.Get(0));
    nodeLinkList[1].Add(wifiApNodes.Get(1));
    nodeLinkList[1].Add(routers.Get(0));
    nodeLinkList[2].Add(wifiApNodes.Get(2));
    nodeLinkList[2].Add(routers.Get(1));
    nodeLinkList[3].Add(ControlNode.Get(0));
    nodeLinkList[3].Add(routers.Get(0));
    nodeLinkList[4].Add(ControlNode.Get(0));
    nodeLinkList[4].Add(routers.Get(1));
    */

    PointToPointHelper p2pHelper;
    p2pHelper.SetDeviceAttribute("DataRate",StringValue("5Mbps"));
    p2pHelper.SetChannelAttribute("Delay",StringValue("2ms"));

    std::vector<NetDeviceContainer> p2pDevices(5);
    for(uint32_t i = 0;i < 5;i++){
        p2pDevices[i] = p2pHelper.Install(nodeLinkList[i]);
    }

    YansWifiChannelHelper channel =  YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
    phy.SetChannel (channel.Create());

    WifiHelper wifi;
    wifi.SetRemoteStationManager ("ns3::AarfwifiManager");

    WifiMacHelper mac;
    Ssid ssid = Ssid("ns-3-ssid");
    mac.SetType ("ns3::StaWifiMac",
                "Ssid",SsidValue(ssid),
                "ActiveProbing",BooleanValue(false));

    std::vector<NetDeviceContainer> staDevices(Ap);
    for(uint32_t i =0;i<Ap;i++){
        staDevices[i] = wifi.Install(phy,mac,wifiStaNodes[i]);
    }
    
    
    mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

    NetDeviceContainer apDevices;
    apDevices = wifi.Install (phy, mac, wifiApNodes);

    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                            "MinX",DoubleValue(0.0),
                            "MinY",DoubleValue(0.0),
                            "DeltaX",DoubleValue(200.0),
                            "DeltaY",DoubleValue(200.0),
                            "GridWidth",UintegerValue(3),
                            "LayoutType",StringValue("RowFirst"));
    mobility.SetMobilityModel ("ns3::ConstantPositonMobilityModel");
    mobility.Install(wifiApNodes);

    mobility.SetPositionAllocator("ns3::RandomDiscPositionAllocator",
                                "x",StringValue("0"),
                                "y",StringValue("0"),
                                "Rho",StringValue("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"));
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                            "Bounds",RectangleValue(Rectangle(-50,50,-50,50)),
                            "distance",DoubleValue(50),
                            "Mode",StringValue("Distance"));
    mobility.Install (allStaNodes);


    InternetStackHelper stack;
    stack.Install (allStaNodes);
    stack.Install (wifiApNodes);
    stack.Install (ControlNode);
    stack.Install (routers);
    Ipv4AddressHelper address;
    for(uint32_t i = 0; i < 5; i++ ){
        std::ostringstream subset;
        subset<<"10.2"<<i+1<<".0";
        address.SetBase(subset.str().c_str(),"255.255.255.0");
        address.Assign (p2pDevices[i]);
    }
    for(uint32_t i = 0 ; i < Ap;i++){
        std::ostringstream subset;
        subset<<"10.1"<<i+1<<".0";
        address.SetBase(subset.str().c_str(),"255.255.255.0");
        address.Assign (apDevices.Get(i));
        address.Assign (staDevices[i]);
    }
    

    OnOffHelper clientHelper("ns3::TcpSocketFactory",Address());
    clientHelper.SetAttribute ("OnTime", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=10.0]"));  
    clientHelper.SetAttribute ("OffTime", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=10.0]"));
    clientHelper.SetAttribute ("PacketSize",StringValue("1024"));
    clientHelper.SetAttribute ("DataRate",StringValue("5MBps"));

    // uint32_t port = 50000;
    std::vector<ApplicationContainer> clientApps(Ap);
    for(uint32_t i= 0;i < Ap;i++){
        for(uint32_t j = 0; j <  staDevices.size();j++){
            AddressValue  remoteAddress(staDevices[i].Get(j)->GetAddress());
            clientHelper.SetAttribute("remote",remoteAddress);
            clientApps[i] = clientHelper.Install(wifiStaNodes[i].Get(j));
        }
    }

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    


   return 0;
}