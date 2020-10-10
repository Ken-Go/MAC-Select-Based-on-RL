
#include "Edge.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Edge");

int 
main(int argc, char *argv[])
{
    uint32_t nodeofAp = 3;
    uint32_t Ap = 3;
    uint32_t control = 1;
    NodeContainer wifiApNodes;
    wifiApNodes.Create(Ap);
    NodeContainer allStaNodes;
    
    std::vector<NodeContainer> wifiStaNodes(Ap);
    for(int i =0 ; i< wifiStaNodes.size();i++){
        wifiStaNodes[i].Create(nodeofAp);
        for(int j =0;j<nodeofAp;j++){
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
    for(int i = 0;i < 5;i++){
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
    for(int i =0;i<Ap;i++){
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
    for(int i = 0; i < 5; i++ ){
        std::ostringstream subset;
        subset<<"10.2"<<i+1<<".0";
        address.SetBase(subset.str().c_str(),"255.255.255.0");
        address.Assign (p2pDevices[i]);
    }
    for(int i = 0 ; i < Ap;i++){
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

    uint32_t port = 50000;
    std::vector<ApplicationContainer> clientApps(Ap);
    for(int i= 0;i < Ap;i++){
        for(int j = 0; j <  staDevices.size();j++){
            AddressValue  remoteAddress(staDevices[i].Get(j)->GetAddress());
            clientHelper.SetAttribute("remote",remoteAddress);
            clientApps[i] = clientHelper.Install(wifiStaNodes[i].Get(j));
        }
    }

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    


   return 0;
}