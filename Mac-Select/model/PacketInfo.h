#ifndef PACKET_INFO_H
#define PACKET_INFO_H

#include "ns3/core-module.h"
#include "ns3/address.h"
#include "ns3/network-module.h"
#include <iostream>
#include <cmath>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

namespace ns3 {
class PacketInfo{
public:
    PacketInfo();
    void Set(uint32_t seq,std::string macProtocol,double sendTime,double receiveTime,Address source,Address destination,uint32_t length,double rtt);
    void SetSeq(uint32_t seq);
    void SetMacProtocol(std::string macProtocol);
    void SetSendTime(double sendTime);
    void SetReceiveTime(double receiveTime);
    void SetSource(Address source);
    void SetDestination(Address destination);
    void SetLength(uint32_t length);
    void SetRtt(double rtt);
    uint32_t getSeq();
    std::string getMacProtocol();
    double getSendTime();
    double getReceiveTime();
    Address getSource();
    Address getDestination();
    uint32_t getLength();
    double getRtt();
    void writeOut(std::ofstream out);
private:
    uint32_t m_seq;
    std::string m_macProtocol;
    double m_sendTime;
    double m_receiveTime;
    Address m_source;
    Address m_destination;
    uint32_t m_length;
    double m_rtt;

};
}
#endif