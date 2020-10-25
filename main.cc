#include "EdgeApp.h"
#include "SenseApp.h"
#include "Control1App.h"
#include "ControlApp.h"
#include "ns3/simple-wireless-tdma-module.h"
#include "ns3/internet-module.h"
#include "ns3/global-route-manager-impl.h"
#include "ns3/tdma-controller.h"
#include "ns3/tdma-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("main");

void
TdmaHelper::AssignTdmaSlots (Ptr<TdmaMac> mac, uint32_t nodeId) const
{
  NS_LOG_FUNCTION (this << mac << nodeId);
  for (uint32_t i = 0; i < m_numRows; i++)
    {
      std::cout<<"debug===1 m_slotAllotmentArray[i][0]"<<m_slotAllotmentArray[i][0]<<"nodeId"<<nodeId<<std::endl;        
      if (m_slotAllotmentArray[i][0] == nodeId)
        {
        std::cout<<"debug == 2"<<std::endl;
          for (uint32_t j = 1; j < m_numCols; j++)
            {
              //validation of the slots
              NS_ASSERT_MSG (((m_slotAllotmentArray[i][j] == 0) || (m_slotAllotmentArray[i][j] == 1)),
                             "Tdma slots should be assigned with only 0 or 1");
              for (uint32_t k = 0; k < m_numRows; k++)
                {
                  if (k == i)
                    {
                      continue;
                    }
                  NS_ASSERT_MSG (!(m_slotAllotmentArray[k][j] == 1 && m_slotAllotmentArray[i][j] == 1),
                                 "Slot exclusivity is not maintained");
                }
              if (m_slotAllotmentArray[i][j] == 1)
                {
                  m_controller->AddTdmaSlot (j - 1,mac);
                }
            }
        }
    }
}

SPFVertex::NodeExit_t 
SPFVertex::GetRootExitDirection () const
{
  NS_LOG_FUNCTION (this);

  //NS_ASSERT_MSG (m_ecmpRootExits.size () <= 1, "Assumed there is at most one exit from the root to this vertex");
  return GetRootExitDirection (0);
}


int 
main(int argc, char *argv[])
{
    // LogComponentEnable("TdmaHelper");
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
    NodeContainer ControlLeftNode;
    ControlLeftNode.Create(1);

    NodeContainer routers;
    routers.Create(2);


    std::vector<NodeContainer> router1Link(2);
    std::vector<NodeContainer> router2Link(1);
    std::vector<NodeContainer> ControlLink(2); 
    std::vector<NodeContainer> ControlLeftLink(3);
    router1Link[0] = NodeContainer(wifiApNodes.Get(0),routers.Get(0));
    router1Link[1] = NodeContainer(wifiApNodes.Get(1),routers.Get(0));
    router2Link[0] = NodeContainer(wifiApNodes.Get(2),routers.Get(1));
    ControlLink[0] = NodeContainer(ControlNode.Get(0),routers.Get(0));
    ControlLink[1] = NodeContainer(ControlNode.Get(0),routers.Get(1));
    ControlLeftLink[0] = NodeContainer(ControlLeftNode.Get(0),wifiApNodes.Get(0));
    ControlLeftLink[1] = NodeContainer(ControlLeftNode.Get(0),wifiApNodes.Get(1)); 
    ControlLeftLink[2] = NodeContainer(ControlLeftNode.Get(0),wifiApNodes.Get(2));
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

    std::vector<NetDeviceContainer> router1Devices(2);
    std::vector<NetDeviceContainer> router2Devices(1);
    std::vector<NetDeviceContainer> ControlDevices(2);
    std::vector<NetDeviceContainer> ControlLeftDevices(3);
    for(uint32_t i = 0;i < 2;i++){
        router1Devices[i] = p2pHelper.Install(router1Link[i]);
    }
    router2Devices[0] = p2pHelper.Install(router2Link[0]);
    for(uint32_t i = 0;i < 2;i++){
        ControlDevices[i] = p2pHelper.Install(ControlLink[i]);
    }
    for(uint32_t i = 0;i < 3;i++){
        ControlLeftDevices[i] = p2pHelper.Install(ControlLeftLink[i]);
    }

    

    


    WifiHelper wifi;
    wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

    std::vector<NetDeviceContainer> statdmaDevices(Ap);
    std::vector<NetDeviceContainer> stacsmaDevices(Ap);
    std::vector<NetDeviceContainer> apDevices(Ap);
    std::vector<NetDeviceContainer> staDevices(Ap);
    for(uint32_t i = 0; i < Ap;i++){
        YansWifiChannelHelper channel =  YansWifiChannelHelper::Default();
        YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
        phy.SetChannel (channel.Create());
        phy.Set("ChannelNumber",UintegerValue(i));
        WifiMacHelper mac;
        Ssid ssid = Ssid("ns-3-ssid-"+i);

        mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));
        apDevices[i] = wifi.Install (phy, mac, wifiApNodes.Get(i));

        mac.SetType ("ns3::StaWifiMac",
                    "Ssid",SsidValue(ssid),
                    "ActiveProbing",BooleanValue(false));
        stacsmaDevices[i] = wifi.Install(phy,mac,wifiStaNodes[i]);
        
        std::ostringstream filename;
        filename<<"tdmaSlots"<<i<<".txt";
        TdmaHelper tdma = TdmaHelper (filename.str().c_str());
        TdmaControllerHelper controller;
        controller.Set ("SlotTime", TimeValue (MicroSeconds (1100)));
        controller.Set ("GuardTime", TimeValue (MicroSeconds (100)));
        controller.Set ("InterFrameTime", TimeValue (MicroSeconds (0)));
        tdma.SetTdmaControllerHelper (controller);
        std::cout<<wifiStaNodes[i].GetN()<<std::endl;
        statdmaDevices[i] = tdma.Install (wifiStaNodes[i]);

        staDevices[i] = NetDeviceContainer(stacsmaDevices[i],statdmaDevices[i]);
    }
    // NetDeviceContainer staallDevices = wifi.Install(phy,mac,allStaNodes);
    // NetDeviceContainer apallDevices = wifi.Install(phy,mac,allApNodes);

    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                            "MinX",DoubleValue(0.0),
                            "MinY",DoubleValue(0.0),
                            "DeltaX",DoubleValue(200.0),
                            "DeltaY",DoubleValue(200.0),
                            "GridWidth",UintegerValue(3),
                            "LayoutType",StringValue("RowFirst"));
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install(wifiApNodes);

    mobility.SetPositionAllocator("ns3::RandomDiscPositionAllocator",
                                "X",StringValue("0"),
                                "Y",StringValue("0"),
                                "Rho",StringValue("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"));
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                            "Bounds",RectangleValue(Rectangle(-100,100,-100,100)),
                            "Distance",DoubleValue(50),
                            "Mode",StringValue("Distance"));
    mobility.Install (wifiStaNodes[0]);
    mobility.SetPositionAllocator("ns3::RandomDiscPositionAllocator",
                                "X",StringValue("200"),
                                "Y",StringValue("0"),
                                "Rho",StringValue("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"));
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                            "Bounds",RectangleValue(Rectangle(100,300,-100,100)),
                            "Distance",DoubleValue(50),
                            "Mode",StringValue("Distance"));
    mobility.Install(wifiStaNodes[1]);
    mobility.SetPositionAllocator("ns3::RandomDiscPositionAllocator",
                                "X",StringValue("400"),
                                "Y",StringValue("0"),
                                "Rho",StringValue("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"));
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                            "Bounds",RectangleValue(Rectangle(300,500,-100,100)),
                            "Distance",DoubleValue(50),
                            "Mode",StringValue("Distance"));
    mobility.Install(wifiStaNodes[2]);

    InternetStackHelper stack;
    stack.Install (allStaNodes);
    stack.Install (wifiApNodes);
    stack.Install (ControlNode);
    stack.Install (ControlLeftNode);
    stack.Install (routers);

    Ipv4AddressHelper address;
    
    uint32_t i = 0,j = 0;
    std::vector<Ipv4InterfaceContainer> stacsmaInterface(Ap);
    std::vector<Ipv4InterfaceContainer> statdmaInterface(Ap);
    std::vector<std::vector<Ipv4InterfaceContainer> > staInterface(Ap);
    std::vector<Ipv4InterfaceContainer> router1Interfaces(2);
    std::vector<Ipv4InterfaceContainer> router2Interface(1);
    std::vector<Ipv4InterfaceContainer> ControlInterface(2);
    std::vector<Ipv4InterfaceContainer> ControlLeftInterface(3); 
    Ipv4InterfaceContainer ApInterfaces;
    

    for(uint32_t i = 0;i < Ap;i++){
        std::ostringstream subset;
        subset<<"10.1."<<++j<<".0";
        address.SetBase(subset.str().c_str(),"255.255.255.0");
        Ipv4InterfaceContainer ApInterface = address.Assign(apDevices[i]);
        ApInterfaces.Add(ApInterface);
        stacsmaInterface[i] = address.Assign(stacsmaDevices[i]);
        statdmaInterface[i] = address.Assign(statdmaDevices[i]);
        staInterface[i].resize(stacsmaInterface.size());
        for(uint32_t j = 0; j < stacsmaInterface.size();j++){
            staInterface[i][j].Add(stacsmaInterface[i].Get(j));
            staInterface[i][j].Add(statdmaInterface[i].Get(j));    
        }
    }
   
    for(int i =0;i < 2;i++){
        std::ostringstream subset;
        subset<<"10.1."<<++j<<".0";
        address.SetBase(subset.str().c_str(),"255.255.255.0");
        ControlInterface[i] =  address.Assign(ControlDevices[i]);
    }

    for(int i = 0 ;i < 3 ;i++){
         std::ostringstream subset;
        subset<<"10.1."<<++j<<".0";
        address.SetBase(subset.str().c_str(),"255.255.255.0");
        ControlLeftInterface[i] = address.Assign(ControlLeftDevices[i]);
    }
    for(int i =0;i < 2;i++){
        std::ostringstream subset;
        subset<<"10.1."<<++j<<".0";
        address.SetBase(subset.str().c_str(),"255.255.255.0");
        router1Interfaces[i] =  address.Assign(router1Devices[i]);
    }

    std::ostringstream subset;
    subset<<"10.1."<<++j<<".0";
    address.SetBase(subset.str().c_str(),"255.255.255.0");
    router2Interface[0] = address.Assign(router2Devices[0]);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();   

    //CreateSocket
    std::vector<Ptr<Socket> > ApSockets;
    std::vector<std::vector<Ptr<Socket>> > Stasockets;
    Stasockets.resize(Ap);
    for(i = 0; i < Ap;i++){
        Ptr<Socket> ns3TcpSocket = Socket::CreateSocket(wifiApNodes.Get(i),TcpSocketFactory::GetTypeId ());
        ApSockets.push_back(ns3TcpSocket);
    }

    for(i = 0; i < Ap;i++){
        for(uint32_t j  = 0; j < nodeofAp;j++){
            Ptr<Socket> ns3TcpSocket = Socket::CreateSocket(wifiStaNodes[i].Get(j),TcpSocketFactory::GetTypeId ());
            Stasockets[i].push_back(ns3TcpSocket);
        }
    }
    Ptr<Socket> ControlLSocket = Socket::CreateSocket(ControlLeftNode.Get(0),TcpSocketFactory::GetTypeId ());
    Ptr<Socket> ControlSocket = Socket::CreateSocket(ControlNode.Get(0),TcpSocketFactory::GetTypeId ());    
    
  


    



    //EdgeApp
    std::vector<std::vector<Ptr<EdgeApp>> > StaApps;
    StaApps.resize(Ap);
    for(i = 0; i < Ap;i++){
        for(uint32_t j = 0; j < nodeofAp; j++){
            Ptr<EdgeApp> app = CreateObject<EdgeApp> ();
            app->Setup(i,staInterface[i][j],j,Stasockets[i][j],ApInterfaces.GetAddress(i),1024,1024,DataRate("1Mbps"),j);
            app->SetAttribute("Local",AddressValue(InetSocketAddress(stacsmaInterface[i].GetAddress(j))));
            app->SetAttribute("Report",BooleanValue(true));
            wifiStaNodes[i].Get(j)->AddApplication(app);
            app->SetStartTime(Seconds (0.));
            app->SetStopTime(Seconds (20.));
            StaApps[i].push_back(app);
        }
    }
    
    //SenseApp
    std::vector<Ptr<SenseApp> > SenseApps;
    for(i = 0; i < Ap;i++){
        Ptr<SenseApp> senseapp = CreateObject<SenseApp> ();
        senseapp->Setup(ApSockets[i],stacsmaInterface[i],
                        InetSocketAddress(ApInterfaces.GetAddress(i)),
                        InetSocketAddress(ControlLeftInterface[0].GetAddress(0)),
                        InetSocketAddress(ControlInterface[0].GetAddress(0)),
                        nodeofAp,i);
        senseapp->SetAttribute("Local",AddressValue(InetSocketAddress(ApInterfaces.GetAddress(i))));
        senseapp->SetStartTime(Seconds(0.));
        senseapp->SetStopTime(Seconds(20.));
        SenseApps.push_back(senseapp);
        wifiApNodes.Get(i)->AddApplication(senseapp);
    }

    //ControlApp
    Ptr<Control1App> controlLApp = CreateObject<Control1App>();
    controlLApp->SetAttribute("Local",AddressValue(InetSocketAddress(ControlLeftInterface[0].GetAddress(0))));
    controlLApp->Setup(ControlLSocket,
                    ApInterfaces,
                    InetSocketAddress(ControlLeftInterface[0].GetAddress(0)),Ap);
    ControlLeftNode.Get(0)->AddApplication(controlLApp);
    controlLApp->SetStartTime(Seconds(0.));
    controlLApp->SetStopTime(Seconds(20.));

    Ptr<ControlApp> controlApp = CreateObject<ControlApp> ();
    controlApp->SetAttribute("Local",AddressValue(InetSocketAddress(ControlInterface[0].GetAddress(0))));
    controlApp->Setup(ControlSocket,
                    ApInterfaces,Ap,nodeofAp,Ap*nodeofAp,
                    InetSocketAddress(ControlInterface[0].GetAddress(0)));
    ControlNode.Get(0)->AddApplication(controlApp);
    controlApp->SetStartTime(Seconds(0.));
    controlApp->SetStopTime(Seconds(20.));

    std::cout<<"================Start Simulation======"<<std::endl;
    Simulator::Stop(Seconds (30.0));

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}