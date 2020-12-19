#include "ControlApp.h"

namespace ns3 {
    NS_LOG_COMPONENT_DEFINE ("ControlApp");
    NS_OBJECT_ENSURE_REGISTERED(ControlApp);
    TypeId
    ControlApp::GetTypeId(void){
        static TypeId tid = TypeId("ns3::ControlApp")
            .SetParent<Application>()
            .SetGroupName("Application")
            .AddConstructor<ControlApp>()
            .AddAttribute("Local",
                        "local Address",
                        AddressValue(),
                        MakeAddressAccessor(&ControlApp::m_local),
                        MakeAddressChecker())
            .AddAttribute ("sendPort","send packet from this port",
                        UintegerValue(0),
                        MakeUintegerAccessor(&ControlApp::m_sendPort),
                        MakeUintegerChecker<uint32_t>())
            .AddAttribute ("receivePort","receive packet from this port",
                        UintegerValue(0),
                        MakeUintegerAccessor(&ControlApp::m_receivePort),
                        MakeUintegerChecker<uint32_t>())
        ;
        return tid;
    }
    ControlApp::ControlApp()
        : m_qlearns(),
        m_metrics(),
        m_states(),
        m_lastMetrics(),
        m_WeekUp(),
        m_apnum(0),
        m_edgenum(),
        m_allnum(0), 
        m_sendSocket(),
        m_receiveSocket(),
        m_children(),
        m_local(),
        m_peer(),
        m_sendPort(0),
        m_receivePort(0),
        m_peerPort(0),
        m_updataEvent(),
        m_reportTimeEvent(),
        m_count(0)
    {
        
    }
    ControlApp::~ControlApp(){

    }
    void 
    ControlApp::Setup(uint32_t apnum,uint32_t edgeOfAp,Address local,uint32_t sendPort,uint32_t receivePort,Address peer,uint32_t peerPort,Ipv4InterfaceContainer children)
    {
        m_apnum = apnum;
        m_edgenum = edgeOfAp;
        m_local = local;
        m_children = children;
        m_sendPort = sendPort;
        m_receivePort = receivePort;
        m_peerPort = peerPort;
        m_peer = peer;

        m_qlearns.resize(m_apnum);
        m_metrics.resize(m_apnum);
        m_states.resize(m_apnum);
        m_lastMetrics.resize(m_apnum);
        m_WeekUp.resize(m_apnum);
        for(uint32_t i =0;i < m_apnum;i++){
            m_qlearns[i].resize(m_edgenum);
            m_metrics[i].resize(m_edgenum);
            m_states[i].resize(m_edgenum);
            m_lastMetrics[i].resize(m_edgenum);
            m_WeekUp[i].resize(m_edgenum);
        }
        for(uint32_t i = 0; i < m_apnum;i++){
            for(uint32_t j = 0; j < m_edgenum;j++){
                m_metrics[i][j] = 0;
                m_lastMetrics[i][j] = 0;       
                m_states[i][j] = 0;
                m_WeekUp[i][j] = 1;
                QLearn* q = new QLearn();
                m_qlearns[i][j] = q;
            }
        }


    }
    // void
    // ControlApp::SendControl(Ptr<Socket> socket,uint32_t apIndex,std::vector<uint32_t> state){
    //     Ptr<Packet> packet = Create<Packet>(0);
    //     EdgeTag tag;
    //     tag.SetTagValue(3);
    //     packet->AddPacketTag(tag);
    //     SeqTsSizeHeader nums;
    //     nums.SetSeq(m_edgenum);
    //     packet->AddHeader(nums);
    //     std::vector<SeqTsSizeHeader> headers(m_edgenum);
    //     for(uint32_t i; i < m_edgenum;i++){
    //         m_qlearns[apIndex][i]->get_env_feedback(state[i],0);
    //         headers[i].SetSeq(m_qlearns[apIndex][i]->choose_action());
    //         packet->AddHeader(headers[i]);
    //     }
    //     socket->Send(packet,0);
    // }
    void
    ControlApp::SendControl(Ptr<Socket> socket,uint32_t apIndex,Address from){
        Ptr<Packet> packet = Create<Packet>(0);
        EdgeTag tag;
        tag.SetTagValue(3);
        packet->AddPacketTag(tag);

        SeqTsSizeHeader nums;
        nums.SetSeq(m_edgenum);
        
        std::vector<SeqTsSizeHeader> headers(m_edgenum);
        for(int i = m_edgenum-1; i >= 0;i--){
            if(m_WeekUp[apIndex][i] == 1){
                uint32_t action = m_qlearns[apIndex][i]->choose_action();
                // NS_LOG_INFO("Control decide Ap Node " << apIndex <<" Edge Node " << i << " action is " << action);
                headers[i].SetSeq(action);
                packet->AddHeader(headers[i]);
            }else{
                uint32_t action  =  3;
                headers[i].SetSeq(action);
                packet->AddHeader(headers[i]);
            }
        }
        packet->AddHeader(nums);
        // socket->Send(packet,0);

        socket->SendTo(packet,0,from);
    }
    void
    ControlApp::StartApplication(void){
        std::cout<<"Start Control App"<<std::endl;
        //report time, the interval is 1 s;
        ReportTime();
        m_sendSocket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
        m_sendSocket->Bind(m_local);
        m_sendSocket->Connect(m_peer);
        m_sendSocket->SetRecvCallback(MakeCallback (&ControlApp::HandleRead,this));

        m_receiveSocket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
        InetSocketAddress receive_local = InetSocketAddress(Ipv4Address::GetAny(),m_receivePort);
        m_receiveSocket->Bind(receive_local);
        m_receiveSocket->SetRecvCallback(MakeCallback (&ControlApp::HandleRead,this));

        for(uint32_t i = 0; i < m_apnum;i++){
            for(uint32_t j = 0; j < m_edgenum;j++){
                m_qlearns[i][j]->get_env_feedback(0,0);
            }
            std::cout<<"send init control to Sense "<<i<<std::endl;
            SendControl(m_sendSocket,i,InetSocketAddress(m_children.GetAddress(i),m_peerPort));
        } 
    }
    void
    ControlApp::StopApplication(void) {

    }
    void ControlApp::ReportTime(){
        NS_LOG_INFO("Now,the simulator time is " << Simulator::Now().GetSeconds());
        Time tNext (Seconds (1));
        m_reportTimeEvent = Simulator::Schedule(tNext,&ControlApp::ReportTime,this);
    }
    void
    ControlApp::HandleRead(Ptr<Socket> socket)
    {
        Ptr<Packet> packet;
        Address from;
        Address localAddress;
        while((packet = socket->RecvFrom(from)))
        {  
            if(packet->GetSize() > 0){
                EdgeTag tag;
                packet->PeekPacketTag(tag);
                if(tag.GetTagValue() == 0){     //data  packet
                    
                }else if(tag.GetTagValue() == 1){   //uplink metrics packet
                    
                    SeqTsSizeHeader index;
                    packet->RemoveHeader(index);
                    uint32_t mindex  = index.GetSeq(); 
                    
                    SeqTsSizeHeader edgenum;
                    packet->RemoveHeader(edgenum);
                    m_edgenum = edgenum.GetSeq();

                    std::vector<SeqTsSizeHeader> headers(m_edgenum),headers_usingtdma(m_edgenum);
                    
                    for(uint32_t i = 0; i < m_edgenum;i++){
                        packet->RemoveHeader(headers[i]);
                        m_metrics[mindex][i] = headers[i].GetSeq(); 
                        if(m_count == 0){
                            m_lastMetrics[mindex][i] = headers[i].GetSeq();
                        }       
                    }
                    
                    for(uint32_t i = 0; i < m_edgenum;i++){
                        packet->RemoveHeader(headers_usingtdma[i]);
                        m_states[mindex][i] = headers_usingtdma[i].GetSeq();        
                    }
                    
                    for(uint32_t i = 0; i < m_edgenum;i++){
                        if(m_WeekUp[mindex][i] == 1){
                            double reward = (m_lastMetrics[mindex][i] - m_metrics[mindex][i]) / (double) m_lastMetrics[mindex][i];
                            m_qlearns[mindex][i]->get_env_feedback(m_states[mindex][i],reward);
                            m_qlearns[mindex][i]->UpdateQtable();
                            m_qlearns[mindex][i]->changeState();   
                        }
                    }
                    m_count++;
                    // NS_LOG_INFO("ControlApp:: send to from " << InetSocketAddress::ConvertFrom(from).GetIpv4() << " port: " << InetSocketAddress::ConvertFrom(from).GetPort());
                    SendControl(m_sendSocket,mindex,from);
                }else if(tag.GetTagValue() == 2){   //download control packet

                }else if(tag.GetTagValue() == 3){   //download metrics packet

                }else if(tag.GetTagValue() == 4){   //report children nodes is weekup or sleep
                    SeqTsSizeHeader index;
                    packet->RemoveHeader(index);
                    uint32_t mindex = index.GetSeq();

                    SeqTsSizeHeader edgeIndex;
                    packet->RemoveHeader(edgeIndex);
                    uint32_t edge_index = edgeIndex.GetSeq();

                    SeqTsSizeHeader State;
                    packet->RemoveHeader(State);
                    uint32_t state = State.GetSeq();

                    m_WeekUp[mindex][edge_index] = state; 
                    // NS_LOG_INFO("ControlApp::updata child node: Ap node " << mindex<<" edge index" << edge_index << " state " <<state);
                    if(state == 1){
                        Ptr<Packet> packet = Create<Packet>(0);
                        EdgeTag tag;
                        tag.SetTagValue(5);
                        packet->AddPacketTag(tag);
                        uint32_t action = m_qlearns[mindex][edge_index]->choose_action_random();
                        // NS_LOG_INFO("Control Node realize ap node " << mindex <<" edge node " <<edge_index <<" restart"
                        //             <<" and decide to choose" << action <<" .(0:tdma,1:csma)");
                        SeqTsSizeHeader Index;
                        Index.SetSeq(edge_index);
                        SeqTsSizeHeader header;
                        header.SetSeq(action);
                        

                        packet->AddHeader(header);
                        packet->AddHeader(Index);
                        socket->SendTo(packet,0,from);
                    }
                }
            }
        }
        
    }
    
}
