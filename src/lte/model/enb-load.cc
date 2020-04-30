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
//std::map<uint16_t, Ptr<LteUeNetDevice>> ueNetDeviceMap;

NetDeviceContainer enbLteDevs;

// void initUeDevices(NetDeviceContainer newDevs) {

//     uint32_t nDevices = newDevs.GetN();
//     for (uint16_t i = 0; i < nDevices; i++) {

//         Ptr<NetDevice> tempNetDevice = newDevs.Get(i);
//         Ptr<LteUeNetDevice> ueNetDevice = tempNetDevice->GetObject<LteUeNetDevice>();
//         uint16_t rnti = ueNetDevice->GetRrc()->GetRnti();

//         NS_LOG_UNCOND("Inserting " << rnti << " to ueMap");
//         ueNetDeviceMap.insert(std::pair<uint16_t, Ptr<LteUeNetDevice>>(rnti, ueNetDevice));
//     }
// }

void initEnbDevices(NetDeviceContainer newDevs) {
    enbLteDevs = newDevs;
}

uint8_t counter = 0;

void updateLoad(uint16_t cellId, int rbAllocated, uint16_t totalRb) {
    float load = ((float) rbAllocated) / (float) totalRb;

    loadMap.insert(std::pair<uint16_t, float>(cellId, load));
    if (counter > 10) {
        loadBalancingAlgorithm();
        counter = 0;
    } else {
        counter++;
        
    }
    
    NS_LOG_UNCOND("Updating load to [" << std::to_string(load) << "] id " << cellId << " ||| " << rbAllocated << " kk " << totalRb);
}

float getLoad(uint16_t cellId) {
    std::map<uint16_t, float>::iterator it;

    it = loadMap.find(cellId);
    if (it != loadMap.end()) {
        return it->second;
    }
    return 0;
}

float * calculateNewOffsets(uint16_t cellId, uint16_t neighborCellId) {
    static float newOffsets[2]; //CIO 21, CIO 12
    float offset; //cio12
    float minOffset = getMinOffset(cellId, neighborCellId) 
    float load = getLoad(cellId); 
    float neighborLoad = getLoad(neighborCellId); 
    // Updating cell offsets
    float delta = (offset-minOffset)*(1-neighborLoad/load);
    newOffsets[0] = offset - delta;

    float hysteresis;
    float neighborHysteresis;
    newOffsets[1] = hysteresis + neighborHysteresis - newOffsets[0];

    return newOffsets;
}

float getOffset(uint16_t cellId, uint16_t neighborCellId) {
    // get cell offsets
    return 0.0;
}

float getMinOffset(uint16_t cellId, uint16_t neighborCellId) {
    // get cell offsets
    float m_th; //-107
    float neighborHysteresis;
    float m_1max; //~-111
    return 0.0;
}

void loadBalancingAlgorithm() {
    NS_LOG_UNCOND("Load balancing Iteration Begin");

    uint32_t nDevices = enbLteDevs.GetN();
    for (uint16_t i = 0; i < nDevices; i++) {

        //NS_LOG_UNCOND("Load balancing Iteration 1");
        Ptr<NetDevice> tempNetDevice = enbLteDevs.Get(i);
        Ptr<LteEnbNetDevice> enbNetDevice = tempNetDevice->GetObject<LteEnbNetDevice>();

        Ptr<LteEnbRrc> rrc = enbNetDevice->GetRrc();

        
        std::map<uint16_t, Ptr<UeManager>> *ueManagerMap;
        rrc->getUeMap(&ueManagerMap);

        //NS_LOG_UNCOND("Load balancing Iteration 2");        

        for (std::map<uint16_t, Ptr<UeManager>>::iterator it = ueManagerMap->begin(); it != ueManagerMap->end();
             ++it) {

            

            // UE Cell Id
            //NS_LOG_UNCOND("Load balancing Iteration 3");
            uint16_t cellId = rrc->ComponentCarrierToCellId(it->second->GetComponentCarrierId());
            float cellLoad = getLoad(cellId);

            NS_LOG_UNCOND("Cell ID: " << cellId <<  " load is " << std::to_string(cellLoad));

            PointerValue ptr;
            Ptr<UeManager> tempUeManager = it->second;


            LteRrcSap::MeasurementReport savedMessage = tempUeManager->savedMessage;

            
            //NS_LOG_UNCOND("Load balance Iteration 4");
            for (std::list <LteRrcSap::MeasResultEutra>::iterator it = savedMessage.measResults.measResultListEutra.begin (); it != savedMessage.measResults.measResultListEutra.end (); ++it)
            {
                // NS_LOG_UNCOND("Measurement report looping");
                uint16_t possibleCellId = it->physCellId;
                if (cellId == possibleCellId) {
                    continue;
                }

                float curLoad = getLoad(possibleCellId);
                NS_LOG_UNCOND("Cell ID:" << possibleCellId <<  " load is " << std::to_string(curLoad));
                

                float loadGap = curLoad - cellLoad;
                NS_LOG_UNCOND("Load Gap: [" << loadGap << "] between cell: [" << cellId << "] and cell: " << possibleCellId);
                // NS_LOG_UNCOND ("neighbour cellId " << it->physCellId
                //                                 << " RSRP " << (it->haveRsrpResult ? (uint16_t) it->rsrpResult : 255)
                //                                 << " RSRQ " << (it->haveRsrqResult ? (uint16_t) it->rsrqResult : 255));

                
            }
        }

    }
    NS_LOG_UNCOND("Load balancing Iteration End");
}
