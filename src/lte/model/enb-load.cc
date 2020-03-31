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

std::map<uint16_t, float> loadMap;
std::map<uint16_t, Ptr<LteUeNetDevice>> ueNetDeviceMap;

NetDeviceContainer enbLteDevs;

void initUeDevices(NetDeviceContainer newDevs) {

    uint32_t nDevices = newDevs.GetN();
    for (uint16_t i = 0; i < nDevices; i++) {

        Ptr<NetDevice> tempNetDevice = newDevs.Get(i);
        Ptr<LteUeNetDevice> ueNetDevice = tempNetDevice->GetObject<LteUeNetDevice>();
        uint16_t rnti = ueNetDevice->GetRrc()->GetRnti();

        NS_LOG_UNCOND("Inserting " << rnti << " to ueMap");
        ueNetDeviceMap.insert(std::pair<uint16_t, Ptr<LteUeNetDevice>>(rnti, ueNetDevice));
    }
}

void initEnbDevices(NetDeviceContainer newDevs) {
    enbLteDevs = newDevs;
}

uint8_t counter = 0;

void updateLoad(uint16_t cellId, int rbAllocated, uint8_t totalRb) {
    float load = rbAllocated / totalRb;

    loadMap.insert(std::pair<uint16_t, float>(cellId, load));
    if (counter > 10) {
        loadBalancingAlgorithm();
        counter = 0;
    } else {
        counter++;
        
    }
    
    //NS_LOG_UNCOND("Updating load " << load << " id " << cellId);
}

float getLoad(uint16_t cellId) {
    std::map<uint16_t, float>::iterator it;

    it = loadMap.find(cellId);
    if (it != loadMap.end()) {
        return it->second;
    }
    return 0;
}

void loadBalancingAlgorithm() {


    uint32_t nDevices = enbLteDevs.GetN();
    for (uint16_t i = 0; i < nDevices; i++) {

        NS_LOG_UNCOND("Load balancing Iteration 1");
        Ptr<NetDevice> tempNetDevice = enbLteDevs.Get(i);
        Ptr<LteEnbNetDevice> enbNetDevice = tempNetDevice->GetObject<LteEnbNetDevice>();

        Ptr<LteEnbRrc> rrc = enbNetDevice->GetRrc();

        
        std::map<uint16_t, Ptr<UeManager>> *ueManagerMap;
        rrc->getUeMap(&ueManagerMap);

        NS_LOG_UNCOND("Load balancing Iteration 2");
        for (std::map<uint16_t, Ptr<UeManager>>::iterator it = ueManagerMap->begin(); it != ueManagerMap->end();
             ++it) {
            // UE Cell Id
            NS_LOG_UNCOND("Load balancing Iteration 3");
            uint16_t cellId = rrc->ComponentCarrierToCellId(it->second->GetComponentCarrierId());
            float cellLoad = getLoad(cellId);

            PointerValue ptr;
            Ptr<UeManager> tempUeManager = it->second;

            uint16_t rnti = tempUeManager->GetRnti();

            std::map<uint16_t, Ptr<LteUeNetDevice>>::iterator ueNetDeviceIt = ueNetDeviceMap.find(rnti);

            if (ueNetDeviceIt == ueNetDeviceMap.end()) {
                NS_LOG_UNCOND("Did not find this UE " << rnti);
                continue;
            }

            Ptr<LteUeRrc> ueRrc = ueNetDeviceIt->second->GetRrc();

            NS_LOG_UNCOND("Load balancing Iteration 4");
            std::map<uint16_t, LteUeRrc::MeasValues>::iterator cellIt;
            for (cellIt = ueRrc->m_storedMeasValues.begin(); cellIt != ueRrc->m_storedMeasValues.end();
                 cellIt++) {
  

                NS_LOG_UNCOND("Load balancing Iteration 5");
                uint16_t possibleCellId = cellIt->first;
                if (cellId == possibleCellId) {
                    continue;
                }

                float loadGap = getLoad(possibleCellId) - cellLoad;
                NS_LOG_UNCOND("Load Gap " << loadGap << " between cell:" << cellId << " and cell: " << possibleCellId);
            }
        }

    }
}