#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <vector>

class SenseApp : public UdpServer{
public:
void SenseEnv(){
}
void CalculateDelay(){

}
};

class ControlApp : public UdpServer{
public:
    vector<vector<double> > q_table;
void Q-Learing(){

} 

};
class EdgeApp : public OnOffApplication{
public:


};

