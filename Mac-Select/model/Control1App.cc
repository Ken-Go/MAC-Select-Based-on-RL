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
            .AddAttribute ("Port","port",
                        UintegerValue(0),
                        MakeUintegerAccessor(&Control1App::m_port),
                        MakeUintegerChecker<uint16_t>())
        ;
        return tid;
    }
    Control1App::Control1App()
        : m_periods(),
        m_reports(),
        m_rti(),
        m_sockets(),
        m_local(),
        m_childs(),
        m_childnum(0),
        m_port()
    {
        
    }

    Control1App::~Control1App(){

    }
    void 
    Control1App::Setup(Ptr<Socket> sockets,Ipv4InterfaceContainer children,Address local,uint32_t childnum){
        m_sockets = sockets;
        m_childs = children;
        m_local = local;
        m_childnum = childnum;
        m_periods = 20;
        m_reports.resize(m_childnum,true);
        m_rti.resize(m_childnum,5);
    }

    void
    Control1App::StartApplication(void){
        std::cout<<"Start Control Left"<<std::endl;
        if(!m_local.IsInvalid()){
            m_sockets->Bind(m_local);
        }else{
            m_sockets->Bind();
        }

        broadcast();
        m_sockets->SetRecvCallback(MakeCallback(&Control1App::HandleRead,this));
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
        packet->AddHeader(rti);
        packet->AddHeader(report);
        m_sockets->SendTo(packet,0,m_childs.GetAddress(addindex));
    }
}
