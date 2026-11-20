#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"
#include "ns3/mobility-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("PartESimulation");

// Function to shut down the primary link (R1-R4) at t=6s
void TearDownLink (Ptr<Node> nodeA, Ptr<Node> nodeB, uint32_t interfaceA, uint32_t interfaceB)
{
  NS_LOG_UNCOND ("\n[t=6.0s] *** EVENT: Link R1-R4 FAILS ***");
  Ptr<Ipv4> ipv4A = nodeA->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4B = nodeB->GetObject<Ipv4> ();
  ipv4A->SetDown (interfaceA);
  ipv4B->SetDown (interfaceB);
  Ipv4GlobalRoutingHelper::RecomputeRoutingTables ();
}

// Function to restore the primary link (R1-R4) at t=9s
void RestoreLink (Ptr<Node> nodeA, Ptr<Node> nodeB, uint32_t interfaceA, uint32_t interfaceB)
{
  NS_LOG_UNCOND ("\n[t=9.0s] *** EVENT: Link R1-R4 RESTORED ***");
  Ptr<Ipv4> ipv4A = nodeA->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4B = nodeB->GetObject<Ipv4> ();
  ipv4A->SetUp (interfaceA);
  ipv4B->SetUp (interfaceB);
  Ipv4GlobalRoutingHelper::RecomputeRoutingTables ();
}

int main (int argc, char *argv[])
{
  CommandLine cmd (__FILE__);
  cmd.Parse (argc, argv);

  Time::SetResolution (Time::NS);

  // Enable packet logging
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  // 1. Create 5 Router Nodes (R1 to R5)
  NodeContainer nodes;
  nodes.Create (5); // Node 0=R1, 1=R2, 2=R3, 3=R4, 4=R5

  // 2. Configure Point-to-Point Links
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));

  // Link containers
  NetDeviceContainer dR1R2 = p2p.Install (nodes.Get (0), nodes.Get (1));
  NetDeviceContainer dR2R3 = p2p.Install (nodes.Get (1), nodes.Get (2));
  NetDeviceContainer dR3R5 = p2p.Install (nodes.Get (2), nodes.Get (4));
  NetDeviceContainer dR1R4 = p2p.Install (nodes.Get (0), nodes.Get (3));
  NetDeviceContainer dR4R5 = p2p.Install (nodes.Get (3), nodes.Get (4));

  // 3. Install Mobility Model to prevent NetAnim Warnings
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (10.0, 30.0, 0.0)); // R1
  positionAlloc->Add (Vector (30.0, 10.0, 0.0)); // R2
  positionAlloc->Add (Vector (50.0, 10.0, 0.0)); // R3
  positionAlloc->Add (Vector (30.0, 50.0, 0.0)); // R4
  positionAlloc->Add (Vector (70.0, 30.0, 0.0)); // R5
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);

  // 4. Install Internet Stack & Global Routing
  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;

  // Subnet 1: R1 - R2 (10.1.1.0/24)
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer iR1R2 = address.Assign (dR1R2);

  // Subnet 2: R2 - R3 (10.1.2.0/24)
  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer iR2R3 = address.Assign (dR2R3);

  // Subnet 3: R3 - R5 (10.1.3.0/24)
  address.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer iR3R5 = address.Assign (dR3R5);

  // Subnet 4: R1 - R4 (10.1.4.0/24)
  address.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer iR1R4 = address.Assign (dR1R4);

  // Subnet 5: R4 - R5 (10.1.5.0/24)
  address.SetBase ("10.1.5.0", "255.255.255.0");
  Ipv4InterfaceContainer iR4R5 = address.Assign (dR4R5);

  // Populate global routing tables
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // 5. Schedule Link Failure (t=6.0s) and Link Recovery (t=9.0s) on R1-R4
  Simulator::Schedule (Seconds (6.0), &TearDownLink, nodes.Get (0), nodes.Get (3), 2, 1);
  Simulator::Schedule (Seconds (9.0), &RestoreLink, nodes.Get (0), nodes.Get (3), 2, 1);

  // 6. Setup Echo Server on R5
  uint16_t port = 9;
  UdpEchoServerHelper echoServer (port);
  ApplicationContainer serverApps = echoServer.Install (nodes.Get (4));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (14.0));

  // 7. Setup Echo Client on R1 sending to R5's IP on R4-R5 link (10.1.5.2)
  UdpEchoClientHelper echoClient (iR4R5.GetAddress (1), port);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (100));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (0.5)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (512));

  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
  clientApps.Start (Seconds (1.0));
  clientApps.Stop (Seconds (14.0));

  // 8. NetAnim Setup
  AnimationInterface anim ("bgp_partE_netanim.xml");

  // 9. FlowMonitor Setup
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

  std::cout << "Starting 5-router diamond topology routing resilience simulation..." << std::endl;

  Simulator::Stop (Seconds (15.0));
  Simulator::Run ();

  monitor->SerializeToXmlFile ("interas-partE-results.xml", true, true);

  Simulator::Destroy ();
  return 0;
}
