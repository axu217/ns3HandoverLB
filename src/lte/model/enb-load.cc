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
#include <string>
using namespace ns3;

std::map<uint16_t, float> loadMap;

typedef std::map<uint16_t, double> innerMap;
std::map<uint16_t, innerMap> cioMap;


NetDeviceContainer enbLteDevs;


void initEnbDevices(NetDeviceContainer newDevs) {
    enbLteDevs = newDevs;
}

uint8_t counter = 0;

void updateLoad(uint16_t cellId, int rbAllocated, uint16_t totalRb) {
    float load = ((float) rbAllocated) / (float) totalRb;

    loadMap.insert(std::pair<uint16_t, float>(cellId, load));
    if (counter > 50) {
        // loadBalancingAlgorithm();
        
        counter = 0;
    } else {
        counter++;
        
    }
    NS_LOG_UNCOND("Updating load: " << cellId << "-[" << std::to_string(load) << "] id " << " ||| " << rbAllocated << " kk " << totalRb);
    
    
}

float getLoad(uint16_t cellId) {
    std::map<uint16_t, float>::iterator it;

    it = loadMap.find(cellId);
    if (it != loadMap.end()) {
        return it->second;
    }
    return 0;
}

void setCio(uint16_t source, uint16_t target, double newCioValue) {
    if (cioMap.find(source) == cioMap.end()){
        return;
    }

    innerMap tempInnerMap = cioMap[source];

    if (tempInnerMap.find(target) == tempInnerMap.end()) {
        return;
    }

    tempInnerMap[target] = newCioValue;
}

double getCio(uint16_t source, uint16_t target) {
    
    if (cioMap.find(source) == cioMap.end()){
        return 0;
    }

    innerMap tempInnerMap = cioMap[source];

    if (tempInnerMap.find(target) == tempInnerMap.end()) {
        return 0;
    }

    return tempInnerMap[target];
}

void loadBalancingAlgorithm() {
    NS_LOG_UNCOND("Load balancing Iteration Begin");

    uint32_t nDevices = enbLteDevs.GetN();


    // For each eNodeB
    for (uint16_t i = 0; i < nDevices; i++) {

        //NS_LOG_UNCOND("Load balancing Iteration 1");
        Ptr<NetDevice> tempNetDevice = enbLteDevs.Get(i);
        Ptr<LteEnbNetDevice> enbNetDevice = tempNetDevice->GetObject<LteEnbNetDevice>();

        Ptr<LteEnbRrc> rrc = enbNetDevice->GetRrc();

        
        std::map<uint16_t, Ptr<UeManager>> *ueManagerMap;
        rrc->getUeMap(&ueManagerMap);

        //NS_LOG_UNCOND("Load balancing Iteration 2");

       
    
        
        // For each UE that belongs to outer loop's eNodeb
        for (std::map<uint16_t, Ptr<UeManager>>::iterator it = ueManagerMap->begin(); it != ueManagerMap->end();
             ++it) {

            

            // UE Cell Id
            uint16_t cellId = rrc->ComponentCarrierToCellId(it->second->GetComponentCarrierId());
            float cellLoad = getLoad(cellId);

            NS_LOG_UNCOND("Cell ID: " << cellId <<  " load is " << std::to_string(cellLoad));

            PointerValue ptr;
            Ptr<UeManager> tempUeManager = it->second;


            LteRrcSap::MeasurementReport savedMessage = tempUeManager->savedMessage;

            
            // For each neighboring cell that middle loop UE may want to switch to
            for (std::list <LteRrcSap::MeasResultEutra>::iterator it = savedMessage.measResults.measResultListEutra.begin (); it != savedMessage.measResults.measResultListEutra.end (); ++it)
            {
                // NS_LOG_UNCOND("Measurement report looping");
                uint16_t possibleCellId = it->physCellId;
                if (cellId == possibleCellId) {
                    continue;
                }

        

                float potentialTargetLoad = getLoad(possibleCellId);
                NS_LOG_UNCOND("Cell ID:" << possibleCellId <<  " load is " << std::to_string(potentialTargetLoad));

                // double sourceTargetCio = getCio(cellId, possibleCellId);



                double m1Max = -107.0;

                double loadFactor = 1 - (potentialTargetLoad / cellLoad);



                // float loadGap = potentialTargetLoad - cellLoad;
                // NS_LOG_UNCOND("Load Gap: [" << loadGap << "] between cell: [" << cellId << "] and cell: " << possibleCellId);
                // NS_LOG_UNCOND ("neighbour cellId " << it->physCellId
                //                                 << " RSRP " << (it->haveRsrpResult ? (uint16_t) it->rsrpResult : 255)
                //                                 << " RSRQ " << (it->haveRsrqResult ? (uint16_t) it->rsrqResult : 255));

                
            }
        }

    }
    NS_LOG_UNCOND("Load balancing Iteration End");
}

