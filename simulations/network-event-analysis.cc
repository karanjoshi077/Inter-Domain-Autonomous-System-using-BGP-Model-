#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("InterASRoutingLite");

void
FailLink (Ptr<Ipv4> ipv4A, uint32_t ifA, Ptr<Ipv4> ipv4B, uint32_t ifB)
{
  ipv4A->SetDown (ifA);
  ipv4B->SetDown (ifB);
  Ipv4GlobalRoutingHelper::RecomputeRoutingTables ();
  NS_LOG_INFO ("*** Link AS1-AS4 FAILED at t=" << Simulator::Now ().GetSeconds () << "s ***");
}

void
RestoreLink (Ptr<Ipv4> ipv4A, uint32_t ifA, Ptr<Ipv4> ipv4B, uint32_t ifB)
{
  ipv4A->SetUp (ifA);
  ipv4B->SetUp (ifB);
  Ipv4GlobalRoutingHelper::RecomputeRoutingTables ();
  NS_LOG_INFO ("*** Link AS1-AS4 RESTORED at t=" << Simulator::Now ().GetSeconds () << "s ***");
}

void
DumpFlowStats (Ptr<FlowMonitor> monitor, std::string filename)
{
  monitor->CheckForLostPackets ();
  monitor->SerializeToXmlFile (filename, true, true);
  NS_LOG_INFO ("Flow stats snapshot written to " << filename << " at t=" << Simulator::Now ().GetSeconds () << "s");
}

int main (int argc, char *argv[])
{
  Time::SetResolution (Time::NS);
  CommandLine cmd;
  cmd.Parse (argc, argv);

  LogComponentEnable ("InterASRoutingLite", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  NS_LOG_INFO ("Starting link failure and recovery - Failure and Recovery simulation...");

  NodeContainer routers;
  routers.Create (4);

  InternetStackHelper internet;
  internet.Install (routers);

  PointToPointHelper p2p12;
  p2p12.SetDeviceAttribute ("DataRate", StringValue ("8Mbps"));
  p2p12.SetChannelAttribute ("Delay", StringValue ("3ms"));
  NetDeviceContainer d12 = p2p12.Install (routers.Get (0), routers.Get (1));

  PointToPointHelper p2p23;
  p2p23.SetDeviceAttribute ("DataRate", StringValue ("6Mbps"));
  p2p23.SetChannelAttribute ("Delay", StringValue ("4ms"));
  NetDeviceContainer d23 = p2p23.Install (routers.Get (1), routers.Get (2));

  PointToPointHelper p2p34;
  p2p34.SetDeviceAttribute ("DataRate", StringValue ("8Mbps"));
  p2p34.SetChannelAttribute ("Delay", StringValue ("3ms"));
  NetDeviceContainer d34 = p2p34.Install (routers.Get (2), routers.Get (3));

  PointToPointHelper p2p14;
  p2p14.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  p2p14.SetChannelAttribute ("Delay", StringValue ("1ms"));
  NetDeviceContainer d14 = p2p14.Install (routers.Get (0), routers.Get (3));

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i12 = ipv4.Assign (d12);

  ipv4.SetBase ("192.168.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i23 = ipv4.Assign (d23);

  ipv4.SetBase ("192.168.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i34 = ipv4.Assign (d34);

  ipv4.SetBase ("192.168.4.0", "255.255.255.0");
  Ipv4InterfaceContainer i14 = ipv4.Assign (d14);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Ptr<Ipv4> ipv4A1 = routers.Get (0)->GetObject<Ipv4> ();
  uint32_t ifIndexA1 = static_cast<uint32_t> (ipv4A1->GetInterfaceForDevice (d14.Get (0)));

  Ptr<Ipv4> ipv4A4 = routers.Get (3)->GetObject<Ipv4> ();
  uint32_t ifIndexA4 = static_cast<uint32_t> (ipv4A4->GetInterfaceForDevice (d14.Get (1)));

  Simulator::Schedule (Seconds (6.0), &FailLink, ipv4A1, ifIndexA1, ipv4A4, ifIndexA4);
  Simulator::Schedule (Seconds (9.0), &RestoreLink, ipv4A1, ifIndexA1, ipv4A4, ifIndexA4);

  uint16_t port = 8080;
  UdpEchoServerHelper server (port);
  ApplicationContainer apps = server.Install (routers.Get (3));
  apps.Start (Seconds (0.5));
  apps.Stop (Seconds (16.0));

  UdpEchoClientHelper client (i34.GetAddress (1), port);
  client.SetAttribute ("MaxPackets", UintegerValue (50));
  client.SetAttribute ("Interval", TimeValue (Seconds (0.5)));
  client.SetAttribute ("PacketSize", UintegerValue (512));

  apps = client.Install (routers.Get (0));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (14.0));

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

  Simulator::Schedule (Seconds (5.99), &DumpFlowStats, monitor, std::string ("interas-partD-cumulative-before.xml"));
  Simulator::Schedule (Seconds (8.99), &DumpFlowStats, monitor, std::string ("interas-partD-cumulative-during.xml"));
  Simulator::Schedule (Seconds (14.99), &DumpFlowStats, monitor, std::string ("interas-partD-cumulative-final.xml"));

  AnimationInterface anim ("bgp_partD_netanim.xml");
  anim.SetConstantPosition (routers.Get (0), 10.0, 30.0);
  anim.SetConstantPosition (routers.Get (1), 30.0, 10.0);
  anim.SetConstantPosition (routers.Get (2), 50.0, 10.0);
  anim.SetConstantPosition (routers.Get (3), 70.0, 30.0);
  anim.EnablePacketMetadata (true);

  Simulator::Stop (Seconds (16.0));
  Simulator::Run ();

  Simulator::Destroy ();
  return 0;
}
