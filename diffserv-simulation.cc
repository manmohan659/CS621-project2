// diffserv-simulation.cc
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/gnuplot.h"
#include "diffserv.h"
#include "spq.h"
#include "drr.h"
#include <string>
#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("DiffServSimulation");

// Function to create the basic 3-node topology
void CreateTopology (
  NodeContainer &nodes,
  NetDeviceContainer &devices,
  InternetStackHelper &stack,
  Ipv4AddressHelper &address,
  Ipv4InterfaceContainer &interfaces
)
{
  // Create 3 nodes
  nodes.Create (3);

  // Create point-to-point links
  PointToPointHelper p2p1, p2p2;
  
  // Set the first link to 4 Mbps
  p2p1.SetDeviceAttribute ("DataRate", StringValue ("4Mbps"));
  p2p1.SetChannelAttribute ("Delay", StringValue ("2ms"));
  
  // Set the second link to 1 Mbps (bottleneck)
  p2p2.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
  p2p2.SetChannelAttribute ("Delay", StringValue ("2ms"));
  
  // Create NetDevices
  NetDeviceContainer devices1 = p2p1.Install (nodes.Get (0), nodes.Get (1));
  NetDeviceContainer devices2 = p2p2.Install (nodes.Get (1), nodes.Get (2));
  
  // Combine all NetDevices
  devices.Add (devices1);
  devices.Add (devices2);
  
  // Install internet stack on all nodes
  stack.Install (nodes);
  
  // Assign IP addresses
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces1 = address.Assign (devices1);
  
  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces2 = address.Assign (devices2);
  
  // Combine all interfaces
  interfaces.Add (interfaces1);
  interfaces.Add (interfaces2);
  
  // Set up routing
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
}

// Function to setup SPQ validation scenario
void SetupSPQValidation (
  NodeContainer &nodes,
  Ipv4InterfaceContainer &interfaces,
  std::string configFile,
  ApplicationContainer &apps,
  Ptr<FlowMonitor> &flowMonitor,
  FlowMonitorHelper &flowHelper
)
{
  NS_LOG_INFO ("Setting up SPQ validation scenario");
  
  // Get the router node (middle node)
  Ptr<Node> router = nodes.Get (1);
  
  // Create SPQ queue
  Ptr<SPQ> spq = CreateObject<SPQ> ();
  spq->SetConfigFile (configFile);
  
  // Get the NetDevice for the bottleneck link (router to receiver)
  Ptr<NetDevice> dev = router->GetDevice (1);
  
  // Set the queue disc on the device
  dev->SetAttribute ("TxQueue", PointerValue (spq));
  
  // Create two bulk send applications with different priorities
  uint16_t port = 9;
  
  // Application B (lower priority) - starts at time 0
  BulkSendHelper sourceB ("ns3::TcpSocketFactory", InetSocketAddress (interfaces.GetAddress (3), port));
  sourceB.SetAttribute ("MaxBytes", UintegerValue (0)); // Unlimited
  ApplicationContainer sourceAppB = sourceB.Install (nodes.Get (0));
  sourceAppB.Start (Seconds (0.0));
  sourceAppB.Stop (Seconds (30.0));
  
  // Application A (higher priority) - starts at time 10
  BulkSendHelper sourceA ("ns3::TcpSocketFactory", InetSocketAddress (interfaces.GetAddress (3), port + 1));
  sourceA.SetAttribute ("MaxBytes", UintegerValue (0)); // Unlimited
  ApplicationContainer sourceAppA = sourceA.Install (nodes.Get (0));
  sourceAppA.Start (Seconds (10.0));
  sourceAppA.Stop (Seconds (20.0));
  
  // Create packet sink applications
  PacketSinkHelper sinkB ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkA ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port + 1));
  
  ApplicationContainer sinkAppB = sinkB.Install (nodes.Get (2));
  ApplicationContainer sinkAppA = sinkA.Install (nodes.Get (2));
  
  sinkAppB.Start (Seconds (0.0));
  sinkAppA.Start (Seconds (0.0));
  
  // Combine all applications
  apps.Add (sourceAppA);
  apps.Add (sourceAppB);
  apps.Add (sinkAppA);
  apps.Add (sinkAppB);
  
  // Install flow monitor
  flowMonitor = flowHelper.InstallAll ();
}

// Function to setup DRR validation scenario
void SetupDRRValidation (
  NodeContainer &nodes,
  Ipv4InterfaceContainer &interfaces,
  std::string configFile,
  ApplicationContainer &apps,
  Ptr<FlowMonitor> &flowMonitor,
  FlowMonitorHelper &flowHelper
)
{
  NS_LOG_INFO ("Setting up DRR validation scenario");
  
  // Get the router node (middle node)
  Ptr<Node> router = nodes.Get (1);
  
  // Create DRR queue
  Ptr<DRR> drr = CreateObject<DRR> ();
  drr->SetConfigFile (configFile);
  
  // Get the NetDevice for the bottleneck link (router to receiver)
  Ptr<NetDevice> dev = router->GetDevice (1);
  
  // Set the queue disc on the device
  dev->SetAttribute ("TxQueue", PointerValue (drr));
  
  // Create three bulk send applications with different weights
  uint16_t port = 9;
  
  // Application A (weight 3)
  BulkSendHelper sourceA ("ns3::TcpSocketFactory", InetSocketAddress (interfaces.GetAddress (3), port));
  sourceA.SetAttribute ("MaxBytes", UintegerValue (0)); // Unlimited
  ApplicationContainer sourceAppA = sourceA.Install (nodes.Get (0));
  sourceAppA.Start (Seconds (0.0));
  sourceAppA.Stop (Seconds (30.0));
  
  // Application B (weight 2)
  BulkSendHelper sourceB ("ns3::TcpSocketFactory", InetSocketAddress (interfaces.GetAddress (3), port + 1));
  sourceB.SetAttribute ("MaxBytes", UintegerValue (0)); // Unlimited
  ApplicationContainer sourceAppB = sourceB.Install (nodes.Get (0));
  sourceAppB.Start (Seconds (0.0));
  sourceAppB.Stop (Seconds (30.0));
  
  // Application C (weight 1)
  BulkSendHelper sourceC ("ns3::TcpSocketFactory", InetSocketAddress (interfaces.GetAddress (3), port + 2));
  sourceC.SetAttribute ("MaxBytes", UintegerValue (0)); // Unlimited
  ApplicationContainer sourceAppC = sourceC.Install (nodes.Get (0));
  sourceAppC.Start (Seconds (0.0));
  sourceAppC.Stop (Seconds (30.0));
  
  // Create packet sink applications
  PacketSinkHelper sinkA ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkB ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port + 1));
  PacketSinkHelper sinkC ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port + 2));
  
  ApplicationContainer sinkAppA = sinkA.Install (nodes.Get (2));
  ApplicationContainer sinkAppB = sinkB.Install (nodes.Get (2));
  ApplicationContainer sinkAppC = sinkC.Install (nodes.Get (2));
  
  sinkAppA.Start (Seconds (0.0));
  sinkAppB.Start (Seconds (0.0));
  sinkAppC.Start (Seconds (0.0));
  
  // Combine all applications
  apps.Add (sourceAppA);
  apps.Add (sourceAppB);
  apps.Add (sourceAppC);
  apps.Add (sinkAppA);
  apps.Add (sinkAppB);
  apps.Add (sinkAppC);
  
  // Install flow monitor
  flowMonitor = flowHelper.InstallAll ();
}

// Function to generate throughput vs time plot
void GenerateThroughputPlot (
  Ptr<FlowMonitor> flowMonitor,
  FlowMonitorHelper &flowHelper,
  std::string filename,
  bool isSPQ
)
{
  NS_LOG_INFO ("Generating throughput plot");
  
  // Create gnuplot object
  Gnuplot plot (filename + ".png");
  plot.SetTitle ("Throughput vs Time");
  plot.SetLegend ("Time (s)", "Throughput (Packets/sec)");
  
  // Create datasets for each flow
  std::map<FlowId, FlowMonitor::FlowStats> stats = flowMonitor->GetFlowStats ();
  
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      // Only consider flows from sender to receiver
      if (i->second.txPackets > 0)
        {
          Gnuplot2dDataset dataset;
          std::string title;
          
          if (isSPQ)
            {
              // For SPQ, we have two applications
              if (i->first == 1)
                {
                  title = "Low Priority";
                  dataset.SetStyle (Gnuplot2dDataset::LINES);
                  dataset.SetLinesType (LinesType (1));
                  dataset.SetColor ("blue");
                }
              else if (i->first == 2)
                {
                  title = "High Priority";
                  dataset.SetStyle (Gnuplot2dDataset::LINES);
                  dataset.SetLinesType (LinesType (1));
                  dataset.SetColor ("red");
                }
              else
                {
                  continue; // Skip other flows
                }
            }
          else
            {
              // For DRR, we have three applications
              if (i->first == 1)
                {
                  title = "Weight 3";
                  dataset.SetStyle (Gnuplot2dDataset::LINES);
                  dataset.SetLinesType (LinesType (1));
                  dataset.SetColor ("red");
                }
              else if (i->first == 2)
                {
                  title = "Weight 2";
                  dataset.SetStyle (Gnuplot2dDataset::LINES);
                  dataset.SetLinesType (LinesType (1));
                  dataset.SetColor ("green");
                }
              else if (i->first == 3)
                {
                  title = "Weight 1";
                  dataset.SetStyle (Gnuplot2dDataset::LINES);
                  dataset.SetLinesType (LinesType (1));
                  dataset.SetColor ("blue");
                }
              else
                {
                  continue; // Skip other flows
                }
            }
          
          dataset.SetTitle (title);
          
          // Add data points
          // Note: This is a simplified approach - in a real implementation,
          // you would need a more sophisticated method to get time series data
          for (uint32_t j = 0; j < i->second.timeFirstTxPacket.GetSeconds (); j += 1)
            {
              dataset.Add (j, 0);
            }
          
          for (uint32_t j = i->second.timeFirstTxPacket.GetSeconds (); j < i->second.timeLastTxPacket.GetSeconds (); j += 1)
            {
              double throughput = i->second.txPackets / (i->second.timeLastTxPacket.GetSeconds () - i->second.timeFirstTxPacket.GetSeconds ());
              dataset.Add (j, throughput);
            }
          
          for (uint32_t j = i->second.timeLastTxPacket.GetSeconds (); j <= 30; j += 1)
            {
              dataset.Add (j, 0);
            }
          
          plot.AddDataset (dataset);
        }
    }
  
  // Generate the plot
  std::ofstream plotFile (filename + ".plt");
  plot.GenerateOutput (plotFile);
  plotFile.close ();
  
  // Use gnuplot to generate the image
  std::string cmd = "gnuplot " + filename + ".plt";
  if (system (cmd.c_str ()) != 0)
    {
      std::cerr << "Failed to run gnuplot command" << std::endl;
    }
}

int main (int argc, char *argv[])
{
  // Enable logging
  LogComponentEnable ("DiffServSimulation", LOG_LEVEL_INFO);
  
  // Parse command line arguments
  std::string mode = "spq";
  std::string configFile = "spq.conf";
  
  CommandLine cmd;
  cmd.AddValue ("mode", "Simulation mode (spq or drr)", mode);
  cmd.AddValue ("config", "Configuration file", configFile);
  cmd.Parse (argc, argv);
  
  // Create the basic topology
  NodeContainer nodes;
  NetDeviceContainer devices;
  InternetStackHelper stack;
  Ipv4AddressHelper address;
  Ipv4InterfaceContainer interfaces;
  
  CreateTopology (nodes, devices, stack, address, interfaces);
  
  // Setup the validation scenario based on the mode
  ApplicationContainer apps;
  Ptr<FlowMonitor> flowMonitor;
  FlowMonitorHelper flowHelper;
  
  if (mode == "spq")
    {
      SetupSPQValidation (nodes, interfaces, configFile, apps, flowMonitor, flowHelper);
    }
  else if (mode == "drr")
    {
      SetupDRRValidation (nodes, interfaces, configFile, apps, flowMonitor, flowHelper);
    }
  else
    {
      NS_FATAL_ERROR ("Unknown mode: " << mode);
    }
  
  // Run the simulation
  Simulator::Stop (Seconds (30.0));
  Simulator::Run ();
  
  // Generate throughput vs time plot
  GenerateThroughputPlot (flowMonitor, flowHelper, mode + "-throughput", mode == "spq");
  
  // Cleanup
  Simulator::Destroy ();
  
  return 0;
}