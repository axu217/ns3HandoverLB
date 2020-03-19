#include <ns3/log.h>
#include <ns3/math.h>
#include <ns3/pointer.h>
#include <cfloat>
#include <ns3/boolean.h>
#include <ns3/simulator.h>
#include <map>
#include <ns3/enb-load.h>

std::map<uint16_t, float> loadMap;

void updateLoad(uint16_t cellId, int rbAllocated, uint8_t totalRb) {
    float load = rbAllocated/totalRb;
    
    loadMap.insert(std::pair<uint16_t, float>(cellId, load));
    NS_LOG_UNCOND("Updating load " << load << " id " << cellId);
}

float getLoad(uint16_t cellId) {
    std::map<uint16_t, float>::iterator it;

    it = loadMap.find(cellId);
    if (it != loadMap.end()) {
        return it->second;
    }
    return 0;

}