#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/ipv4-static-routing-helper.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("InterASRoutingLite");

int main (int argc, char *argv[])
{
  Time::SetResolution (Time::NS);
  CommandLine cmd;
  cmd.Parse (argc, argv);

  LogComponentEnable ("InterASRoutingLite", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  NS_LOG_INFO ("Starting Part C - Mechanism 2 (static route) simulation...");

  NodeContainer routers;
  routers.Create (4);

  InternetStackHelper internet;
  internet.Install (routers);

  // AS1 -- AS2
  PointToPointHelper p2p12;
  p2p12.SetDeviceAttribute ("DataRate", StringValue ("8Mbps"));
  p2p12.SetChannelAttribute ("Delay", StringValue ("3ms"));
  NetDeviceContainer d12 = p2p12.Install (routers.Get (0), routers.Get (1));

  // AS2 -- AS3
  PointToPointHelper p2p23;
  p2p23.SetDeviceAttribute ("DataRate", StringValue ("6Mbps"));
  p2p23.SetChannelAttribute ("Delay", StringValue ("4ms"));
  NetDeviceContainer d23 = p2p23.Install (routers.Get (1), routers.Get (2));

  // AS3 -- AS4
  PointToPointHelper p2p34;
  p2p34.SetDeviceAttribute ("DataRate", StringValue ("8Mbps"));
  p2p34.SetChannelAttribute ("Delay", StringValue ("3ms"));
  NetDeviceContainer d34 = p2p34.Install (routers.Get (2), routers.Get (3));

// AS1 -- AS4 (direct peering link, Part B)
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

  Ptr<Ipv4StaticRouting> staticRoutingA1 = Ipv4StaticRoutingHelper ().GetStaticRouting (routers.Get (0)->GetObject<Ipv4> ());
  staticRoutingA1->AddNetworkRouteTo (Ipv4Address ("192.168.3.0"), Ipv4Mask ("255.255.255.0"), i12.GetAddress (1), 1);

  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("as1-routing-table.txt", std::ios::out);
  Ipv4RoutingHelper::PrintRoutingTableAt (Seconds (1.9), routers.Get (0), routingStream);

  Ptr<Ipv4StaticRouting> staticRoutingA2 = Ipv4StaticRoutingHelper ().GetStaticRouting (routers.Get (1)->GetObject<Ipv4> ());
  staticRoutingA2->AddNetworkRouteTo (Ipv4Address ("192.168.3.0"), Ipv4Mask ("255.255.255.0"), i23.GetAddress (1), 2);

  Ptr<OutputStreamWrapper> routingStream2 = Create<OutputStreamWrapper> ("as2-routing-table.txt", std::ios::out);
  Ipv4RoutingHelper::PrintRoutingTableAt (Seconds (1.9), routers.Get (1), routingStream2);
  Ptr<OutputStreamWrapper> routingStream3 = Create<OutputStreamWrapper> ("as3-routing-table.txt", std::ios::out);
  Ipv4RoutingHelper::PrintRoutingTableAt (Seconds (1.9), routers.Get (2), routingStream3);
  Ptr<OutputStreamWrapper> routingStream4 = Create<OutputStreamWrapper> ("as4-routing-table.txt", std::ios::out);
  Ipv4RoutingHelper::PrintRoutingTableAt (Seconds (1.9), routers.Get (3), routingStream4);

  uint16_t port = 8080;
  UdpEchoServerHelper server (port);
  ApplicationContainer apps = server.Install (routers.Get (3));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (13.0));

  UdpEchoClientHelper client (i34.GetAddress (1), port);
  client.SetAttribute ("MaxPackets", UintegerValue (8));
  client.SetAttribute ("Interval", TimeValue (Seconds (0.75)));
  client.SetAttribute ("PacketSize", UintegerValue (512));

  apps = client.Install (routers.Get (0));
  apps.Start (Seconds (2.0));
  apps.Stop (Seconds (13.0));

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();



  Simulator::Stop (Seconds (14.0));
  Simulator::Run ();

  monitor->SerializeToXmlFile ("interas-partC-mechanism2.xml", true, true);

  Simulator::Destroy ();
  return 0;
}
