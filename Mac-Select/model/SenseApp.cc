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
            .AddAttribute ("Port","port",
                        UintegerValue(0),
                        MakeUintegerAccessor(&SenseApp::m_port),
                        MakeUintegerChecker<uint16_t>())
            // .AddAttribute ("Remote", "The address of the destination",
            //             AddressValue (),
            //             MakeAddressAccessor (&SenseApp::m_peer),
            //             MakeAddressChecker ()) 
        ;
        return tid;
    }

    SenseApp::SenseApp()
        : m_socket(),
        m_childs(),
        m_fatherControlL(),
        m_fatherControl(),
        m_local(),
        m_sendEvent(),
        m_updateEvent(),
        m_temp(),
        m_metrics(),
        m_usingtdmas(),
        m_rti(5),
        m_period(10),        // report period
        m_report(true),   //is report
        m_childNum(0),    //child nums 
        m_info(0),
        m_upTime(0),
        m_updates(),         //all node metrics is update?
        m_tolerate(100),
        m_index(0),
        m_actions(),
        m_port(),
        m_peer()
    {

    }
    SenseApp::~SenseApp(){

    }
    void
    SenseApp::Setup(Ptr<Socket> socket,Address peer,Ipv4InterfaceContainer childrenAddress,Address local,Address fatherControlL,Address fatherControl,uint32_t childnum,uint32_t index){
        m_local = local;
        m_socket = socket;
        m_childs = childrenAddress;
        m_childNum = childnum;
        m_updates.resize(childnum);
        for(uint32_t i; i < childnum ;i++){
            m_updates[i] = false;
        }
        m_fatherControl = fatherControl;
        m_fatherControlL = fatherControlL;
        m_index = index;
        m_actions.resize(m_childNum,0);
        m_usingtdmas.resize(m_childNum,false);
        m_metrics.resize(m_childNum,0);
        m_peer = peer;
    };

    void
    SenseApp::StartApplication(void){
        // InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny(),m_port);
        m_socket->Bind(m_local);
        m_socket->Connect(m_peer);
        m_socket->SetRecvCallback(MakeCallback (&SenseApp::HandleRead,this));
        std::cout<<"start SenseApp"<<std::endl;
        // Update();
        // SendMetrics();
    }

    void
    SenseApp::StopApplication(void){

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
        std::cout<<"reveive packet"<<std::endl;
        while((packet = socket->RecvFrom(from)))
        {   
            NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server received " << packet->GetSize () << " bytes from " <<
                       InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                       InetSocketAddress::ConvertFrom (from).GetPort ());
            if(packet->GetSize() > 0)
            {
                EdgeTag tag;
                packet->PeekPacketTag(tag);
                if(tag.GetTagValue() == 0) // data packet
                {
                    TimeHeader header;
                    packet->RemoveHeader(header);
                
                    Time time(Simulator::Now());
                    m_upTime = time.GetMilliSeconds()-header.GetData();

                    Ptr<Packet> pac = Create<Packet> (0);
                    EdgeTag tag;
                    tag.SetTagValue(0);
                    pac->AddPacketTag(tag);
                    SeqTsSizeHeader header1;
                    header1.SetSeq(m_upTime);
                    TimeHeader header2;
                    header2.SetData(Simulator::Now().GetMilliSeconds());
                    pac->AddHeader(header1);
                    pac->AddHeader(header2);
                    socket->SendTo(pac,0,from);
                }else if(tag.GetTagValue() == 1) //uplink metrics packet
                {
                   
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
                    
                    SeqTsSizeHeader nums;
                    packet->RemoveHeader(nums);
                    uint32_t m_nums= nums.GetSeq();
                    std::vector<SeqTsSizeHeader> headers(m_nums);
                    for(uint32_t i =0;i < m_nums;i++){
                        packet->RemoveHeader(headers[i]);
                        m_actions[i] = headers[i].GetSeq();
                        SendToChild(m_socket,i,m_childs.GetAddress(i),2);
                    }            
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
            if(m_updates[i] == false)
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
        std::cout<<"wait packet"<<std::endl;
        if(collectAll()){
            std::cout<<"SenseApp Collect All"<<std::endl;
            if(m_temp.IsRunning())
                Simulator::Cancel(m_temp);
            Ptr<Packet> pac = Create<Packet>(0);
            EdgeTag tag;
            tag.SetTagValue(1);
            pac->AddPacketTag(tag);
            SeqTsSizeHeader index;
            index.SetSeq(m_index);
            pac->AddHeader(index);
            SeqTsSizeHeader nums;
            nums.SetSeq(m_childNum);
            pac->AddHeader(nums);
            std::vector<SeqTsSizeHeader> headers(m_childNum);
            std::vector<SeqTsSizeHeader> headers_usingtdma(m_childNum);
            for(uint32_t i = 0;i < m_childNum;i++){
                headers[i].SetSeq(m_metrics[i]);  
                pac->AddHeader(headers[i]);
            }
            for(uint32_t i = 0;i < m_childNum;i++){
                headers_usingtdma[i].SetSeq(m_usingtdmas[i]);
                pac->AddHeader(headers_usingtdma[i]);
            }
            m_socket->SendTo(pac,0,m_fatherControl);
            ResetUpdate();
            ScheduleSend();
        }else{
            std::cout<<"SenseApp next sendmetrics"<<std::endl;
            Time tNext(MilliSeconds (m_tolerate));
            m_temp = Simulator::Schedule(tNext,&SenseApp::SendMetrics,this);
        }
    }
	
}
