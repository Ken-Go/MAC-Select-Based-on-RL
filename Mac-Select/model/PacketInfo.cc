#include "PacketInfo.h"
namespace ns3{
    NS_LOG_COMPONENT_DEFINE ("PacketInfo");

PacketInfo::PacketInfo(){
    m_seq = 0;
    m_macProtocol = "csma";
    m_sendTime = 0;
    m_receiveTime = 0;
    m_length = 1024;
    m_rtt = 0;
};
void PacketInfo::Set(uint32_t seq,std::string macProtocol,double sendTime,double receiveTime,Address source,Address destination,uint32_t length,double rtt){
    m_seq = seq;
    m_macProtocol = macProtocol;
    m_sendTime = sendTime;
    m_receiveTime = receiveTime;
    m_source = source;
    m_destination = destination;
    m_length = length;
    m_rtt = rtt;
};

void PacketInfo::SetSeq(uint32_t seq){
    m_seq = seq;
};
void PacketInfo::SetMacProtocol(std::string macProtocol){
    m_macProtocol = macProtocol;
};
void PacketInfo::SetSendTime(double sendTime){
    m_sendTime = sendTime;
};
void PacketInfo::SetReceiveTime(double receiveTime){
    m_receiveTime = receiveTime;
};
void PacketInfo::SetSource(Address source){
    m_source = source;
};
void PacketInfo::SetDestination(Address destination){
    m_destination = destination;
};
void PacketInfo::SetLength(uint32_t length){
    m_length = length;
};
void PacketInfo::SetRtt(double rtt){
    m_rtt = rtt;
};
uint32_t PacketInfo::getSeq(){
    return m_seq;
};
std::string PacketInfo::getMacProtocol(){
    return m_macProtocol;
};
double PacketInfo::getSendTime(){
    return m_sendTime;
};
double PacketInfo::getReceiveTime(){
    return m_receiveTime;
};
Address PacketInfo::getSource(){
    return m_source;
};
Address PacketInfo::getDestination(){
    return m_destination;
};
uint32_t PacketInfo::getLength(){
    return m_length;
};
double PacketInfo::getRtt(){
    return m_rtt;
};
void PacketInfo::writeOut(std::ofstream out){
    std::stringstream ss;
    ss<<m_seq<<"\t"<<m_macProtocol<<"\t"<<m_sendTime<<"\t"<<m_receiveTime<<"\t"<<InetSocketAddress::ConvertFrom(m_source).GetIpv4()<<"\t"<<InetSocketAddress::ConvertFrom(m_destination).GetIpv4()<<"\t"<<m_length<<"\t"<<m_rtt<<"\n";
    out<<ss.str();
}
}