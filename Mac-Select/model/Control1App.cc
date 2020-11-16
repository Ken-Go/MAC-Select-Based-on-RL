#include "Control1App.h"

namespace ns3 {
    NS_LOG_COMPONENT_DEFINE ("Control1App");
    NS_OBJECT_ENSURE_REGISTERED(Control1App);
    TypeId
    Control1App::GetTypeId(void){
        static TypeId tid = TypeId("ns::Control1App")
            .SetParent<Application>()
            .SetGroupName("Application")
            .AddConstructor<Control1App>()
            .AddAttribute("Local",
                        "local address",
                        AddressValue(),
                        MakeAddressAccessor(&Control1App::m_local),
                        MakeAddressChecker())
            .AddAttribute ("sendPort","send packet from port",
                        UintegerValue(0),
                        MakeUintegerAccessor(&Control1App::m_sendPort),
                        MakeUintegerChecker<uint32_t>())
            .AddAttribute ("receivePort","receive packet from this port",
                        UintegerValue(0),
                        MakeUintegerAccessor(&Control1App::m_receivePort),
                        MakeUintegerChecker<uint32_t>())
        ;
        return tid;
    }
    Control1App::Control1App()
        : m_periods(),
        m_reports(),
        m_rti(),
        m_sendSocket(),
        m_receiveSocket(),
        m_local(),
        m_sendPort(),
        m_receivePort(),
        m_peer(),
        m_peerPort(),
        m_children(),
        m_childnum(0),
        m_periodBroad()
    {
        
    }

    Control1App::~Control1App(){

    }
    void 
    Control1App::Setup(uint32_t childnum,Address local,uint32_t sendPort,uint32_t receivePort,Address peer,uint32_t peerPort,Ipv4InterfaceContainer children){
        m_childnum = childnum;
        m_children = children;
        m_local = local;
        
        m_periods = 20;
        m_reports.resize(m_childnum,true);
        m_rti.resize(m_childnum,5);
    }

    void
    Control1App::StartApplication(void){
        std::cout<<"Start Control Left"<<std::endl;
        
        m_sendSocket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
        m_sendSocket->Bind(m_local);
        m_sendSocket->Connect(m_peer);
        m_sendSocket->SetRecvCallback(MakeCallback (&Control1App::HandleRead,this));

        m_receiveSocket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
        InetSocketAddress receive_local = InetSocketAddress(Ipv4Address::GetAny(),m_receivePort);
        m_receiveSocket->Bind(receive_local);
        m_receiveSocket->SetRecvCallback(MakeCallback(&Control1App::HandleRead,this));
        // Ptr <ns3::Ipv4> ipv4 = GetNode()->GetObject <ns3::Ipv4> ();
        // for(uint32_t i = 0; i < GetNode()->GetNDevices();i++){
        //     ipv4->GetRoutingProtocol ()->NotifyInterfaceUp(ipv4->GetInterfaceForDevice(GetNode()->GetDevice(i)));
        // }  
        // broadcast();
        
    }
    void
    Control1App::StopApplication(void){}

    void
    Control1App::HandleRead(Ptr<Socket> socket)
    {
        Ptr<Packet> packet;
        Address from;
        Address localAddress;
        while((packet = socket->RecvFrom(from))){
            EdgeTag tag;
            packet->PeekPacketTag(tag);
            if(tag.GetTagValue() == 0) //data packet
            {
                SeqTsSizeHeader index;
                packet->RemoveHeader(index);
                SendControl(index.GetSeq());
            }else if (tag.GetTagValue() == 1) //up metrics packet
            {
            }else if (tag.GetTagValue() == 2)   //download control packet
            {
            }else if (tag.GetTagValue() == 3)   //download metrics packet
            {
            }
        }
    }
    void
    Control1App::ScheduleBroadcast(){
        Time tNext (Seconds (m_periods));
        m_periodBroad = Simulator::Schedule(tNext,&Control1App::broadcast,this);
    }
    void 
    Control1App::broadcast()
    {
        std::cout<<"Start Broadcast"<<std::endl;
        for(uint32_t i =0; i < m_childnum;i++){
            SendControl(i);
        }
        ScheduleBroadcast();
    }
    void 
    Control1App::SendControl(uint32_t addindex)
    {
        Ptr<Packet> packet = Create<Packet>(0);
        EdgeTag tag;
        tag.SetTagValue(2);
        packet->AddPacketTag(tag);

        SeqTsSizeHeader rti;
        rti.SetSeq(m_rti[addindex]);
        
        SeqTsSizeHeader report;
        if(m_reports[addindex] == true)
            report.SetSeq(1);
        else
        {
            report.SetSeq(0);
        }
        packet->AddHeader(report);
        packet->AddHeader(rti);
        m_sendSocket->SendTo(packet,0,InetSocketAddress(m_children.GetAddress(addindex),m_peerPort));
    }
}
