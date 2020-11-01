#include "EdgeApp.h"

namespace ns3 {
    NS_LOG_COMPONENT_DEFINE ("EdgeApp");
    NS_OBJECT_ENSURE_REGISTERED(EdgeApp);
    // Edge function

    TypeId 
    EdgeApp::GetTypeId(void){
        static TypeId tid = TypeId("ns3::EdgeApp")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<EdgeApp>()
            .AddTraceSource ("Rx", "A packet has been received",
                        MakeTraceSourceAccessor (&EdgeApp::m_rxTrace),
                        "ns3::Packet::TracedCallback")
            .AddTraceSource ("RxWithAddresses", "A packet has been received",
                        MakeTraceSourceAccessor (&EdgeApp::m_rxTraceWithAddress),
                        "ns3::Packet::TwoAddressTracedCallback")
            .AddAttribute("MetricType",
                        " 1:latency 2:throughput 3:latency and throughput",
                        UintegerValue(1),
                        MakeUintegerAccessor(&EdgeApp::m_metrxType),
                        MakeUintegerChecker<uint32_t > ())
            .AddAttribute("Local",
                        "Local Address",
                        AddressValue(),
                        MakeAddressAccessor(&EdgeApp::m_local),
                        MakeAddressChecker())
            .AddAttribute("DateRate",
                        "the data rate in on state",
                        DataRateValue (DataRate("500kb/s")),
                        MakeDataRateAccessor(&EdgeApp::m_dataRate),
                        MakeDataRateChecker())
            .AddAttribute ("Remote", "The address of the destination",
                        AddressValue (),
                        MakeAddressAccessor (&EdgeApp::m_peer),
                        MakeAddressChecker ())  
            .AddAttribute ("PacketSize", "The size of packets sent in on state",
                        UintegerValue (512),
                        MakeUintegerAccessor (&EdgeApp::m_packetSize),
                        MakeUintegerChecker<uint32_t> (1))
            .AddAttribute ("Report", "need report to AP?",
                        BooleanValue (false),
                        MakeBooleanAccessor (&EdgeApp::m_report),
                        MakeBooleanChecker())
            .AddAttribute ("Port","port",
                        UintegerValue(),
                        MakeUintegerAccessor(&EdgeApp::m_port),
                        MakeUintegerChecker<uint16_t>())
            
        ;
        return tid;
    }

    EdgeApp::EdgeApp()
        : m_socket(0),
        m_peer(),
        m_local(),
        m_sendEvent(),
        m_reEvent(),
        m_running(false),
        m_packetsSent(0),
        m_packetSize(0),
        m_nPackets(0),
        m_dataRate(0),
        m_rti (5),
        m_avelatency(0),
        m_count(0),
        m_latencyNow(0),
        m_usingtdma(false),
        m_change (false),
        m_report(false),
        m_metrxType(1),
        m_childIndex(0),
        m_interfaces(),
        m_interfaceIndex(0),
        m_fatherIndex(0),
        m_port(),
        m_BindDevice()
    {
        // ReportOnTime();
    }
    EdgeApp::~EdgeApp(){
    
    }
    void
    EdgeApp::Setup (Ptr<NetDevice> BindDevice,Address local,uint32_t fatherIndex,Ptr<Socket> socket, Address peeraddress, uint32_t packetSize, uint32_t nPackets, DataRate dataRate,uint32_t childnum)
    {
        m_fatherIndex = fatherIndex;
        m_socket = socket;
        m_peer = peeraddress;
        m_packetSize = packetSize;
        m_nPackets = nPackets;
        m_dataRate = dataRate;
        m_childIndex = childnum;
        m_local = local;
        m_BindDevice = BindDevice;
    }
    
    void EdgeApp::StartApplication(void){
        std::cout<<"start Edge app"<<std::endl;
        m_running = true;
        m_packetsSent = 0;


        // m_local = InetSocketAddress("10.1.2.2",m_port);
        // NS_LOG_INFO ("m_local address and port"
        //                << InetSocketAddress::ConvertFrom(m_local).GetIpv4 ()
        //                << " port " << InetSocketAddress::ConvertFrom (m_local).GetPort ());
      

        // Ptr<Socket> tdma_Socket = Socket::CreateSocket (GetNode(), UdpSocketFactory::GetTypeId ());
        // tdma_Socket->Bind(m_local);
        // tdma_Socket->Connect(m_peer);
        
        
        // m_local = InetSocketAddress("10.1.1.2",m_port);
        // NS_LOG_INFO ("m_local address and port"
        //                << InetSocketAddress::ConvertFrom(m_local).GetIpv4 ()
        //                << " port " << InetSocketAddress::ConvertFrom (m_local).GetPort ());
        // Ptr<Socket> wifi_Socket = Socket::CreateSocket (GetNode(), UdpSocketFactory::GetTypeId ());
        // wifi_Socket->Bind(m_local);
        // wifi_Socket->Connect(m_peer);
        
        // Address sockname;
       
        // m_socket = wifi_Socket;

        // m_socket->GetSockName(sockname);
        // NS_LOG_INFO("m_socket local name"
        //             << InetSocketAddress::ConvertFrom(sockname).GetIpv4()
        //             <<" Port " << InetSocketAddress::ConvertFrom(sockname).GetPort());

        // m_socket = tdma_Socket;

        // m_socket->GetSockName(sockname);
        // NS_LOG_INFO("m_socket local name"
        //             << InetSocketAddress::ConvertFrom(sockname).GetIpv4()
        //             <<" Port " << InetSocketAddress::ConvertFrom(sockname).GetPort());


        m_socket->BindToNetDevice(m_BindDevice);
        m_socket->Bind(m_local);
        m_socket->Connect (m_peer);
        m_socket->SetRecvCallback(MakeCallback (&EdgeApp::HandleRead,this));
        m_socket->SetAllowBroadcast(false);
  
        

        // if(m_report)
        //     ReportOnTime();
        SendPacket ();
    }
    void EdgeApp::StopApplication(void){
        m_running = false;
        if(m_sendEvent.IsRunning ()){
            Simulator::Cancel(m_sendEvent);
        }
    }

    void EdgeApp::SendPacket(void){
      

        Ptr<Packet> packet = Create<Packet> (m_packetSize);
        Address add;
        m_socket->GetSockName(add);
        
        // set data tag : 0 data,1 up metrics,2 download control,3 download metrics
        EdgeTag tag;
        tag.SetTagValue(0); 
        packet->AddPacketTag(tag);
        // set time header now
        TimeHeader header;
        header.SetData((uint32_t)Simulator::Now().GetMilliSeconds());
        packet->AddHeader(header);
        m_socket->SendTo(packet,0,m_peer);
        NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                       << "s EdgeApp application sent "
                       <<  packet->GetSize () << " bytes to "
                       << InetSocketAddress::ConvertFrom(m_peer).GetIpv4 ()
                       << " port " << InetSocketAddress::ConvertFrom (m_peer).GetPort ());
        if(++m_packetsSent < m_nPackets){
            ScheduleTx ();
        }
    }
    void
    EdgeApp::ScheduleTx(void){
        if (m_running){
            Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate())));
            m_sendEvent = Simulator::Schedule (tNext,&EdgeApp::SendPacket,this);
        }
    }

    void EdgeApp::ScheduleRe(void){
        Time tNext (Seconds (m_rti));
        m_reEvent = Simulator::Schedule(tNext,&EdgeApp::ReportOnTime,this);
    }

    void EdgeApp::ReportOnTime(){
        Ptr<Packet> packet = Create<Packet> (0);
        //add tag: metrics
        EdgeTag tag;
        tag.SetTagValue(1);
        packet->AddPacketTag(tag);

        SeqTsSizeHeader  nodeNum;
        nodeNum.SetSeq(m_childIndex);
        //add metrics header
        SeqTsSizeHeader metrics;
        if(m_count == 0){
        metrics.SetSeq(0); 
        }else{
            metrics.SetSeq(m_avelatency / m_count);
        }
        //add suingtdma header
        SeqTsSizeHeader usingtdma;
        if(m_usingtdma)
            usingtdma.SetSeq(1);
        else
            usingtdma.SetSeq(0);
        packet->AddHeader(nodeNum);
        packet->AddHeader(metrics);
        packet->AddHeader(usingtdma);
        m_socket->Send(packet);
        //reset 
        m_avelatency = 0;
        m_count = 0;
        if((uint32_t)Simulator::Now().GetSeconds() % m_rti == 0 || Simulator::Now().GetSeconds() == 0){
            // scheduleRe();
            std::cout<<"the time is "<<Simulator::Now().GetSeconds()<<" s"<<std::endl;
            ScheduleRe ();
        }
    }

    void EdgeApp::CalculateRTT(uint64_t startTime,uint64_t endTime){
        m_latencyNow = endTime - startTime;    
    }


    bool
    EdgeApp::GetUsingTdma(){
        return m_usingtdma;
    }

    void EdgeApp::UpdateMacPro(){
        if(m_change){
            std::cout<<"change mac"<<std::endl;
            if(m_usingtdma){ //tdma -> csma
                std::cout<<"change mac tdma -> csma"<<std::endl;
                // this->SetAttribute("Local",AddressValue(InetSocketAddress(m_interfaces.GetAddress(0))));
                m_local = InetSocketAddress(m_interfaces.GetAddress(0));
                m_socket->Bind(m_local);
                m_usingtdma = false;
            }else{   //csma -> tdma
                std::cout<<"change mac csma -> tdma"<<std::endl;
                
                // this->SetAttribute("Local",AddressValue(InetSocketAddress(m_interfaces.GetAddress(1))));
                m_local = InetSocketAddress(m_interfaces.GetAddress(1));
                m_socket->Bind(m_local);
                m_usingtdma = true;
            }
        }
    }




    void EdgeApp::HandleRead (Ptr<Socket> socket){
        // NS_LOG_FUNCTION (this << socket);
        Ptr<Packet> packet;
        Address from;
        Address localAddress;
        while ((packet = socket->RecvFrom(from)))
        {
            socket->GetSockName(localAddress);
            NS_LOG_INFO ("EdgeApp:: At time " << Simulator::Now ().GetSeconds () << "s server received " << packet->GetSize () << " bytes from " <<
                       InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                       InetSocketAddress::ConvertFrom (from).GetPort ());
            m_rxTrace (packet);
            m_rxTraceWithAddress(packet,from,localAddress);
        
            if (packet->GetSize () > 0){
                EdgeTag tag;
                packet->PeekPacketTag(tag);
                
                if (tag.GetTagValue() == 0) {
            
                    // NS_LOG_INFO("it is a data packet" + tag.GetTagValue());
                    //ischange
                    SeqTsSizeHeader seqTs;
                    packet->RemoveHeader(seqTs);
                    uint32_t firstUpTime = seqTs.GetSeq();
                    //reciv time
                    TimeHeader header;
                    packet->RemoveHeader(header);
                    Time time(Simulator::Now());
                    CalculateRTT(header.GetData(),time.GetMilliSeconds());
                    m_count++;
                    m_avelatency = m_avelatency + m_latencyNow + firstUpTime;
                }else if(tag.GetTagValue() == 1){  // uplink metrics packet
                
                }else if (tag.GetTagValue() == 2 ){ //download control packet
                
                    // NS_LOG_INFO("it is a control packet" + tag.GetTagValue());
                    SeqTsSizeHeader action;
                    packet->RemoveHeader(action);
                    uint32_t isChange = action.GetSeq();
                    
                    if (isChange == 0 && m_usingtdma == false){
                        m_change = true;
                        UpdateMacPro();
                    }
                    else if (isChange == 0 && m_usingtdma == true)
                    {
                        m_change = false;
                    }else if(isChange == 1 && m_usingtdma == false){
                        m_change = false;
                    }else if(isChange == 1 && m_usingtdma == true){
                        m_change = true;
                        UpdateMacPro();
                    }
                }else if(tag.GetTagValue() == 3){   // download metrics packet
                    
                }
            }
        } 
    }

}

