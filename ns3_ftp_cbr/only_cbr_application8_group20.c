//
// Network topology
//
//  h1                                h3
//     \ p-p                         / p-p
//      \           (p-p)           /
//       r1 -----------------------r2
//      / p-p                       \ p-p
//     / p-p                         \ p-p
//   h2                               h4


// - Tracing of queues and packet receptions to file "tryagain.tr"

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("DynamicGlobalRoutingExample");

int 
main (int argc, char *argv[])
{
  // The below value configures the default behavior of global routing.
  // By default, it is disabled.  To respond to interface events, set to true
  Config::SetDefault ("ns3::Ipv4GlobalRouting::RespondToInterfaceEvents", BooleanValue (true));

  // Allow the user to override any of the defaults and the above
  // Bind ()s at run-time, via command-line arguments
  CommandLine cmd;
  cmd.Parse (argc, argv);

  NS_LOG_INFO ("Create nodes.");
  NodeContainer c;
  c.Create (6);

  NodeContainer r1h1 = NodeContainer (c.Get (2), c.Get (0));
  NodeContainer r1h2 = NodeContainer (c.Get (2), c.Get (1));
  NodeContainer r2h3 = NodeContainer (c.Get (3), c.Get (4));
  NodeContainer r2h4 = NodeContainer (c.Get (3), c.Get (5));
  NodeContainer r1r2 = NodeContainer (c.Get (2), c.Get (3));

  InternetStackHelper internet;
  internet.Install (c);

  // We create the channels first without any IP addressing information
  NS_LOG_INFO ("Create channels.");
  PointToPointHelper p2p;
  //bandwidth = 30 * 1000000, delay = 0.1
  p2p.SetQueue ("ns3::DropTailQueue", "MaxPackets", StringValue ("375000"));
  p2p.SetDeviceAttribute ("DataRate", StringValue ("80Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("20ms"));
  NetDeviceContainer d2d0 = p2p.Install (r1h1);
  NetDeviceContainer d2d1 = p2p.Install (r1h2);
  NetDeviceContainer d3d4 = p2p.Install (r2h3);
  NetDeviceContainer d3d5 = p2p.Install (r2h4);
  p2p.SetDeviceAttribute ("DataRate", StringValue ("30Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("100ms"));
  NetDeviceContainer d2d3 = p2p.Install (r1r2);

  // Later, we add IP addresses.
  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("172.16.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i2i0 = ipv4.Assign (d2d0);

  ipv4.SetBase ("172.16.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i2i1 = ipv4.Assign (d2d1);

  ipv4.SetBase ("172.16.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i3i4 = ipv4.Assign (d3d4);

  ipv4.SetBase ("172.16.4.0", "255.255.255.0");
  Ipv4InterfaceContainer i3i5 = ipv4.Assign (d3d5);

  ipv4.SetBase ("10.250.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i2i3 = ipv4.Assign (d2d3);

  // Create router nodes, initialize routing database and set up the routing
  // tables in the nodes.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // Create the OnOff application to send UDP datagrams of size
  NS_LOG_INFO ("Create Applications.");
  uint16_t port = 2;  
  OnOffHelper onoff ("ns3::UdpSocketFactory",
                     InetSocketAddress (i3i4.GetAddress (1), port));
  onoff.SetConstantRate (DataRate ("100kbps"));
  onoff.SetAttribute ("PacketSize", UintegerValue (50));

  ApplicationContainer apps = onoff.Install (c.Get (1));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (10.0));

  PacketSinkHelper sink ("ns3::UdpSocketFactory",
                         Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
  apps = sink.Install (c.Get (4));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (10.0));

  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("tryagain.tr");
  p2p.EnableAsciiAll (stream);
  internet.EnableAsciiIpv4All (stream);

  p2p.EnablePcapAll ("tryagain");

  // Trace routing tables 
  Ipv4GlobalRoutingHelper g;
  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("tryagain.routes", std::ios::out);
  g.PrintRoutingTableAllAt (Seconds (12), routingStream);

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
