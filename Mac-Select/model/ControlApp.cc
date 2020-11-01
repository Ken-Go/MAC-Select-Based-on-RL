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
            .AddAttribute ("Port","port",
                        UintegerValue(0),
                        MakeUintegerAccessor(&ControlApp::m_port),
                        MakeUintegerChecker<uint16_t>())
        ;
        return tid;
    }
    ControlApp::ControlApp()
        : m_qlearns(),
        m_metrics(),
        m_apnum(0),
        m_allnum(0), 
        m_socket(),
        m_childs(),
        m_local(),
        m_updataEvent(),
        m_count(0),
        m_port(0),
        m_peer()
    {
        
    }
    ControlApp::~ControlApp(){

    }
    void 
    ControlApp::Setup(Ptr<Socket> socket,Ipv4InterfaceContainer childs,uint32_t apnum,uint32_t edgenum,uint32_t allnum,Address local)
    {
        m_local = local;
        m_socket = socket;
        m_childs = childs;
        m_apnum = apnum;
        m_edgenum = edgenum;
        m_allnum = allnum;
        
        m_qlearns.resize(m_apnum);
        m_metrics.resize(m_apnum);
        m_states.resize(m_apnum);
        m_lastMetrics.resize(m_apnum);
        for(uint32_t i =0;i < m_apnum;i++){
            m_qlearns[i].resize(edgenum);
            m_metrics[i].resize(edgenum);
            m_states[i].resize(edgenum);
            m_lastMetrics[i].resize(edgenum);
        }
        for(uint32_t i = 0; i < m_apnum;i++){
            for(uint32_t j = 0; j < m_edgenum;j++){
                m_metrics[i][j] = 0;
                m_lastMetrics[i][j] = 0;       
                m_states[i][j] = 0;
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
        packet->AddHeader(nums);
        std::vector<SeqTsSizeHeader> headers(m_edgenum);
        for(uint32_t i; i < m_edgenum;i++){
            headers[i].SetSeq(m_qlearns[apIndex][i]->choose_action());
            packet->AddHeader(headers[i]);
        }
        socket->SendTo(packet,0,from);
    }
    void
    ControlApp::StartApplication(void){
        std::cout<<"Start Control App"<<std::endl;
        InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny(),m_port);
        m_socket->Bind(local);
        m_socket->Connect(InetSocketAddress(m_childs.GetAddress(0),m_port));
        m_socket->SetRecvCallback(MakeCallback (&ControlApp::HandleRead,this));
        
        for(uint32_t i = 0; i < m_apnum;i++){
            for(uint32_t j = 0; j < m_edgenum;j++){
                m_qlearns[i][j]->get_env_feedback(0,0);
            }
            std::cout<<"send init control to Sense"<<m_childs.GetN()<<std::endl;
            SendControl(m_socket,i,m_childs.GetAddress(i));
        }
        std::cout<<"finish"<<std::endl;
    }
    void
    ControlApp::StopApplication(void) {

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
                    std::cout<<"Control receive Tag"<<tag.GetTagValue()<<std::endl;
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
                        double reward = (m_lastMetrics[mindex][i] - m_metrics[mindex][i]) / (double) m_lastMetrics[mindex][i];
                        m_qlearns[mindex][i]->get_env_feedback(m_states[mindex][i],
                                                                reward);
                        m_qlearns[mindex][i]->UpdateQtable();
                        m_qlearns[mindex][i]->changeState();    
                    }
                    m_count++;
                    SendControl(socket,mindex,from);
                }else if(tag.GetTagValue() == 2){   //download control packet

                }else if(tag.GetTagValue() == 3){   //download metrics packet

                }
            }
        }
    }

}
