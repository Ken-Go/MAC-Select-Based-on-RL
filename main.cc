
#include "ns3/EdgeApp.h"
#include "ns3/SenseApp.h"
#include "ns3/Control1App.h"
#include "ns3/ControlApp.h"
#include "ns3/simple-wireless-tdma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/yans-wifi-phy.h"
// #include "ns3/RxPowerWattPerChannelBand"
#include "ns3/global-route-manager-impl.h"
#include "ns3/dsdv-helper.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE("main");




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
    LogComponentEnable("main",LOG_LEVEL_INFO);
    LogComponentEnable("EdgeApp",LOG_LEVEL_INFO);
    LogComponentEnable("SenseApp",LOG_LEVEL_INFO);
    LogComponentEnable("ControlApp",LOG_LEVEL_INFO);
    LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
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

    // NodeContainer routers;
    // routers.Create(2);


    // std::vector<NodeContainer> router1Link(2);
    // std::vector<NodeContainer> router2Link(1);
    // std::vector<NodeContainer> ControlLink(2); 
    std::vector<NodeContainer> ControlLeftLink(3);
    std::vector<NodeContainer> ControlLink(3);
    // router1Link[0] = NodeContainer(wifiApNodes.Get(0),routers.Get(0));
    // router1Link[1] = NodeContainer(wifiApNodes.Get(1),routers.Get(0));
    // router2Link[0] = NodeContainer(wifiApNodes.Get(2),routers.Get(1));
    // ControlLink[0] = NodeContainer(ControlNode.Get(0),routers.Get(0));
    // ControlLink[1] = NodeContainer(ControlNode.Get(0),routers.Get(1));
    ControlLink[0] = NodeContainer(ControlNode.Get(0),wifiApNodes.Get(0));
    ControlLink[1] = NodeContainer(ControlNode.Get(0),wifiApNodes.Get(1));
    ControlLink[2] = NodeContainer(ControlNode.Get(0),wifiApNodes.Get(2));
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

   



    std::vector<NetDeviceContainer> statdmaDevices(Ap);
    std::vector<NetDeviceContainer> stacsmaDevices(Ap);
    std::vector<NetDeviceContainer> apDevices(Ap);
    std::vector<NetDeviceContainer> staDevices(Ap);
    std::vector<NetDeviceContainer> tdmaApDevices(Ap);
    for(uint32_t i = 0; i < Ap;i++){
        WifiHelper wifi;
        wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
        YansWifiChannelHelper channel =  YansWifiChannelHelper::Default();
        YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
        phy.SetChannel (channel.Create());
        // phy.Set("ChannelNumber",UintegerValue(i));

        WifiMacHelper mac;
        Ssid ssid = Ssid("ns-3-ssid-"+i);
        mac.SetType("ns3::AdhocWifiMac");

        apDevices[i] = wifi.Install (phy, mac, wifiApNodes.Get(i));
        stacsmaDevices[i] = wifi.Install(phy,mac,wifiStaNodes[i]);

        std::ostringstream filename;
        filename<<"tdmaSlots"<<i<<".txt";
        TdmaHelper tdma = TdmaHelper (filename.str().c_str());
        TdmaControllerHelper controller;
        controller.Set ("SlotTime", TimeValue (MicroSeconds (1100)));
        controller.Set ("GuardTime", TimeValue (MicroSeconds (100)));
        controller.Set ("InterFrameTime", TimeValue (MicroSeconds (0)));
        tdma.SetTdmaControllerHelper (controller);
         
        statdmaDevices[i] = tdma.Install (wifiStaNodes[i]);
        tdmaApDevices[i] = tdma.Install(wifiApNodes.Get(i));    
    }


    PointToPointHelper p2pHelper;
    p2pHelper.SetDeviceAttribute("DataRate",StringValue("15Mbps"));
    p2pHelper.SetChannelAttribute("Delay",StringValue("2ms"));
    // std::vector<NetDeviceContainer> router1Devices(2);
    // std::vector<NetDeviceContainer> router2Devices(1);
    // std::vector<NetDeviceContainer> ControlDevices(2);
    std::vector<NetDeviceContainer> ControlDevices(3);
    std::vector<NetDeviceContainer> ControlLeftDevices(3);
    // for(uint32_t i = 0;i < 2;i++){
    //     router1Devices[i] = p2pHelper.Install(router1Link[i]);
    // }
    // router2Devices[0] = p2pHelper.Install(router2Link[0]);
    for(uint32_t i = 0;i < 3;i++){
        ControlDevices[i] = p2pHelper.Install(ControlLink[i]);
    }
    for(uint32_t i = 0;i < 3;i++){
        ControlLeftDevices[i] = p2pHelper.Install(ControlLeftLink[i]);
    }

    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                            "MinX",DoubleValue(0.0),
                            "MinY",DoubleValue(0.0),
                            "DeltaX",DoubleValue(200.0),
                            "DeltaY",DoubleValue(0.0),
                            "GridWidth",UintegerValue(3),
                            "LayoutType",StringValue("RowFirst"));
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install(wifiApNodes);

    mobility.SetPositionAllocator("ns3::RandomDiscPositionAllocator",
                                "X",StringValue("0"),
                                "Y",StringValue("0"),
                                "Rho",StringValue("ns3::UniformRandomVariable[Min=0.0|Max=25.0]"));
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                            "Bounds",RectangleValue(Rectangle(-50,50,-50,50)),
                            "Distance",DoubleValue(5),
                            "Mode",StringValue("Distance"));
    mobility.Install(wifiStaNodes[0]);
    mobility.SetPositionAllocator("ns3::RandomDiscPositionAllocator",
                                "X",StringValue("200"),
                                "Y",StringValue("0"),
                                "Rho",StringValue("ns3::UniformRandomVariable[Min=0.0|Max=25.0]"));
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                            "Bounds",RectangleValue(Rectangle(150,250,-50,50)),
                            "Distance",DoubleValue(5),
                            "Mode",StringValue("Distance"));
    mobility.Install(wifiStaNodes[1]);
    mobility.SetPositionAllocator("ns3::RandomDiscPositionAllocator",
                                "X",StringValue("400"),
                                "Y",StringValue("0"),
                                "Rho",StringValue("ns3::UniformRandomVariable[Min=0.0|Max=25.0]"));
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                            "Bounds",RectangleValue(Rectangle(350,450,-50,50)),
                            "Distance",DoubleValue(5),
                            "Mode",StringValue("Distance"));
    mobility.Install(wifiStaNodes[2]);


    // DsdvHelper dsdv;
    // dsdv.Set ("PeriodicUpdateInterval",TimeValue (Seconds(0.5)));
    // dsdv.Set ("SettlingTime",TimeValue(Seconds (6)));
    InternetStackHelper stack;
    // stack.SetRoutingHelper(dsdv);
    
    stack.Install (allStaNodes);
    stack.Install (wifiApNodes);
    stack.Install (ControlNode);
    stack.Install (ControlLeftNode);
    // stack.Install (routers);

    Ipv4AddressHelper address;
    
    uint32_t i = 0;
    uint32_t j = 0;
    std::vector<Ipv4InterfaceContainer> stacsmaInterface(Ap);
    std::vector<Ipv4InterfaceContainer> statdmaInterface(Ap);
    
    // std::vector<Ipv4InterfaceContainer> router1Interfaces(2);
    // std::vector<Ipv4InterfaceContainer> router2Interface(1);
    // std::vector<Ipv4InterfaceContainer> ControlInterface(2);
    std::vector<Ipv4InterfaceContainer> ControlLeftInterface(3); 
    std::vector<Ipv4InterfaceContainer> ControlInterface(3); 
    Ipv4InterfaceContainer csma_ApInterfaces,tdma_ApInterfaces;
    
    //csmaInterface + tdmaInterface
    std::vector<std::vector<Ipv4InterfaceContainer> > nodeInterfaces(Ap);
    std::vector<Ipv4InterfaceContainer> ApInterfaces(Ap);

    //control children interfaces
    Ipv4InterfaceContainer controlChildrenInterfaces;

    //controlleft  children interfaces
    Ipv4InterfaceContainer controlLeftChildrenInterfaces;
    for(uint32_t i = 0;i < Ap;i++){
        
        std::ostringstream csma_base;
        csma_base<<"10.1."<<++j<<".0";
        address.SetBase(csma_base.str().c_str(),"255.255.255.0");
        Ipv4InterfaceContainer csma_ApInterface = address.Assign(apDevices[i]);
        csma_ApInterfaces.Add(csma_ApInterface);
        stacsmaInterface[i] = address.Assign(stacsmaDevices[i]);
        
        
        
        std::ostringstream tdma_base;
        tdma_base<<"10.1."<<++j<<".0";
        address.SetBase(tdma_base.str().c_str(),"255.255.255.0");
        Ipv4InterfaceContainer tdma_ApInterface = address.Assign(tdmaApDevices[i]);
        tdma_ApInterfaces.Add(tdma_ApInterface);
        statdmaInterface[i] = address.Assign(statdmaDevices[i]);     
        

        ApInterfaces[i].Add(csma_ApInterfaces.Get(i));
        ApInterfaces[i].Add(tdma_ApInterfaces.Get(i));
        
        nodeInterfaces[i].resize(nodeofAp);
        for(uint32_t k = 0; k < nodeInterfaces.size();k++){
            nodeInterfaces[i][k].Add(stacsmaInterface[i].Get(k));
            nodeInterfaces[i][k].Add(statdmaInterface[i].Get(k));
        }
    }

    for(int i =0;i < 3;i++){
        std::ostringstream subset;
        subset<<"10.1."<<++j<<".0";
        address.SetBase(subset.str().c_str(),"255.255.255.0");
        ControlInterface[i] =  address.Assign(ControlDevices[i]);
        controlChildrenInterfaces.Add(ControlInterface[i].Get(1));
    }


    for(int i = 0 ;i < 3 ;i++){
        std::ostringstream subset;
        subset<<"10.1."<<++j<<".0";
        address.SetBase(subset.str().c_str(),"255.255.255.0");
        ControlLeftInterface[i] = address.Assign(ControlLeftDevices[i]);
        controlLeftChildrenInterfaces.Add(ControlLeftInterface[i].Get(1));
    }

    // for(int i =0;i < 2;i++){
    //     std::ostringstream subset;
    //     subset<<"10.1."<<++j<<".0";
    //     address.SetBase(subset.str().c_str(),"255.255.255.0");
    //     router1Interfaces[i] =  address.Assign(router1Devices[i]);
    // }
    // std::ostringstream subset;
    // subset<<"10.1."<<++j<<".0";
    // address.SetBase(subset.str().c_str(),"255.255.255.0");
    // router2Interface[0] = address.Assign(router2Devices[0]);

    //build route table
    //print all ip address
    // for(uint32_t i = 0;i < Ap; i++){
    //     Ptr<Ipv4> ipv4 = wifiApNodes.Get(i)->GetObject<Ipv4> ();
    //     NS_LOG_INFO("AP Node " << i << " has " << ipv4->GetNInterfaces() << " interfaces: \n");
    //     for(uint32_t j = 0; j < ipv4->GetNInterfaces();++j){
    //         NS_LOG_INFO("\tInterface " << j << " has " << ipv4->GetNAddresses(j) << " addresses:\n" );
    //         for(uint32_t k = 0; k < ipv4->GetNAddresses(j);k++){
    //             NS_LOG_INFO("\t\t"<<ipv4->GetAddress(j,k).GetLocal() <<" \n");
    //         }
    //     }
    //     for(uint32_t inode = 0;inode < nodeofAp; inode++){
    //         Ptr<Ipv4> ipv4 = wifiStaNodes[i].Get(inode)->GetObject<Ipv4> ();
    //         NS_LOG_INFO("AP Node " << i << " Edge Node "<<inode<<" has " << ipv4->GetNInterfaces() << " interfaces: \n");
    //         for(uint32_t j = 0; j < ipv4->GetNInterfaces();++j){
    //             NS_LOG_INFO("\tInterface " << j << " has " << ipv4->GetNAddresses(j) << " addresses:\n" );
    //             for(uint32_t k = 0; k < ipv4->GetNAddresses(j);k++){
    //                 NS_LOG_INFO("\t\t"<<ipv4->GetAddress(j,k).GetLocal() <<" \n");
    //             }
    //         }
    //     }
    // }
    Ptr<Ipv4> control_ipv4 = ControlNode.Get(0)->GetObject<Ipv4> ();
    Ptr<Ipv4> controlL_ipv4 = ControlLeftNode.Get(0)->GetObject<Ipv4> ();
    // NS_LOG_INFO("Control Node has " << control_ipv4->GetNInterfaces() << " interfaces: \n");
    // for(uint32_t j = 0; j < control_ipv4->GetNInterfaces();++j){
    //     NS_LOG_INFO("\tInterface " << j << " has " << control_ipv4->GetNAddresses(j) << " addresses:\n" );
    //     for(uint32_t k = 0; k < control_ipv4->GetNAddresses(j);k++){
    //         NS_LOG_INFO("\t\t"<<control_ipv4->GetAddress(j,k).GetLocal() <<" \n");
    //     }
    // }
    
    // NS_LOG_INFO("Control Left Node has " << controlL_ipv4->GetNInterfaces() << " interfaces: \n");
    // for(uint32_t j = 0; j < controlL_ipv4->GetNInterfaces();++j){
    //     NS_LOG_INFO("\tInterface " << j << " has " << controlL_ipv4->GetNAddresses(j) << " addresses:\n" );
    //     for(uint32_t k = 0; k < controlL_ipv4->GetNAddresses(j);k++){
    //         NS_LOG_INFO("\t\t"<<controlL_ipv4->GetAddress(j,k).GetLocal() <<" \n");
    //     }
    // }

    Ipv4StaticRoutingHelper staticroutinghelper;
    //Ap node build static route table
    for(uint32_t i = 0; i < Ap;i++){
        Ptr<Ipv4> ipv4 = wifiApNodes.Get(i)->GetObject<Ipv4> ();
        Ptr<Ipv4StaticRouting> staticRouting = staticroutinghelper.GetStaticRouting(ipv4);

        for(uint32_t j = 0; j < nodeofAp ; j++){
            std::ostringstream csma_address;
            csma_address<<"10.1."<<2*i+1<<"."<<(j+2);
            // std::cout<<"csma address " << csma_address.str().c_str() << std::endl;    
            staticRouting->AddHostRouteTo(Ipv4Address(csma_address.str().c_str()),1);
            
            std::ostringstream tdma_address;
            tdma_address<<"10.1."<<2*i+2<<"."<<(j+2);
            // std::cout<<"tdma address " << tdma_address.str().c_str() << std::endl;    
            staticRouting->AddHostRouteTo(Ipv4Address(tdma_address.str().c_str()),2);
        }

        std::ostringstream  toFatherL,toFatherR;
        toFatherR<<"10.1."<<7+i<<".1";
        toFatherL<<"10.1."<<10+i<<".1";
        
        staticRouting->AddHostRouteTo(Ipv4Address(toFatherR.str().c_str()),3);
        staticRouting->AddHostRouteTo(Ipv4Address(toFatherL.str().c_str()),4);
    }
    //Edge Node build static route table
    for(uint32_t i = 0;i < Ap; i++){
        for(uint32_t j = 0;j < nodeofAp;j++){
            Ptr<Ipv4> ipv4 = wifiStaNodes[i].Get(j)->GetObject<Ipv4> ();
            Ptr<Ipv4StaticRouting> staticRouting =  staticroutinghelper.GetStaticRouting(ipv4);
            
            for(uint32_t k = 1;k < ipv4->GetNInterfaces() ;k++){
                std::ostringstream base_address;
                base_address<<"10.1."<<(2*i+k)<<".1";
                // std::cout<<"base address " << base_address.str().c_str() << std::endl;    
                staticRouting->AddHostRouteTo(Ipv4Address(base_address.str().c_str()),k);
            }
            
            
        }
        
    }
    //Control node build static table
    // Ptr<Ipv4> ipv4 = ControlNode.Get(0)->GetObject<Ipv4> ();
    Ptr<Ipv4StaticRouting> control_staticRouting =  staticroutinghelper.GetStaticRouting(control_ipv4);
    for(uint32_t k = 1;k < control_ipv4->GetNInterfaces() ;k++){
        std::ostringstream base_address;
        base_address<<"10.1."<<(6+k)<<".2";
        // std::cout<<"base address " << base_address.str().c_str() << std::endl;    
        control_staticRouting->AddHostRouteTo(Ipv4Address(base_address.str().c_str()),k);
    }
    for(uint32_t k = 1;k < controlL_ipv4->GetNInterfaces() ;k++){
        std::ostringstream hop_address,dest_csma_address;
        hop_address<<"10.1."<<(6+k)<<".2";
        dest_csma_address<<"10.1."<<2*k - 1<<".1";
        // std::cout<<"base address " << base_address.str().c_str() << std::endl;    
        control_staticRouting->AddHostRouteTo(Ipv4Address(dest_csma_address.str().c_str()),Ipv4Address(hop_address.str().c_str()),k);
        std::ostringstream dest_tdma_address;
        dest_tdma_address<<"10.1."<<2*k <<".1";
        // std::cout<<"base address " << base_address.str().c_str() << std::endl;    
        control_staticRouting->AddHostRouteTo(Ipv4Address(dest_tdma_address.str().c_str()),Ipv4Address(hop_address.str().c_str()),k);
    }

    Ptr<Ipv4StaticRouting> controlL_staticRouting =  staticroutinghelper.GetStaticRouting(controlL_ipv4);
    for(uint32_t k = 1;k < controlL_ipv4->GetNInterfaces() ;k++){
        std::ostringstream base_address;
        base_address<<"10.1."<<(9+k)<<".2";
        // std::cout<<"base address " << base_address.str().c_str() << std::endl;    
        controlL_staticRouting->AddHostRouteTo(Ipv4Address(base_address.str().c_str()),k);
    }
    for(uint32_t k = 1;k < controlL_ipv4->GetNInterfaces() ;k++){
        std::ostringstream hop_address,dest_csma_address,dest_tdma_address;
        hop_address<<"10.1."<<(9+k)<<".2";
        dest_csma_address<<"10.1."<<2*k - 1<<".1";
        // std::cout<<"base address " << base_address.str().c_str() << std::endl;    
        controlL_staticRouting->AddHostRouteTo(Ipv4Address(dest_csma_address.str().c_str()),Ipv4Address(hop_address.str().c_str()),k);
        
        dest_tdma_address<<"10.1."<<2*k<<".1";
        // std::cout<<"base address " << base_address.str().c_str() << std::endl;    
        controlL_staticRouting->AddHostRouteTo(Ipv4Address(dest_tdma_address.str().c_str()),Ipv4Address(hop_address.str().c_str()),k);
    }
    


    // for(uint32_t i = 0; i < Ap;i++){
    //     for(uint32_t  j = 0; j < nodeofAp; j++){
    //         Ptr <ns3::Ipv4> ipv4 = wifiStaNodes[i].Get(j)->GetObject <ns3::Ipv4> ();
    //         std::stringstream stream;
    //         Ptr<OutputStreamWrapper> routingstream = Create<OutputStreamWrapper> (&stream);
    //         ipv4->GetRoutingProtocol ()->PrintRoutingTable (routingstream);
    //         NS_LOG_INFO("route table infomation\n" << stream.str());
    //     }
    // }
    
    // for(uint32_t i = 0; i < Ap;i++){
    //     Ptr <ns3::Ipv4> ipv4 = wifiApNodes.Get(i)->GetObject <ns3::Ipv4> ();
    //     std::stringstream stream;
    //     Ptr<OutputStreamWrapper> routingstream = Create<OutputStreamWrapper> (&stream);
    //     ipv4->GetRoutingProtocol ()->PrintRoutingTable (routingstream);
    //     NS_LOG_INFO("route table infomation\n" << stream.str());
    // }
    
    // // Ptr <ns3::Ipv4> coipv4 = ControlNode.Get(0)->GetObject <ns3::Ipv4> ();
    // std::stringstream control_stream;
    // Ptr<OutputStreamWrapper> control_routingstream = Create<OutputStreamWrapper> (&control_stream);
    // control_ipv4->GetRoutingProtocol ()->PrintRoutingTable (control_routingstream);
    // NS_LOG_INFO("route table infomation\n" << control_stream.str());
    
    // // Ptr <ns3::Ipv4> ipv4 = ControlLeftNode.Get(0)->GetObject <ns3::Ipv4> ();
    // std::stringstream controlL_stream;
    // Ptr<OutputStreamWrapper> controlL_routingstream = Create<OutputStreamWrapper> (&controlL_stream);
    // controlL_ipv4->GetRoutingProtocol ()->PrintRoutingTable (controlL_routingstream);
    // NS_LOG_INFO("route table infomation\n" << controlL_stream.str());


    //CreateSocket
    // std::vector<Ptr<Socket> > ApSockets;
    // std::vector<std::vector<Ptr<Socket>> > Stasockets;
    // Stasockets.resize(Ap);
    // ApSockets.resize(Ap);
    // for(i = 0; i < Ap;i++){
    //     Ptr<Socket> ns3TcpSocket = Socket::CreateSocket(wifiApNodes.Get(i),UdpSocketFactory::GetTypeId ());
    //     ApSockets[i] = ns3TcpSocket;
    // }

    // for(i = 0; i < Ap;i++){
    //     Stasockets[i].resize(nodeofAp);
    //     for(uint32_t j  = 0; j < nodeofAp;j++){
    //         Ptr<Socket> ns3TcpSocket = Socket::CreateSocket(wifiStaNodes[i].Get(j),UdpSocketFactory::GetTypeId ());
    //         Stasockets[i][j] = ns3TcpSocket;
    //     }
    // }
    // Ptr<Socket> ControlLSocket = Socket::CreateSocket(ControlLeftNode.Get(0),UdpSocketFactory::GetTypeId ());
    // Ptr<Socket> ControlSocket = Socket::CreateSocket(ControlNode.Get(0),UdpSocketFactory::GetTypeId ());    
    



    uint32_t sendPort = 999,receivePort = 888;
    //ControlApp
    Ptr<Control1App> controlLApp = CreateObject<Control1App>();
    controlLApp->SetAttribute("Local",AddressValue(InetSocketAddress(ControlLeftInterface[0].GetAddress(0),sendPort)));
    controlLApp->SetAttribute("sendPort",UintegerValue(sendPort));
    // controlLApp->Setup(ControlLSocket,
    //                 csma_ApInterfaces,
    //                 InetSocketAddress(ControlLeftInterface[0].GetAddress(0),port),Ap);
    controlLApp->Setup(Ap,
                    InetSocketAddress(ControlLeftInterface[0].GetAddress(0),sendPort),
                    sendPort,
                    receivePort,
                    InetSocketAddress(ControlLeftInterface[0].GetAddress(1),receivePort),
                    receivePort,
                    controlLeftChildrenInterfaces);
    ControlLeftNode.Get(0)->AddApplication(controlLApp);
    controlLApp->SetStartTime(Seconds(0.));
    controlLApp->SetStopTime(Seconds(2150.));

    Ptr<ControlApp> controlApp = CreateObject<ControlApp> ();
    controlApp->SetAttribute("Local",AddressValue(InetSocketAddress(ControlInterface[0].GetAddress(0),sendPort)));
    controlApp->SetAttribute("sendPort",UintegerValue(sendPort));
    // controlApp->Setup(ControlSocket,
    //                 csma_ApInterfaces,Ap,nodeofAp,Ap*nodeofAp,
    //                 InetSocketAddress(ControlInterface[0].GetAddress(0),port),
    //                 InetSocketAddress(csma_ApInterfaces.GetAddress(0),port));
    controlApp->Setup(Ap,nodeofAp,
                    InetSocketAddress(ControlInterface[0].GetAddress(0),sendPort),
                    sendPort,
                    receivePort,
                    InetSocketAddress(ControlInterface[0].GetAddress(1),receivePort),
                    receivePort,
                    controlChildrenInterfaces);
    ControlNode.Get(0)->AddApplication(controlApp);
    controlApp->SetStartTime(Seconds(0.));
    controlApp->SetStopTime(Seconds(2150.));

    std::ofstream out;
    for(uint32_t i = 0; i < Ap;i++){
        for(uint32_t j = 0; j < nodeofAp ; j++ ){
            std::stringstream ss;
            ss<<"ApNode"<<i<<"EdgeNode"<<j<<".txt";
            out.open(ss.str(),std::ofstream::trunc);
            out<<"No.\tMAC Protocol\tSendTime\tReceiveTime\tSource\tDestination\tLength\tRtt\n";
            out.close();
        }
    }

    // EdgeApp
    std::vector<std::vector<Ptr<EdgeApp>> > StaApps;
   

    StaApps.resize(Ap);
    for(i = 0; i < Ap;i++){
        j = 0;
        Ptr<EdgeApp> app = CreateObject<EdgeApp> ();
        app->Setup(i,j,nodeInterfaces[i][j],sendPort,receivePort,ApInterfaces[i],receivePort,976,100000,DataRate("1.5Mbps"));
        app->SetAttribute("Local",AddressValue(InetSocketAddress(stacsmaInterface[i].GetAddress(j),sendPort)));
        app->SetAttribute("sendPort",UintegerValue(sendPort));
        app->SetAttribute("receivePort",UintegerValue(receivePort));
        app->SetAttribute("Report",BooleanValue(true));
        app->SetAttribute("SendStart",BooleanValue(false));
        wifiStaNodes[i].Get(j)->AddApplication(app);
        app->SetStartTime(Seconds (0));
        app->SetStopTime(Seconds (2100.1));
        StaApps[i].push_back(app);
    }
    

    for(i = 0; i < Ap;i++){
        j = 1;
        Ptr<EdgeApp> app = CreateObject<EdgeApp> ();
        app->Setup(i,j,nodeInterfaces[i][j],sendPort,receivePort,ApInterfaces[i],receivePort,976,100000,DataRate("1.5Mbps"));
        app->SetAttribute("Local",AddressValue(InetSocketAddress(stacsmaInterface[i].GetAddress(j),sendPort)));
        app->SetAttribute("sendPort",UintegerValue(sendPort));
        app->SetAttribute("receivePort",UintegerValue(receivePort));
        app->SetAttribute("Report",BooleanValue(true));
        app->SetAttribute("SendStart",BooleanValue(false));
        wifiStaNodes[i].Get(j)->AddApplication(app);
        app->SetStartTime(Seconds (0));
        app->SetStopTime(Seconds (300.1));
        // StaApps[i].push_back(app);
        
        Ptr<EdgeApp> app1 = CreateObject<EdgeApp> ();
        app1->Setup(i,j,nodeInterfaces[i][j],sendPort,receivePort,ApInterfaces[i],receivePort,976,100000,DataRate("1.5Mbps"));
        app1->SetAttribute("Local",AddressValue(InetSocketAddress(stacsmaInterface[i].GetAddress(j),sendPort)));
        app1->SetAttribute("sendPort",UintegerValue(sendPort));
        app1->SetAttribute("receivePort",UintegerValue(receivePort));
        app1->SetAttribute("Report",BooleanValue(true));
        wifiStaNodes[i].Get(j)->AddApplication(app1);
        app1->SetStartTime(Seconds (600));
        app1->SetStopTime(Seconds (900.1));

        Ptr<EdgeApp> app2 = CreateObject<EdgeApp> ();
        app2->Setup(i,j,nodeInterfaces[i][j],sendPort,receivePort,ApInterfaces[i],receivePort,976,100000,DataRate("1.5Mbps"));
        app2->SetAttribute("Local",AddressValue(InetSocketAddress(stacsmaInterface[i].GetAddress(j),sendPort)));
        app2->SetAttribute("sendPort",UintegerValue(sendPort));
        app2->SetAttribute("receivePort",UintegerValue(receivePort));
        app2->SetAttribute("Report",BooleanValue(true));
        wifiStaNodes[i].Get(j)->AddApplication(app2);
        app2->SetStartTime(Seconds (1500));
        app2->SetStopTime(Seconds (2100.1));
    }

    for(i = 0; i < Ap;i++){
        j = 2;
        Ptr<EdgeApp> app = CreateObject<EdgeApp> ();
        app->Setup(i,j,nodeInterfaces[i][j],sendPort,receivePort,ApInterfaces[i],receivePort,976,100000,DataRate("1.5Mbps"));
        app->SetAttribute("Local",AddressValue(InetSocketAddress(stacsmaInterface[i].GetAddress(j),sendPort)));
        app->SetAttribute("sendPort",UintegerValue(sendPort));
        app->SetAttribute("receivePort",UintegerValue(receivePort));
        app->SetAttribute("Report",BooleanValue(true));
        app->SetAttribute("SendStart",BooleanValue(false));
        wifiStaNodes[i].Get(j)->AddApplication(app);
        app->SetStartTime(Seconds (0));
        app->SetStopTime(Seconds (300.1));
        // StaApps[i].push_back(app);

        Ptr<EdgeApp> app1 = CreateObject<EdgeApp> ();
        app1->Setup(i,j,nodeInterfaces[i][j],sendPort,receivePort,ApInterfaces[i],receivePort,976,100000,DataRate("1.5Mbps"));
        app1->SetAttribute("Local",AddressValue(InetSocketAddress(stacsmaInterface[i].GetAddress(j),sendPort)));
        app1->SetAttribute("sendPort",UintegerValue(sendPort));
        app1->SetAttribute("receivePort",UintegerValue(receivePort));
        app1->SetAttribute("Report",BooleanValue(true));
        wifiStaNodes[i].Get(j)->AddApplication(app1);
        app1->SetStartTime(Seconds (900));
        app1->SetStopTime(Seconds (1200.1));

        Ptr<EdgeApp> app2 = CreateObject<EdgeApp> ();
        app2->Setup(i,j,nodeInterfaces[i][j],sendPort,receivePort,ApInterfaces[i],receivePort,976,100000,DataRate("1.5Mbps"));
        app2->SetAttribute("Local",AddressValue(InetSocketAddress(stacsmaInterface[i].GetAddress(j),sendPort)));
        app2->SetAttribute("sendPort",UintegerValue(sendPort));
        app2->SetAttribute("receivePort",UintegerValue(receivePort));
        app2->SetAttribute("Report",BooleanValue(true));
        wifiStaNodes[i].Get(j)->AddApplication(app2);
        app2->SetStartTime(Seconds (1800));
        app2->SetStopTime(Seconds (2100.1));
    }


    // SenseApp
    std::vector<Ptr<SenseApp> > SenseApps;
    for( i = 0; i < Ap;i++){
        // std::cout<<"build senseapp"<<std::endl;
        Ptr<SenseApp> senseapp = CreateObject<SenseApp> ();
        // senseapp->Setup(ApSockets[i],
        //                 InetSocketAddress(stacsmaInterface[i].GetAddress(0),port),
        //                 stacsmaInterface[i],
        //                 InetSocketAddress(tdma_ApInterfaces.GetAddress(i),port),
        //                 InetSocketAddress(ControlLeftInterface[i].GetAddress(0),port),
        //                 InetSocketAddress(ControlInterface[i].GetAddress(0),port),
        //                 nodeofAp,i);
        senseapp->Setup(nodeofAp,i,
                        InetSocketAddress(csma_ApInterfaces.GetAddress(i),sendPort),
                        sendPort,
                        receivePort,
                        InetSocketAddress(stacsmaInterface[i].GetAddress(0),receivePort),
                        receivePort,
                        stacsmaInterface[i],
                        InetSocketAddress(ControlLeftInterface[i].GetAddress(0),receivePort),
                        InetSocketAddress(ControlInterface[i].GetAddress(0),receivePort)
                    );
        senseapp->SetAttribute("Local",AddressValue(InetSocketAddress(csma_ApInterfaces.GetAddress(i),sendPort)));
        senseapp->SetAttribute("sendPort",UintegerValue(sendPort));
        senseapp->SetAttribute("receivePort",UintegerValue(receivePort));
        wifiApNodes.Get(i)->AddApplication(senseapp);
        senseapp->SetStartTime(Seconds(0));
        senseapp->SetStopTime(Seconds(2150.0));
        SenseApps.push_back(senseapp);
    }

    // Ipv4GlobalRoutingHelper::PopulateRoutingTables();   
    std::cout<<"================Start Simulation======"<<std::endl;
    Simulator::Stop(Seconds (2150.1));

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}