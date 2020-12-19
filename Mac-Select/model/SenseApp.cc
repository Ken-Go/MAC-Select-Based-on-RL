#include "SenseApp.h"

namespace ns3 {
    NS_LOG_COMPONENT_DEFINE ("SenseApp");
    NS_OBJECT_ENSURE_REGISTERED(SenseApp);

    TypeId
    SenseApp::GetTypeId(void){
        static TypeId tid = TypeId("ns3::SenseApp")
            .SetParent<Application>()
            .SetGroupName("Applicaition")
            .AddConstructor<SenseApp>()
            // .AddAttribute()
            .AddAttribute("Local",
                        "Local Address",
                        AddressValue(),
                        MakeAddressAccessor(&SenseApp::m_local),
                        MakeAddressChecker())
            .AddAttribute ("sendPort","send packet port number",
                        UintegerValue(0),
                        MakeUintegerAccessor(&SenseApp::m_localPort),
                        MakeUintegerChecker<uint32_t>())
            .AddAttribute ("receivePort","receive packet port number",
                        UintegerValue(0),
                        MakeUintegerAccessor(&SenseApp::m_receivePort),
                        MakeUintegerChecker<uint32_t>())
            .AddAttribute ("Index","Ap index",
                        UintegerValue(0),
                        MakeUintegerAccessor(&SenseApp::m_index),
                        MakeUintegerChecker<uint32_t>())
            // .AddAttribute ("Remote", "The address of the destination",
            //             AddressValue (),
            //             MakeAddressAccessor (&SenseApp::m_peer),
            //             MakeAddressChecker ()) 
        ;
        return tid;
    }

    SenseApp::SenseApp()
        : m_sendSocket(),
        m_receiveSocket(),
        m_childs(),
        m_fatherControlL(),
        m_fatherControl(),
        m_local(),
        m_peer(),
        m_localPort(0),
        m_peerPort(0),
        m_sendEvent(),
        m_updateEvent(),
        m_temp(),
        m_index(0),
        m_childNum(0),
        m_tolerate(100),
        m_info(),
        m_upTime(),
        m_metrics(),
        m_usingtdmas(),
        m_updates(),         //all node metrics is update?
        m_actions(),
        m_weekup(),
        m_rti(5),
        m_period(10),        // report period
        m_report(true)   //is report
    {

    }
    SenseApp::~SenseApp(){

    }
    void
    SenseApp::Setup(uint32_t childnum,uint32_t index,Address local,uint32_t localPort,uint32_t receivePort,Address peer,uint32_t peerPort,Ipv4InterfaceContainer childrenAddress,Address fatherControlL,Address fatherControl){
        m_childNum = childnum;
        m_index = index;
        m_local = local;
        m_localPort = localPort;
        m_receivePort = receivePort;
        m_peer = peer;
        m_peerPort = peerPort;
        m_childs = childrenAddress;
        m_fatherControl = fatherControl;
        m_fatherControlL = fatherControlL;
        
        m_updates.resize(childnum);
        for(uint32_t i; i < childnum ;i++){
            m_updates[i] = false;
        }
        m_actions.resize(m_childNum,0);
        m_usingtdmas.resize(m_childNum,false);
        m_metrics.resize(m_childNum,0);
        m_weekup.resize(m_childNum,1);
    };

    void
    SenseApp::StartApplication(void){
        // InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny(),m_port);
        
        
        m_sendSocket = Socket::CreateSocket(GetNode(),UdpSocketFactory::GetTypeId());
        m_sendSocket->SetRecvCallback(MakeCallback (&SenseApp::HandleRead,this));
        m_sendSocket->Bind(m_local);
        m_sendSocket->Connect(m_peer);
        
        m_receiveSocket = Socket::CreateSocket(GetNode(),UdpSocketFactory::GetTypeId());
        Address local = InetSocketAddress (Ipv4Address::GetAny(),m_receivePort);
        m_receiveSocket->Bind(local);
        m_receiveSocket->SetRecvCallback(MakeCallback (&SenseApp::HandleRead,this));

        std::cout<<"start SenseApp"<<std::endl;     
        // Update();
        SendMetrics();
    }

    void
    SenseApp::StopApplication(void){
        NS_LOG_INFO("Ap Node " << m_index << " stop Sense Application!!!");
    }

    void
    SenseApp::SendToChild(Ptr<Socket> socket,uint32_t index,Address To,uint32_t tag_value){
        Ptr<Packet> packet = Create<Packet>(0);
        EdgeTag tag;
        tag.SetTagValue(tag_value);
        packet->AddPacketTag(tag);

        SeqTsSizeHeader action;
        action.SetSeq(m_actions[index]);
        packet->AddHeader(action);
        socket->SendTo(packet,0,To);
    }

    void
    SenseApp::HandleRead(Ptr<Socket> socket)
    {
        Ptr<Packet> packet;
        Address from;
        Address localAddress;
        while((packet = socket->RecvFrom(from)))
        {   
            
            // NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s Ap Node "<<m_index<<" received " << packet->GetSize () << " bytes from " <<
            //            InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
            //            InetSocketAddress::ConvertFrom (from).GetPort ());
            if(packet->GetSize() > 0)
            {
                EdgeTag tag;
                packet->PeekPacketTag(tag);
                if(tag.GetTagValue() == 0) // data packet  get packe from edge node
                {
                    //get edge node index
                    SeqTsSizeHeader ts;
                    packet->RemoveHeader(ts);
                    // uint32_t childIndex = ts.GetSeq();
                    // NS_LOG_INFO("Ap Node "<< m_index <<" receive packet from edge node "
                    //              << childIndex <<" at time "<< Simulator::Now().GetSeconds()<<" s");
                    
                    //get packet seq num
                    SeqTsSizeHeader seqnum;
                    packet->RemoveHeader(seqnum);
                    uint32_t seq_num = seqnum.GetSeq();

                    //get send time
                    TimeHeader header;
                    packet->RemoveHeader(header);
                    Time time(Simulator::Now());
                    m_upTime = time.GetMilliSeconds()-header.GetData();
                    
                    Ptr<Packet> pac = Create<Packet> (0);
                    EdgeTag tag;
                    tag.SetTagValue(0);
                    pac->AddPacketTag(tag);

                    //set seq num
                    SeqTsSizeHeader seq2num;
                    seq2num.SetSeq(seq_num);
                    
                    // set uplink time
                    SeqTsSizeHeader header1;
                    header1.SetSeq(m_upTime);
                
                    // set downlink send time
                    TimeHeader header2;
                    header2.SetData(Simulator::Now().GetMilliSeconds());
                    
                    pac->AddHeader(header2);
                    pac->AddHeader(header1);
                    pac->AddHeader(seq2num);
                    
                    socket->SendTo(pac,0,from);
                }else if(tag.GetTagValue() == 1) //uplink metrics packet
                {
                    // NS_LOG_INFO("Receive uplink metrics");
                    SeqTsSizeHeader nodeNum,metrics,usingtdma;
                    packet->RemoveHeader(nodeNum);
                    packet->RemoveHeader(metrics);
                    packet->RemoveHeader(usingtdma);
                    uint32_t node = nodeNum.GetSeq();
                    m_metrics[node] = metrics.GetSeq();
                    m_usingtdmas[node] = usingtdma.GetSeq();
                    m_updates[node] = true;
                }else if(tag.GetTagValue() == 2) //download control packet
                {
                    
                }else if(tag.GetTagValue() == 3) //download metrics packet
                {
                    // NS_LOG_INFO("Ap Node "<< m_index <<" receive packet from Control node at time "<< Simulator::Now().GetSeconds() <<" s");
                    SeqTsSizeHeader nums;
                    packet->RemoveHeader(nums);
                  
                    uint32_t m_nums= nums.GetSeq();
                    std::vector<SeqTsSizeHeader> headers(m_nums);
                    for(uint32_t i = 0;i < m_nums;i++){
                        packet->RemoveHeader(headers[i]);
                        m_actions[i] = headers[i].GetSeq();
                        if(m_weekup[i] == 1){
                            // std::cout<<"Ap Node"<<m_index<<" children index "<<i<<" action is "<< m_actions[i] <<" (0:tdma,1:csma)"<<std::endl;
                            SendToChild(m_sendSocket,i,InetSocketAddress(m_childs.GetAddress(i),m_peerPort),2);
                        }                        
                    }     
                           
                }else if(tag.GetTagValue() == 4){   //report children node is weekup or sleep
                    SeqTsSizeHeader ts;
                    packet->RemoveHeader(ts);
                    uint32_t child_index = ts.GetSeq();
                    SeqTsSizeHeader state;
                    packet->RemoveHeader(state);
                    uint32_t child_state = state.GetSeq();
                    if(child_state == 0){
                        m_weekup[child_index] = 0;
                        m_usingtdmas[child_index] = false;
                        m_metrics[child_index] = 0;
                    }else{
                        m_weekup[child_index] = 1;
                        m_usingtdmas[child_index] = false;
                        m_metrics[child_index] = 0;
                    }

                    Ptr<Packet> pac = Create<Packet> (0);
                    EdgeTag tag;
                    tag.SetTagValue(4);
                    pac->AddPacketTag(tag);
                    
                    // set ap Node index
                    SeqTsSizeHeader ApNode;
                    ApNode.SetSeq(m_index);
                    // set edge node index
                    SeqTsSizeHeader EdgeNode;
                    EdgeNode.SetSeq(child_index);
                    // set state
                    SeqTsSizeHeader State;
                    State.SetSeq(m_weekup[child_index]);
                    
                    pac->AddHeader(State);
                    pac->AddHeader(EdgeNode);
                    pac->AddHeader(ApNode);
                    // NS_LOG_INFO("Send to father: children node state!! "<< child_state << " 0: sleep,1:weekup");
                    socket->SendTo(pac,0,m_fatherControl);
                }else if (tag.GetTagValue() == 5) { //download restart control
                    SeqTsSizeHeader edgeindex;
                    packet->RemoveHeader(edgeindex);
                    uint32_t  edge_index = edgeindex.GetSeq();

                    SeqTsSizeHeader action;
                    packet->RemoveHeader(action);
                    uint32_t Action = action.GetSeq();

                    m_actions[edge_index] = Action;
                    // std::cout<<"SenseApp::Tag==5--Ap Node"<<m_index<<" children index "<<edge_index<<" action is "<< m_actions[edge_index] <<" (0:tdma,1:csma)"<<std::endl;
                    SendToChild(m_sendSocket,edge_index,InetSocketAddress(m_childs.GetAddress(edge_index),m_peerPort),2);
                } 
            }
        }
    }

    // void
    // SenseApp::Update()
    // {
    //     Ptr<Packet> packet = Create<Packet>(0);
    //     EdgeTag tag;
    //     tag.SetTagValue(0);
    //     m_socket->SendTo(packet,0,m_father);
    // }

    // void
    // SenseApp::ScheduleUpdate(void)
    // {
    //     Time tNext (Seconds (m_period));
    //     m_updateEvent = Simulator::Schedule(tNext,&SenseApp::Update,this);
    // }
    void
    SenseApp::ScheduleSend(void)
    {
        Time tNext(Seconds (m_rti));
        m_sendEvent = Simulator::Schedule (tNext,&SenseApp::SendMetrics,this);
    }
    bool 
    SenseApp::collectAll(){
        for(uint32_t i = 0;i < m_childNum;i++){
            if(m_updates[i] == false && m_weekup[i] == 1)
                return false;
        }
        return true;
    }
    void 
    SenseApp::ResetUpdate(){
        for(uint32_t i = 0;i < m_childNum;i++){
            m_updates[i] = false;
        }
    }
    void
    SenseApp::SendMetrics()
    {   
        if(collectAll()){
            // std::cout<<"SenseApp Collect All"<<std::endl;
            if(m_temp.IsRunning())
                Simulator::Cancel(m_temp);
            Ptr<Packet> pac = Create<Packet>(0);
            EdgeTag tag;
            tag.SetTagValue(1);
            pac->AddPacketTag(tag);

            SeqTsSizeHeader index;
            index.SetSeq(m_index);
            
            SeqTsSizeHeader nums;
            nums.SetSeq(m_childNum);
            

            std::vector<SeqTsSizeHeader> headers(m_childNum);
            std::vector<SeqTsSizeHeader> headers_usingtdma(m_childNum);
            for(uint32_t i = 0;i < m_childNum;i++){
                headers[i].SetSeq(m_metrics[i]);  
                headers_usingtdma[i].SetSeq(m_usingtdmas[i]);
                
            }
            for(int i = m_childNum-1;i >= 0;i--){    
                pac->AddHeader(headers_usingtdma[i]);
            }
            for(int i = m_childNum-1;i >= 0 ;i--){    
                pac->AddHeader(headers[i]);
            }
            pac->AddHeader(nums);
            pac->AddHeader(index);

            m_sendSocket->SendTo(pac,0,m_fatherControl);
            ResetUpdate();
            ScheduleSend();
        }else{
            
            // std::cout<<"SenseApp next sendmetrics"<<std::endl;
            Time tNext(MilliSeconds (m_tolerate));
            m_temp = Simulator::Schedule(tNext,&SenseApp::SendMetrics,this);
        }
    }
	
}
