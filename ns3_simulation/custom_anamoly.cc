#include "ns3/applications-module.h"
#include "ns3/command-line.h"
#include "ns3/config-store-module.h"
#include "ns3/internet-module.h"
#include "ns3/mmwave-helper.h"
#include "ns3/mmwave-point-to-point-epc-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/log.h"

using namespace ns3;
using namespace mmwave;

NS_LOG_COMPONENT_DEFINE("ColoredMmWaveSimulation");

int main(int argc, char* argv[])
{
    // Increase simulation time to 10 seconds for better flow capture.
    double simTime = 15.0;
    double interPacketInterval = 100; // microseconds

    CommandLine cmd;
    cmd.Parse(argc, argv);

    // Set defaults for mmWave simulation configuration.
    Config::SetDefault("ns3::MmWaveHelper::RlcAmEnabled", BooleanValue(false));
    Config::SetDefault("ns3::MmWaveHelper::HarqEnabled", BooleanValue(true));
    Config::SetDefault("ns3::MmWaveFlexTtiMacScheduler::HarqEnabled", BooleanValue(true));

// Enable logging for UDP applications (optional; can help verify traffic)
  LogComponentEnable("UdpClient", LOG_LEVEL_INFO);
  LogComponentEnable("UdpServer", LOG_LEVEL_INFO);

    Ptr<MmWaveHelper> mmwaveHelper = CreateObject<MmWaveHelper>();
    mmwaveHelper->SetSchedulerType("ns3::MmWaveFlexTtiMacScheduler");

    Ptr<MmWavePointToPointEpcHelper> epcHelper = CreateObject<MmWavePointToPointEpcHelper>();
    mmwaveHelper->SetEpcHelper(epcHelper);
    mmwaveHelper->SetHarqEnabled(true);

    Ptr<Node> pgw = epcHelper->GetPgwNode();

    // Remote Host
    NodeContainer remoteHostContainer;
    remoteHostContainer.Create(1);
    Ptr<Node> remoteHost = remoteHostContainer.Get(0);
    InternetStackHelper internet;
    internet.Install(remoteHostContainer);

    PointToPointHelper p2ph;
    p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100Gb/s")));
    p2ph.SetDeviceAttribute("Mtu", UintegerValue(1500));
    p2ph.SetChannelAttribute("Delay", TimeValue(Seconds(0.010)));

    NetDeviceContainer internetDevices = p2ph.Install(pgw, remoteHost);
    Ipv4AddressHelper ipv4h;
    ipv4h.SetBase("1.0.0.0", "255.0.0.0");
    Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign(internetDevices);
    Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress(1);

    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> remoteHostStaticRouting =
        ipv4RoutingHelper.GetStaticRouting(remoteHost->GetObject<Ipv4>());
    remoteHostStaticRouting->AddNetworkRouteTo(Ipv4Address("7.0.0.0"), Ipv4Mask("255.0.0.0"), 1);

    // Create eNB and UE nodes.
    NodeContainer enbNodes, ueNodes;
    enbNodes.Create(2); // eNB0 (Node 0), eNB1 (Node 1)
    ueNodes.Create(2);  // UE3 (Node 2), UE4 (Node 3)
    Ptr<Node> ue3 = ueNodes.Get(0);
    Ptr<Node> ue4 = ueNodes.Get(1);

    // Combine all nodes for NetAnim visualization.
    NodeContainer allNodes;
    allNodes.Add(enbNodes);
    allNodes.Add(ueNodes);
    allNodes.Add(remoteHost); // Node 4

    // Set fixed positions for NetAnim.
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> posAlloc = CreateObject<ListPositionAllocator>();
    posAlloc->Add(Vector(0.0, 0.0, 0.0));     // eNB0 position
    posAlloc->Add(Vector(75.0, 0.0, 0.0));    // eNB1 position
    posAlloc->Add(Vector(30.0, 30.0, 0.0));   // UE3 position
    posAlloc->Add(Vector(60.0, -30.0, 0.0));  // UE4 position
    posAlloc->Add(Vector(150.0, 0.0, 0.0));   // RemoteHost position
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.SetPositionAllocator(posAlloc);
    mobility.Install(allNodes);

    // Install mmWave devices.
    NetDeviceContainer enbDevs = mmwaveHelper->InstallEnbDevice(enbNodes);
    NetDeviceContainer ueDevs = mmwaveHelper->InstallUeDevice(ueNodes);
    internet.Install(ueNodes);

    Ipv4InterfaceContainer ueIpIface = epcHelper->AssignUeIpv4Address(ueDevs);

    for (uint32_t i = 0; i < ueNodes.GetN(); ++i)
    {
        Ptr<Ipv4StaticRouting> ueStaticRouting =
            ipv4RoutingHelper.GetStaticRouting(ueNodes.Get(i)->GetObject<Ipv4>());
        ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(), 1);
    }

    // Attach UEs to the closest eNB.
    mmwaveHelper->AttachToClosestEnb(ueDevs, enbDevs);

    // Traffic: UE4 sends UDP packets to UE3.
    uint16_t port = 4000;
    PacketSinkHelper sink("ns3::UdpSocketFactory",
                          InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer serverApps = sink.Install(ue3);

    UdpClientHelper client(ueIpIface.GetAddress(0), port);
    client.SetAttribute("Interval", TimeValue(MicroSeconds(interPacketInterval)));
    client.SetAttribute("MaxPackets", UintegerValue(1000000));
    ApplicationContainer clientApps = client.Install(ue4);

    // Start traffic applications after 1.0 second.
    serverApps.Start(Seconds(1.0));
    clientApps.Start(Seconds(1.0));

// Optionally, stop the applications a little before simulation end (optional; not required)
  serverApps.Stop(Seconds(simTime - 1.0));
  clientApps.Stop(Seconds(simTime - 1.0));


    // NetAnim configuration.
    AnimationInterface anim("anamolynetwork.xml");

    // Set node descriptions.
    anim.UpdateNodeDescription(0, "eNB0");
    anim.UpdateNodeDescription(1, "eNB1");
    anim.UpdateNodeDescription(2, "UE3");
    anim.UpdateNodeDescription(3, "UE4");
    anim.UpdateNodeDescription(4, "RemoteHost");

    // Set node colors.
    anim.UpdateNodeColor(0, 255, 0, 0);     // Red for eNB0
    anim.UpdateNodeColor(1, 255, 0, 0);     // Red for eNB1
    anim.UpdateNodeColor(2, 0, 255, 0);     // Green for UE3
    anim.UpdateNodeColor(3, 0, 255, 0);     // Green for UE4
    anim.UpdateNodeColor(4, 0, 0, 255);     // Blue for RemoteHost

    // Set explicit link descriptions for better visualization.
    anim.UpdateLinkDescription(3, 2, "UE4 → UE3");
    anim.UpdateLinkDescription(0, 2, "eNB0 ↔ UE3");
    anim.UpdateLinkDescription(1, 3, "eNB1 ↔ UE4");
    anim.UpdateLinkDescription(0, 4, "eNB0 ↔ RemoteHost");
    anim.UpdateLinkDescription(1, 4, "eNB1 ↔ RemoteHost");

    // Install FlowMonitor to capture flows.
    FlowMonitorHelper flowmonHelper;
    Ptr<FlowMonitor> monitor = flowmonHelper.InstallAll();

Simulator::Stop(Seconds(simTime));
  Simulator::Run();



    // Save FlowMonitor output after simulation ends.
    flowmonHelper.SerializeToXmlFile("flowmon_results.xml", true, true);

    Simulator::Destroy();

    return 0;
}