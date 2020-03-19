#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-monitor.h"
#include "ns3/internet-module.h"
#include "ns3/lte-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-epc-helper.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/position-allocator.h"
#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LTEUdpEchoExample");

int main(int argc, char *argv[]) {
    NS_LOG_INFO("Create nodes.");

    // remote host node
    NodeContainer remoteNodeContainer;
    remoteNodeContainer.Create(2);

    // create lte helper
    Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();

    // create epc helper
    Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper>();
    lteHelper->SetEpcHelper(epcHelper);

    // impl path loss model
    lteHelper->SetAttribute("PathlossModel", StringValue("ns3::FriisPropagationLossModel"));

    // Get P-GW from EPC Helper
    Ptr<Node> pgw = epcHelper->GetPgwNode();

    InternetStackHelper internet;
    internet.Install(remoteNodeContainer);

    NS_LOG_INFO("Create channels.");

    PointToPointHelper p2ph;
    p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate("1Gb/s")));
    p2ph.SetDeviceAttribute("Mtu",
                            UintegerValue(30000)); // jumbo frames here as well
    // p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));

    NetDeviceContainer internetDevices =
        p2ph.Install(pgw, remoteNodeContainer.Get(0)); // [P-GW] ------- [internet] ------ [remote host node]
    Ipv4AddressHelper ipv4h;
    ipv4h.SetBase("1.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign(internetDevices);

    Ipv4Address remoteHostAddr;

    remoteHostAddr = internetIpIfaces.GetAddress(1); // remote host ip adr

    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> remoteHostStaticRouting =
        ipv4RoutingHelper.GetStaticRouting(remoteNodeContainer.Get(1)->GetObject<Ipv4>());

    // hardcoded UE addresses for now
    remoteHostStaticRouting->AddNetworkRouteTo(Ipv4Address("7.0.0.0"), Ipv4Mask("255.255.255.0"), 1);

    // create enb

    NodeContainer enbs;
    enbs.Create(1);

    Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator>();
    enbPositionAlloc->Add(Vector(0.0, 0.0, 0.0));
    enbPositionAlloc->Add(Vector(100, 0.0, 0.0));

    MobilityHelper enbMobility;
    enbMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    enbMobility.SetPositionAllocator(enbPositionAlloc);

    enbMobility.Install(enbs);
    NetDeviceContainer enbLteDev = lteHelper->InstallEnbDevice(enbs);

    // ues client
    NodeContainer ues;
    ues.Create(1);

    MobilityHelper ueMobility;

    ueMobility.SetPositionAllocator("ns3::UniformDiscPositionAllocator", "X", DoubleValue(0), "Y",
                                    DoubleValue(0), "rho", DoubleValue(100.0));
    ueMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    ueMobility.Install(ues);
    NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice(ues);

    InternetStackHelper internet2;
    internet2.Install(ues);

    Ptr<Node> ue = ues.Get(0);
    Ptr<NetDevice> ueLteDevice = ueLteDevs.Get(0);
    Ipv4InterfaceContainer ueIpIface = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueLteDevice));

    // set the default gateway for the UE
    Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting(ue->GetObject<Ipv4>());
    ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(), 1);

    // we can now attach the UE, which will also activate the default EPS bearer
    lteHelper->Attach(ueLteDevice);

    //////// network setting clear;

    // server setting

    uint16_t port = 9; // well-known echo port number
    UdpEchoServerHelper server(port);
    ApplicationContainer apps = server.Install(remoteNodeContainer.Get(0));
    apps.Start(Seconds(1.0));
    apps.Stop(Seconds(10.0));

    //
    // Create a UdpEchoClient application to send UDP datagrams from node zero to
    // node one.
    //

    uint32_t packetSize = 2048;
    uint32_t maxPacketCount = 50;
    Time interPacketInterval = Seconds(0.2);
    UdpEchoClientHelper client(remoteHostAddr, port);
    client.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
    client.SetAttribute("Interval", TimeValue(interPacketInterval));
    client.SetAttribute("PacketSize", UintegerValue(packetSize));

    apps = client.Install(ues.Get(0));
    apps.Start(Seconds(2.0));
    apps.Stop(Seconds(10.0));

    // client.SetFill (apps.Get(0), 0xff, 1024 * 10);

    // AsciiTraceHelper ascii;
    // p2ph.EnableAsciiAll (ascii.CreateFileStream ("lte-udp-echo.tr"));
    // p2ph.EnablePcapAll ("lte-udp-echo", false);

    Ptr<FlowMonitor> flowmon;
    FlowMonitorHelper flowmonHelper;
    flowmon = flowmonHelper.InstallAll();

    NS_LOG_INFO("Run Simulation.");

    Simulator::Stop(Seconds(10.0));
    Simulator::Run();

    flowmon->SerializeToXmlFile("lte-udp-echo.flowmon", false, false);

    Simulator::Destroy();
    NS_LOG_INFO("Done.");
}