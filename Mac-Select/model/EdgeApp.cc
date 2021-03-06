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
            .AddAttribute ("sendPort","send port number",
                        UintegerValue(),
                        MakeUintegerAccessor(&EdgeApp::m_localPort),
                        MakeUintegerChecker<uint32_t>())
            .AddAttribute ("receivePort","receive packet port num",
                        UintegerValue(),
                        MakeUintegerAccessor(&EdgeApp::m_receivePort),
                        MakeUintegerChecker<uint32_t>())
            .AddAttribute ("SendStart","send start info",
                        BooleanValue(true),
                        MakeBooleanAccessor (&EdgeApp::m_sendStart),
                        MakeBooleanChecker())
            
        ;
        return tid;
    }

    EdgeApp::EdgeApp()
        : m_sendSocket(0),
        m_csmaSocket(0),
        m_tdmaSocket(0),
        m_receiveSocket(0),
        m_peer(),
        m_local(),
        m_localPort(0),
        m_receivePort(0),
        m_peerPort(0),
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
        m_fatherIndex(0),
        m_port(),
        m_outStream(),
        m_packetSeq()
    {
        // ReportOnTime();
    }
    EdgeApp::~EdgeApp(){
    
    }
    void
    EdgeApp::Setup (uint32_t fatherIndex,uint32_t index,Ipv4InterfaceContainer locals,uint32_t localPort,uint32_t receivePort,Ipv4InterfaceContainer peers,uint32_t peerPort, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
    {
        m_fatherIndex = fatherIndex;
        m_childIndex = index;
        m_locals = locals;
        m_peers = peers;
        m_localPort = localPort;
        m_receivePort = receivePort;
        m_peerPort = peerPort;        
        
        m_packetSize = packetSize;
        m_nPackets = nPackets;
        m_dataRate = dataRate;
        m_packetSeq = 0;
    
    }
    
    void EdgeApp::StartApplication(void){
        std::cout<<"start Edge app"<<std::endl;
        m_running = true;
        m_packetsSent = 0;
        //create two socket and send msg to different channel respectively;
        m_csmaSocket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
        m_csmaSocket->Bind(InetSocketAddress(m_locals.GetAddress(0),m_localPort));
        m_csmaSocket->Connect(InetSocketAddress(m_peers.GetAddress(0),m_peerPort));
        m_csmaSocket->SetRecvCallback(MakeCallback (&EdgeApp::HandleRead,this));

        m_tdmaSocket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
        m_tdmaSocket->Bind(InetSocketAddress(m_locals.GetAddress(1),m_localPort));
        m_tdmaSocket->Connect(InetSocketAddress(m_peers.GetAddress(1),m_peerPort));
        m_tdmaSocket->SetRecvCallback(MakeCallback (&EdgeApp::HandleRead,this));

        m_sendSocket = m_csmaSocket;
        m_peer = InetSocketAddress(m_peers.GetAddress(0),m_peerPort);
        m_local = InetSocketAddress(m_locals.GetAddress(0),m_localPort);
        
        m_usingtdma = false;
        // m_sendSocket = m_tdmaSocket;
        // m_peer = InetSocketAddress(m_peers.GetAddress(1),m_peerPort);
        // m_local = InetSocketAddress(m_locals.GetAddress(1),m_localPort);
        
        
        m_receiveSocket = Socket::CreateSocket(GetNode(),UdpSocketFactory::GetTypeId());
        InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(),m_receivePort);
        m_receiveSocket->Bind(local);
        m_receiveSocket->SetRecvCallback(MakeCallback (&EdgeApp::HandleRead,this));
        
        std::stringstream ss;
        ss<<"ApNode"<<m_fatherIndex<<"EdgeNode"<<m_childIndex<<".txt";
        m_outStream.open(ss.str(),std::ofstream::out | std::ofstream::app);
        
        // m_outStream.close();

        
        //report metrics
        if(m_report){
            // ReportOnTime();
            Time tNext (Seconds (m_rti));
            m_reEvent = Simulator::Schedule(tNext,&EdgeApp::ReportOnTime,this);
        }
           
        m_change = false;
        if(m_sendStart){
            SendStart();
        }
        
        SendPacket ();

    }
    void EdgeApp::StopApplication(void){
        m_running = false;
        NS_LOG_INFO("Ap Node "<<m_fatherIndex<< " Edge Node " << m_childIndex << " stop Edge application");
        if(m_sendEvent.IsRunning ()){
            Simulator::Cancel(m_sendEvent);
        }
        if(m_reEvent.IsRunning ()){
            Simulator::Cancel(m_reEvent);
        }
        m_outStream.close();
        SendEnd();
        if(m_receiveSocket != 0){
            m_receiveSocket->Close();
        }
        if(m_sendSocket != 0){
            m_sendSocket->Close();
        }
        if(m_tdmaSocket != 0){
            m_tdmaSocket->Close();
        }
        if(m_csmaSocket != 0){
            m_csmaSocket->Close();
        }
    }
    void EdgeApp::SendStart(void){
        Ptr<Packet> packet = Create<Packet> (0);
        Address address;
        
        EdgeTag tag;
        tag.SetTagValue(4);
        packet->AddPacketTag(tag);
        //set Edge Node Index
        SeqTsSizeHeader ts;
        ts.SetSeq(m_childIndex);
        //set state is weekup or sleep(0:sleep,1:weekup)
        SeqTsSizeHeader start;
        start.SetSeq(1);

        packet->AddHeader(start);
        packet->AddHeader(ts);

        Address peerto;
        m_sendSocket->GetPeerName(peerto);
        m_sendSocket->SendTo(packet,0,peerto);
    }
    void EdgeApp::SendEnd(void){
        Ptr<Packet> packet = Create<Packet> (0);
        Address address;
        
        EdgeTag tag;
        tag.SetTagValue(4);
        packet->AddPacketTag(tag);

        //set Edge Node Index
        SeqTsSizeHeader ts;
        ts.SetSeq(m_childIndex);

        //set state is weekup or sleep(0:sleep,1:weekup)
        SeqTsSizeHeader sleep;
        sleep.SetSeq(0);

        packet->AddHeader(sleep);
        packet->AddHeader(ts);

        Address peerto;
        m_sendSocket->GetPeerName(peerto);
        m_sendSocket->SendTo(packet,0,peerto);
    }
    void EdgeApp::SendPacket(void){
        Ptr<Packet> packet = Create<Packet> (m_packetSize);
        Address add;
        m_sendSocket->GetSockName(add);
        // set data tag : 0 data,1 up metrics,2 download control,3 download metrics
        EdgeTag tag;
        tag.SetTagValue(0); 
        packet->AddPacketTag(tag);

        //set Edge Node index
        SeqTsSizeHeader ts;
        ts.SetSeq(m_childIndex);
       

        //set seq num
        SeqTsSizeHeader seqnum;
        seqnum.SetSeq(m_packetSeq);
        
        
        // set time header now
        TimeHeader header;
        header.SetData((uint32_t)Simulator::Now().GetMilliSeconds());

        packet->AddHeader(header);
        packet->AddHeader(seqnum);
        packet->AddHeader(ts);
            
        Address peerto;
        m_sendSocket->GetPeerName(peerto);
        
        
        // NS_LOG_INFO ("Ap Node "<< m_fatherIndex
        //             << " Edge Node "
        //             << m_childIndex
        //             <<" local address is "
        //             << InetSocketAddress::ConvertFrom(m_local).GetIpv4()
        //             << " At time " << Simulator::Now ().GetSeconds ()
        //             << "s EdgeApp application sent "
        //             << packet->GetSize () << " bytes to "
        //             << InetSocketAddress::ConvertFrom(peerto).GetIpv4 ()
        //             << " port " << InetSocketAddress::ConvertFrom (peerto).GetPort ());
        
        m_sendSocket->SendTo(packet,0,peerto);
        PacketInfo packetInfo;
        packetInfo.SetSendTime(Simulator::Now ().GetMilliSeconds ());
        packetInfo.SetSeq(m_packetSeq);
        if (m_usingtdma){
            packetInfo.SetMacProtocol("TDMA");
        }else{
            packetInfo.SetMacProtocol("CSMA");
        }
        packetInfo.SetLength(packet->GetSize());
        packetInfo.SetSource(m_local);
        packetInfo.SetDestination(peerto);
        m_PacketInfos.push_back(packetInfo);
        m_packetSeq++;
        if(m_running){
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
        // NS_LOG_INFO("report On time:" << Simulator::Now().GetSeconds());
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
        //add usingtdma header
        SeqTsSizeHeader usingtdma;
        if(m_usingtdma)
            usingtdma.SetSeq(1);
        else
            usingtdma.SetSeq(0);
        
        packet->AddHeader(usingtdma);
        packet->AddHeader(metrics);
        packet->AddHeader(nodeNum);

        m_sendSocket->Send(packet);

        //reset 
        m_avelatency = 0;
        m_count = 0;

        // std::stringstream writeline;
        // writeline<<"\n"<<m_rtt.size()<<"\t"<<Simulator::Now().GetSeconds()<<"\t";
        // m_outStream<<writeline.str();
        
        if((uint32_t)Simulator::Now().GetSeconds() % m_rti == 0 || Simulator::Now().GetSeconds() == 0){
            // scheduleRe();
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
        Ptr <ns3::Ipv4> ipv4 = GetNode()->GetObject<ns3::Ipv4> ();
        
        if(m_change){
            // std::cout<<"change mac"<<std::endl;
            if(m_usingtdma){ //tdma -> csma
                // std::cout<<"change mac tdma -> csma"<<std::endl;
                // this->SetAttribute("Local",AddressValue(InetSocketAddress(m_interfaces.GetAddress(0))));
                // m_local = InetSocketAddress(m_interfaces.GetAddress(0));
                // m_socket->Bind(m_local);
                m_sendSocket = m_csmaSocket;
                m_peer = InetSocketAddress(m_peers.GetAddress(0),m_peerPort);
                m_local = InetSocketAddress(m_locals.GetAddress(0),m_localPort);
                m_usingtdma = false;
            }else{   //csma -> tdma
                // std::cout<<"change mac csma -> tdma"<<std::endl;
                m_sendSocket = m_tdmaSocket;
                m_peer = InetSocketAddress(m_peers.GetAddress(1),m_peerPort);
                m_local = InetSocketAddress(m_locals.GetAddress(1),m_localPort);
                m_usingtdma = true;
            }
            m_change = false;
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
            // NS_LOG_INFO ("Ap Node "<<m_fatherIndex
            //                 << " EdgeApp Node "
            //                 << m_childIndex
            //                 << " At time " 
            //                 << Simulator::Now ().GetSeconds () 
            //                 << " s server received " 
            //                 << packet->GetSize () 
            //                 << " bytes from " 
            //                 << InetSocketAddress::ConvertFrom (from).GetIpv4 () 
            //                 << " port " 
            //                 << InetSocketAddress::ConvertFrom (from).GetPort ());
            m_rxTrace (packet);
            m_rxTraceWithAddress(packet,from,localAddress);
        
            if (packet->GetSize () > 0){
                EdgeTag tag;
                packet->PeekPacketTag(tag);
                
                if (tag.GetTagValue() == 0) {
                    // NS_LOG_INFO("Receive Data packet");
                    //get packet seq num
                    SeqTsSizeHeader seqnum;
                    packet->RemoveHeader(seqnum);
                    uint32_t index = seqnum.GetSeq();
                    // NS_LOG_INFO("receive seq is : " << index);

                    //get uplink time
                    SeqTsSizeHeader seqTs;
                    packet->RemoveHeader(seqTs);
                    uint32_t firstUpTime = seqTs.GetSeq();
                    // NS_LOG_INFO("fistr up time " << firstUpTime << " ms");
                    
                    //reciv time
                    TimeHeader header;
                    packet->RemoveHeader(header);
                    uint32_t downlinkSendtime = header.GetData();
                    // NS_LOG_INFO("downlinkSendtime " << downlinkSendtime << " ms");

                    Time time(Simulator::Now());
                    uint32_t downloadlinktime = time.GetMilliSeconds() - downlinkSendtime;
                    uint32_t rtt = firstUpTime + downloadlinktime;
                    // NS_LOG_INFO("m_PacketInfos size is : " << m_PacketInfos.size());
                    if(m_PacketInfos[index].getSeq() != index ){
                        NS_ASSERT("Infomation index is wrong!!");
                    }else{
                        m_PacketInfos[index].SetReceiveTime(Simulator::Now().GetMilliSeconds());
                        m_PacketInfos[index].SetRtt(rtt);  
                        std::stringstream ss;
                        ss<<m_PacketInfos[index].getSeq()<<"\t"<<m_PacketInfos[index].getMacProtocol()<<"\t"<<m_PacketInfos[index].getSendTime()<<"\t"<<m_PacketInfos[index].getReceiveTime()<<"\t"<<InetSocketAddress::ConvertFrom(m_PacketInfos[index].getSource()).GetIpv4()<<"\t"<<InetSocketAddress::ConvertFrom(m_PacketInfos[index].getDestination()).GetIpv4()<<"\t"<<m_PacketInfos[index].getLength()<<"\t"<<m_PacketInfos[index].getRtt()<<"\n";
                        m_outStream<<ss.str();
                    }
                    // CalculateRTT(header.GetData(),time.GetMilliSeconds());
                    m_count++;
                    m_avelatency = m_avelatency + rtt;
                }else if(tag.GetTagValue() == 1){  // uplink metrics packet
                
                }else if (tag.GetTagValue() == 2 ){ //download control packet
                    // NS_LOG_INFO("EdgeApp::Receive Control packet");
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
                    
                }else if(tag.GetTagValue() == 4){    //report weekup  or sleep

                }
            }
        } 
    }

}

