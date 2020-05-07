#if !defined(enbLoad)
#define enbLoad

#include <cfloat>
#include <map>
#include <ns3/boolean.h>
#include <ns3/log.h>
#include <ns3/math.h>
#include <ns3/pointer.h>
#include <ns3/simulator.h>
#include <ns3/config.h>
#include <ns3/simulator.h>
#include <ns3/names.h>
#include <ns3/net-device.h>
#include <ns3/net-device-container.h>
#include "ns3/core-module.h"
#include "ns3/lte-module.h"
#include <ns3/enb-load.h>
#include <ns3/lte-enb-component-carrier-manager.h>
#include <ns3/lte-ue-rrc.h>
#include <ns3/component-carrier-enb.h>
#include <ns3/lte-enb-rrc.h>
#include <ns3/lte-ue-net-device.h>
#include <ns3/lte-ue-net-device.h>
#include <ns3/ptr.h>


using namespace ns3;

//void initUeDevices(NetDeviceContainer newDevs);
void initEnbDevices(NetDeviceContainer newDevs);

void updateLoad(uint16_t cellId, int rbAllocated, uint16_t totalRb);

float getLoad(uint16_t cellId);
double getCio(uint16_t source, uint16_t target);

void loadBalancingAlgorithm();

#endif // enbLoad
