#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/gnuplot.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-flow-classifier.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/traffic-control-module.h"

#include "dest-port-filter.h"
#include "diffserv.h"
#include "drr.h"
#include "filter.h"
#include "spq.h"
#include "traffic-class.h"

#include <fstream>
#include <map>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("DiffServSimulation");

static FlowMonitorHelper g_flowHelper;

uint16_t portBase = 9;

static std::map<FlowId, std::map<double, double>> g_flowPlotData;
static std::map<FlowId, uint64_t> g_lastRxPackets;
static std::map<FlowId, Ipv4FlowClassifier::FiveTuple> g_flowFiveTuples;

static uint16_t g_appBPort_SPQ;
static uint16_t g_appAPort_SPQ;
static uint16_t g_appAPort_DRR;
static uint16_t g_appBPort_DRR;
static uint16_t g_appCPort_DRR;

static double g_plotBinInterval = 0.5;
static double g_simDuration = 40.0;

void WriteDefaultConfigFile(const std::string& filename,
                            const std::string& content)
{
  std::ofstream outfile(filename.c_str());
  if (outfile.is_open())
  {
    outfile << content;
    outfile.close();
    NS_LOG_INFO("Created default config file: " << filename);
  }
  else
  {
    NS_LOG_ERROR("Could not write default config file: " << filename);
  }
}

void CreateTopology(NodeContainer& nodes, NetDeviceContainer& p2pDevices,
                    InternetStackHelper& stack, Ipv4AddressHelper& address,
                    Ipv4InterfaceContainer& routerInterfaces,
                    Ipv4InterfaceContainer& sourceHostInterface,
                    Ipv4InterfaceContainer& sinkHostInterface)
{
  nodes.Create(3);
  PointToPointHelper p2pLink1, p2pLink2;

  p2pLink1.SetDeviceAttribute("DataRate", StringValue("4Mbps"));
  p2pLink1.SetChannelAttribute("Delay", StringValue("2ms"));

  p2pLink2.SetDeviceAttribute("DataRate", StringValue("1Mbps"));
  p2pLink2.SetChannelAttribute("Delay", StringValue("2ms"));

  NetDeviceContainer devices01 = p2pLink1.Install(nodes.Get(0), nodes.Get(1));
  NetDeviceContainer devices12 = p2pLink2.Install(nodes.Get(1), nodes.Get(2));

  p2pDevices.Add(devices01);
  p2pDevices.Add(devices12);

  stack.Install(nodes);

  address.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces01 = address.Assign(devices01);
  sourceHostInterface.Add(interfaces01.Get(0));
  routerInterfaces.Add(interfaces01.Get(1));

  address.SetBase("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces12 = address.Assign(devices12);
  routerInterfaces.Add(interfaces12.Get(0));
  sinkHostInterface.Add(interfaces12.Get(1));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();
}

void SetupSPQValidation(NodeContainer& nodes,
                        Ipv4InterfaceContainer& sinkNodeInterface,
                        std::string configFile, ApplicationContainer& apps,
                        Ptr<FlowMonitor>& flowMonitorInstance,
                        FlowMonitorHelper& localFlowHelper, bool useCiscoConfig)
{
  NS_LOG_INFO("Setting up SPQ validation scenario");

  g_appBPort_SPQ = portBase;
  g_appAPort_SPQ = portBase + 1;

  Ptr<Node> router = nodes.Get(1);
  Ptr<SPQ> spq = CreateObject<SPQ>();

  if (useCiscoConfig)
  {
    NS_LOG_INFO("Using Cisco configuration format for SPQ: " << configFile);
    if (!configFile.empty())
      spq->SetCiscoConfigFile(configFile);
  }
  else
  {
    NS_LOG_INFO("Using standard configuration format for SPQ: " << configFile);
    NS_ASSERT_MSG(!configFile.empty(),
                  "SPQ standard config file must be provided.");
    if (!spq->SetConfigFile(configFile))
    {
      NS_FATAL_ERROR("Failed to set SPQ config file: " << configFile);
    }
  }

  NS_ASSERT_MSG(spq->GetNTrafficClasses() >= 2,
                "SPQ config did not create at least 2 queues for validation.");

  Ptr<TrafficClass> highPriorityClass = spq->GetTrafficClass(0);
  NS_ASSERT_MSG(
      highPriorityClass,
      "Could not get high priority traffic class (index 0) from SPQ object.");
  Ptr<Filter> highFilter = CreateObject<Filter>();
  highFilter->AddFilterElement(CreateObject<DestPortFilter>(g_appAPort_SPQ));
  highPriorityClass->AddFilter(highFilter);

  Ptr<TrafficClass> lowPriorityClass = spq->GetTrafficClass(1);
  NS_ASSERT_MSG(
      lowPriorityClass,
      "Could not get low priority traffic class (index 1) from SPQ object.");
  Ptr<Filter> lowFilter = CreateObject<Filter>();
  lowFilter->AddFilterElement(CreateObject<DestPortFilter>(g_appBPort_SPQ));
  lowPriorityClass->AddFilter(lowFilter);

  Ptr<NetDevice> routerEgressDev = router->GetDevice(1);
  routerEgressDev->SetAttribute("TxQueue", PointerValue(spq));

  // OnOffHelper sourceB(
  //     "ns3::UdpSocketFactory",
  //     InetSocketAddress(sinkNodeInterface.GetAddress(0), g_appBPort_SPQ));
  // sourceB.SetAttribute("MaxBytes", UintegerValue(0));
  // sourceB.SetAttribute("DataRate", DataRateValue(DataRate("1Mbps")));
  // sourceB.SetAttribute("PacketSize", UintegerValue(1024));
  // // Configure for constant "ON" time with no "OFF" time
  // sourceB.SetAttribute(
  //     "OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
  // sourceB.SetAttribute(
  //     "OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));
  // ApplicationContainer sourceAppB = sourceB.Install(nodes.Get(0));
  // sourceAppB.Start(Seconds(0.0));
  // sourceAppB.Stop(Seconds(g_simDuration));

  // OnOffHelper sourceA(
  //     "ns3::UdpSocketFactory",
  //     InetSocketAddress(sinkNodeInterface.GetAddress(0), g_appAPort_SPQ));
  // sourceA.SetAttribute("MaxBytes", UintegerValue(0));
  // sourceA.SetAttribute("DataRate", DataRateValue(DataRate("1Mbps")));
  // sourceA.SetAttribute("PacketSize", UintegerValue(1024));
  // sourceA.SetAttribute(
  //     "OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
  // sourceA.SetAttribute(
  //     "OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));
  // ApplicationContainer sourceAppA = sourceA.Install(nodes.Get(0));
  // sourceAppA.Start(Seconds(12.0));
  // sourceAppA.Stop(Seconds(20.0));

  UdpClientHelper clientB(sinkNodeInterface.GetAddress(0), g_appBPort_SPQ);
  clientB.SetAttribute("MaxPackets", UintegerValue(4294967295u)); // Unlimited
  // Calculate interval based on desired data rate and packet size
  // For 1Mbps with 1024-byte packets: (1024*8)/(1*10^6) = 0.008192 seconds
  // between packets
  clientB.SetAttribute("Interval", TimeValue(Seconds(0.008192)));
  clientB.SetAttribute("PacketSize", UintegerValue(1024));
  ApplicationContainer sourceAppB = clientB.Install(nodes.Get(0));
  sourceAppB.Start(Seconds(0.0));
  sourceAppB.Stop(Seconds(g_simDuration));

  UdpClientHelper clientA(sinkNodeInterface.GetAddress(0), g_appAPort_SPQ);
  clientA.SetAttribute("MaxPackets", UintegerValue(4294967295u)); // Unlimited
  // Calculate interval based on desired data rate and packet size
  // For 1Mbps with 1024-byte packets: (1024*8)/(1*10^6) = 0.008192 seconds
  // between packets
  clientA.SetAttribute("Interval", TimeValue(Seconds(0.008192)));
  clientA.SetAttribute("PacketSize", UintegerValue(1024));
  ApplicationContainer sourceAppA = clientA.Install(nodes.Get(0));
  sourceAppA.Start(Seconds(12.0));
  sourceAppA.Stop(Seconds(20.0));

  PacketSinkHelper sinkB(
      "ns3::UdpSocketFactory",
      InetSocketAddress(Ipv4Address::GetAny(), g_appBPort_SPQ));
  ApplicationContainer sinkAppB = sinkB.Install(nodes.Get(2));
  sinkAppB.Start(Seconds(0.0));
  sinkAppB.Stop(Seconds(g_simDuration));

  PacketSinkHelper sinkA(
      "ns3::UdpSocketFactory",
      InetSocketAddress(Ipv4Address::GetAny(), g_appAPort_SPQ));
  ApplicationContainer sinkAppA = sinkA.Install(nodes.Get(2));
  sinkAppA.Start(Seconds(0.0));
  sinkAppA.Stop(Seconds(g_simDuration));

  apps.Add(sourceAppA);
  apps.Add(sourceAppB);
  apps.Add(sinkAppA);
  apps.Add(sinkAppB);

  flowMonitorInstance = localFlowHelper.InstallAll();
}

void SetupSPQValidationFromCisco(NodeContainer& nodes,
                                 Ipv4InterfaceContainer& sinkNodeInterface,
                                 const std::string& ciscoConfigFile,
                                 ApplicationContainer& apps,
                                 Ptr<FlowMonitor>& flowMonitorInstance,
                                 FlowMonitorHelper& localFlowHelper)
{
  NS_LOG_INFO("Setting up SPQ validation scenario using Cisco config");

  g_appBPort_SPQ = portBase;
  g_appAPort_SPQ = portBase + 1;

  Ptr<Node> router = nodes.Get(1);
  Ptr<SPQ> spq = CreateObject<SPQ>();

  NS_ASSERT_MSG(!ciscoConfigFile.empty(),
                "Cisco config file must be provided.");
  spq->SetCiscoConfigFile(ciscoConfigFile);

  NS_ASSERT_MSG(
      spq->GetNTrafficClasses() >= 2,
      "SPQ Cisco config did not create at least 2 queues for validation.");

  Ptr<TrafficClass> highPriorityClass = spq->GetTrafficClass(0);
  NS_ASSERT_MSG(highPriorityClass,
                "Could not get high priority traffic class (index 0).");
  Ptr<Filter> highFilter = CreateObject<Filter>();
  highFilter->AddFilterElement(CreateObject<DestPortFilter>(g_appAPort_SPQ));
  highPriorityClass->AddFilter(highFilter);

  Ptr<TrafficClass> lowPriorityClass = spq->GetTrafficClass(1);
  NS_ASSERT_MSG(lowPriorityClass,
                "Could not get low priority traffic class (index 1).");
  Ptr<Filter> lowFilter = CreateObject<Filter>();
  lowFilter->AddFilterElement(CreateObject<DestPortFilter>(g_appBPort_SPQ));
  lowPriorityClass->AddFilter(lowFilter);

  Ptr<NetDevice> routerEgressDev = router->GetDevice(1);
  routerEgressDev->SetAttribute("TxQueue", PointerValue(spq));

  BulkSendHelper sourceB(
      "ns3::TcpSocketFactory",
      InetSocketAddress(sinkNodeInterface.GetAddress(0), g_appBPort_SPQ));
  sourceB.SetAttribute("MaxBytes", UintegerValue(0));
  ApplicationContainer sourceAppB = sourceB.Install(nodes.Get(0));
  sourceAppB.Start(Seconds(0.0));
  sourceAppB.Stop(Seconds(g_simDuration));

  BulkSendHelper sourceA(
      "ns3::TcpSocketFactory",
      InetSocketAddress(sinkNodeInterface.GetAddress(0), g_appAPort_SPQ));
  sourceA.SetAttribute("MaxBytes", UintegerValue(0));
  ApplicationContainer sourceAppA = sourceA.Install(nodes.Get(0));
  sourceAppA.Start(Seconds(12.0));
  sourceAppA.Stop(Seconds(20.0));

  PacketSinkHelper sinkB(
      "ns3::TcpSocketFactory",
      InetSocketAddress(Ipv4Address::GetAny(), g_appBPort_SPQ));
  ApplicationContainer sinkAppB = sinkB.Install(nodes.Get(2));
  sinkAppB.Start(Seconds(0.0));
  sinkAppB.Stop(Seconds(g_simDuration));

  PacketSinkHelper sinkA(
      "ns3::TcpSocketFactory",
      InetSocketAddress(Ipv4Address::GetAny(), g_appAPort_SPQ));
  ApplicationContainer sinkAppA = sinkA.Install(nodes.Get(2));
  sinkAppA.Start(Seconds(0.0));
  sinkAppA.Stop(Seconds(g_simDuration));

  apps.Add(sourceAppA);
  apps.Add(sourceAppB);
  apps.Add(sinkAppA);
  apps.Add(sinkAppB);

  flowMonitorInstance = localFlowHelper.InstallAll();
}

void SetupDRRValidation(NodeContainer& nodes,
                        Ipv4InterfaceContainer& sinkNodeInterface,
                        std::string configFile, ApplicationContainer& apps,
                        Ptr<FlowMonitor>& flowMonitorInstance,
                        FlowMonitorHelper& localFlowHelper)
{
  std::cout << "SetupDRRValidation" << std::endl;
  NS_LOG_INFO("Setting up DRR validation scenario");

  g_appAPort_DRR = portBase;
  g_appBPort_DRR = portBase + 1;
  g_appCPort_DRR = portBase + 2;

  Ptr<Node> router = nodes.Get(1);
  Ptr<DRR> drr = CreateObject<DRR>();

  NS_ASSERT_MSG(!configFile.empty(), "DRR config file must be provided.");
  if (!drr->SetConfigFile(configFile))
  {
    NS_FATAL_ERROR("Failed to set DRR config file: " << configFile);
  }
  std::cout << "DRR::SetConfigFile complete" << std::endl;

  NS_ASSERT_MSG(drr->GetNTrafficClasses() >= 3,
                "DRR config did not create at least 3 queues for validation.");

  std::cout << "creating traffic classes" << std::endl;
  Ptr<TrafficClass> classA = drr->GetTrafficClass(0);
  NS_ASSERT_MSG(classA, "Could not get DRR traffic class 0.");
  Ptr<Filter> filterA = CreateObject<Filter>();
  filterA->AddFilterElement(CreateObject<DestPortFilter>(g_appAPort_DRR));
  classA->AddFilter(filterA);

  Ptr<TrafficClass> classB = drr->GetTrafficClass(1);
  NS_ASSERT_MSG(classB, "Could not get DRR traffic class 1.");
  Ptr<Filter> filterB = CreateObject<Filter>();
  filterB->AddFilterElement(CreateObject<DestPortFilter>(g_appBPort_DRR));
  classB->AddFilter(filterB);

  Ptr<TrafficClass> classC = drr->GetTrafficClass(2);
  NS_ASSERT_MSG(classC, "Could not get DRR traffic class 2.");
  Ptr<Filter> filterC = CreateObject<Filter>();
  filterC->AddFilterElement(CreateObject<DestPortFilter>(g_appCPort_DRR));
  classC->AddFilter(filterC);
  std::cout << "created traffic classes" << std::endl;

  Ptr<NetDevice> routerEgressDev = router->GetDevice(1);
  routerEgressDev->SetAttribute("TxQueue", PointerValue(drr));

  ApplicationContainer sourceAppsLocal, sinkAppsLocal;

  // std::cout << "creating udp sources and sinks" << std::endl;
  // UdpClientHelper sourceWt3(sinkNodeInterface.GetAddress(0), g_appAPort_DRR);
  // sourceWt3.SetAttribute("MaxPackets", UintegerValue(0)); // Unlimited
  // packets sourceWt3.SetAttribute("Interval",
  //                        TimeValue(Seconds(0.01)));          // Packet
  //                        interval
  // sourceWt3.SetAttribute("PacketSize", UintegerValue(1000)); // Bytes per
  // packet sourceAppsLocal.Add(sourceWt3.Install(nodes.Get(0)));
  // UdpServerHelper sinkWt3(g_appAPort_DRR);
  // sinkAppsLocal.Add(sinkWt3.Install(nodes.Get(2)));

  // UdpClientHelper sourceWt2(sinkNodeInterface.GetAddress(0), g_appBPort_DRR);
  // sourceWt2.SetAttribute("MaxPackets", UintegerValue(0)); // Unlimited
  // packets sourceWt2.SetAttribute("Interval",
  //                        TimeValue(Seconds(0.01)));          // Packet
  //                        interval
  // sourceWt2.SetAttribute("PacketSize", UintegerValue(1000)); // Bytes per
  // packet sourceAppsLocal.Add(sourceWt2.Install(nodes.Get(0)));
  // UdpServerHelper sinkWt2(g_appBPort_DRR);
  // sinkAppsLocal.Add(sinkWt2.Install(nodes.Get(2)));

  // UdpClientHelper sourceWt1(sinkNodeInterface.GetAddress(0), g_appCPort_DRR);
  // sourceWt1.SetAttribute("MaxPackets", UintegerValue(0)); // Unlimited
  // packets sourceWt1.SetAttribute("Interval",
  //                        TimeValue(Seconds(0.01)));          // Packet
  //                        interval
  // sourceWt1.SetAttribute("PacketSize", UintegerValue(1000)); // Bytes per
  // packet sourceAppsLocal.Add(sourceWt1.Install(nodes.Get(0)));
  // UdpServerHelper sinkWt1(g_appCPort_DRR);
  // sinkAppsLocal.Add(sinkWt1.Install(nodes.Get(2)));

  // sourceAppsLocal.Start(Seconds(0.0));
  // sourceAppsLocal.Stop(Seconds(g_simDuration));
  // sinkAppsLocal.Start(Seconds(0.0));
  // sinkAppsLocal.Stop(Seconds(g_simDuration));

  // apps.Add(sourceAppsLocal);
  // apps.Add(sinkAppsLocal);

  std::cout << "creating udp sources and sinks" << std::endl;

  OnOffHelper sourceWt3(
      "ns3::UdpSocketFactory",
      InetSocketAddress(sinkNodeInterface.GetAddress(0), g_appAPort_DRR));
  sourceWt3.SetAttribute("DataRate", StringValue("4Mbps")); // Adjust as needed
  sourceWt3.SetAttribute("PacketSize", UintegerValue(1000));
  sourceAppsLocal.Add(sourceWt3.Install(nodes.Get(0)));

  PacketSinkHelper sinkWt3(
      "ns3::UdpSocketFactory",
      InetSocketAddress(Ipv4Address::GetAny(), g_appAPort_DRR));
  sinkAppsLocal.Add(sinkWt3.Install(nodes.Get(2)));

  OnOffHelper sourceWt2(
      "ns3::UdpSocketFactory",
      InetSocketAddress(sinkNodeInterface.GetAddress(0), g_appBPort_DRR));
  sourceWt2.SetAttribute("DataRate", StringValue("4Mbps"));
  sourceWt2.SetAttribute("PacketSize", UintegerValue(1000));
  sourceAppsLocal.Add(sourceWt2.Install(nodes.Get(0)));

  PacketSinkHelper sinkWt2(
      "ns3::UdpSocketFactory",
      InetSocketAddress(Ipv4Address::GetAny(), g_appBPort_DRR));
  sinkAppsLocal.Add(sinkWt2.Install(nodes.Get(2)));

  OnOffHelper sourceWt1(
      "ns3::UdpSocketFactory",
      InetSocketAddress(sinkNodeInterface.GetAddress(0), g_appCPort_DRR));
  sourceWt1.SetAttribute("DataRate", StringValue("4Mbps"));
  sourceWt1.SetAttribute("PacketSize", UintegerValue(1000));
  sourceAppsLocal.Add(sourceWt1.Install(nodes.Get(0)));

  PacketSinkHelper sinkWt1(
      "ns3::UdpSocketFactory",
      InetSocketAddress(Ipv4Address::GetAny(), g_appCPort_DRR));
  sinkAppsLocal.Add(sinkWt1.Install(nodes.Get(2)));

  sourceAppsLocal.Start(Seconds(0.0));
  sourceAppsLocal.Stop(Seconds(g_simDuration));
  sinkAppsLocal.Start(Seconds(0.0));
  sinkAppsLocal.Stop(Seconds(g_simDuration));

  apps.Add(sourceAppsLocal);
  apps.Add(sinkAppsLocal);

  flowMonitorInstance = localFlowHelper.InstallAll();
}

void RecordPeriodicStats(Ptr<FlowMonitor> monitor,
                         Ptr<Ipv4FlowClassifier> classifier, bool isSPQScenario)
{
  monitor->CheckForLostPackets();
  NS_ASSERT_MSG(classifier, "Passed classifier is null in RecordPeriodicStats");

  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats();
  for (auto const& [flowId, flowStats] : stats)
  {
    if (g_flowFiveTuples.find(flowId) == g_flowFiveTuples.end())
    {
      Ipv4FlowClassifier::FiveTuple ft = classifier->FindFlow(flowId);
      if (ft.sourceAddress != Ipv4Address())
      {
        g_flowFiveTuples[flowId] = ft;
      }
      else
      {
        NS_LOG_DEBUG("Classifier could not find flow "
                     << flowId << " in RecordPeriodicStats yet.");
      }
    }

    uint16_t destPort = 0;
    if (g_flowFiveTuples.count(flowId))
    {
      destPort = g_flowFiveTuples[flowId].destinationPort;
    }
    else
    {
      NS_LOG_WARN("Could not determine destination port for flowId "
                  << flowId
                  << " in RecordPeriodicStats. Skipping stat update.");
      continue;
    }

    bool relevantFlow = false;
    if (isSPQScenario)
    {
      if (destPort == g_appAPort_SPQ || destPort == g_appBPort_SPQ)
      {
        relevantFlow = true;
      }
    }
    else
    {
      if (destPort == g_appAPort_DRR || destPort == g_appBPort_DRR ||
          destPort == g_appCPort_DRR)
      {
        relevantFlow = true;
      }
    }

    if (!relevantFlow && flowStats.rxPackets == 0 &&
        g_lastRxPackets.find(flowId) == g_lastRxPackets.end())
    {
      continue;
    }
    if (!relevantFlow && destPort != 0)
    {
      continue;
    }

    uint64_t currentTotalRxPkts = flowStats.rxPackets;
    uint64_t pktsInThisBin = 0;

    if (g_lastRxPackets.count(flowId))
    {
      pktsInThisBin = currentTotalRxPkts - g_lastRxPackets[flowId];
    }
    else
    {
      pktsInThisBin = currentTotalRxPkts;
    }

    double throughputPktsPerSec =
        static_cast<double>(pktsInThisBin) / g_plotBinInterval;

    g_flowPlotData[flowId][Simulator::Now().GetSeconds()] =
        throughputPktsPerSec;
    g_lastRxPackets[flowId] = currentTotalRxPkts;
  }
}

void GenerateThroughputPlot(FlowMonitorHelper& localFlowHelper,
                            std::string filename, bool isSPQ)
{
  NS_LOG_INFO("Generating throughput plot: " << filename);

  Gnuplot plot(filename + ".png");
  plot.SetTerminal("pngcairo enhanced font 'arial,10' size 800,600");
  plot.SetTitle("Throughput vs Time");
  plot.SetLegend("Time (s)", "Throughput (Packets/sec)");
  plot.SetExtra("set xrange [0:" + std::to_string(g_simDuration) + "]");
  plot.SetExtra("set yrange [0:]");

  Ptr<FlowMonitor> monitor = localFlowHelper.GetMonitor();
  if (!monitor)
  {
    NS_LOG_ERROR("FlowMonitor is null in GenerateThroughputPlot (from helper)");
    return;
  }

  Ptr<Ipv4FlowClassifier> classifier =
      DynamicCast<Ipv4FlowClassifier>(localFlowHelper.GetClassifier());
  NS_ASSERT_MSG(
      classifier,
      "FlowMonitorHelper does not have an Ipv4FlowClassifier for plotting.");

  for (auto const& [flowId, timeSeriesData] : g_flowPlotData)
  {
    Gnuplot2dDataset dataset;
    std::string title = "Flow " + std::to_string(flowId);
    std::string color = "black";

    uint16_t destPort = 0;
    if (g_flowFiveTuples.count(flowId))
    {
      destPort = g_flowFiveTuples[flowId].destinationPort;
    }
    else
    {

      NS_LOG_WARN("FlowId " << flowId
                            << " FiveTuple not found in g_flowFiveTuples "
                               "during plotting. Plotting with default title.");

      Ipv4FlowClassifier::FiveTuple ft = classifier->FindFlow(flowId);
      if (ft.sourceAddress != Ipv4Address())
      {
        g_flowFiveTuples[flowId] = ft;
        destPort = ft.destinationPort;
      }
    }

    if (destPort == 0 && timeSeriesData.empty())
    {
      NS_LOG_DEBUG("Skipping empty, unclassified flow " << flowId
                                                        << " in plot.");
      continue;
    }

    bool plotThisFlow = false;
    if (isSPQ)
    {
      if (g_appBPort_SPQ != 0 && destPort == g_appBPort_SPQ)
      {
        title = "Low Priority (Port " + std::to_string(destPort) + ")";
        color = "blue";
        plotThisFlow = true;
      }
      else if (g_appAPort_SPQ != 0 && destPort == g_appAPort_SPQ)
      {
        title = "High Priority (Port " + std::to_string(destPort) + ")";
        color = "red";
        plotThisFlow = true;
      }
    }
    else
    {
      if (g_appAPort_DRR != 0 && destPort == g_appAPort_DRR)
      {
        title = "DRR W3 (Port " + std::to_string(destPort) + ")";
        color = "red";
        plotThisFlow = true;
      }
      else if (g_appBPort_DRR != 0 && destPort == g_appBPort_DRR)
      {
        title = "DRR W2 (Port " + std::to_string(destPort) + ")";
        color = "blue";
        plotThisFlow = true;
      }
      else if (g_appCPort_DRR != 0 && destPort == g_appCPort_DRR)
      {
        title = "DRR W1 (Port " + std::to_string(destPort) + ")";
        color = "green";
        plotThisFlow = true;
      }
    }

    if (!plotThisFlow)
    {
      NS_LOG_DEBUG(
          "Skipping flow "
          << flowId << " to port " << destPort
          << " as it's not explicitly handled for plotting this scenario.");
      continue;
    }

    dataset.SetTitle(title);
    dataset.SetStyle(Gnuplot2dDataset::LINES);
    dataset.SetExtra("lw 2 lc rgb '" + color + "'");

    dataset.Add(0.0, 0.0);
    double prev_bin_end_time = 0.0;
    double prev_bin_value = 0.0;

    if (timeSeriesData.empty())
    {
      NS_LOG_DEBUG(
          "Flow " << title
                  << " has no time series data. Plotting as zero line.");
      dataset.Add(g_simDuration, 0.0);
    }
    else
    {
      for (auto const& [current_bin_end_time, current_bin_value] :
           timeSeriesData)
      {
        if (current_bin_end_time > prev_bin_end_time)
        {
          double step_time = current_bin_end_time - 0.00001;
          if (step_time > prev_bin_end_time)
          {
            dataset.Add(step_time, prev_bin_value);
          }
          else
          {
            dataset.Add(prev_bin_end_time, prev_bin_value);
          }
        }
        else if (prev_bin_end_time == 0.0 && prev_bin_value == 0.0 &&
                 current_bin_end_time > 0.0)
        {
          dataset.Add(current_bin_end_time - 0.00001, 0.0);
        }
        dataset.Add(current_bin_end_time, current_bin_value);

        prev_bin_end_time = current_bin_end_time;
        prev_bin_value = current_bin_value;
      }
    }

    if (prev_bin_end_time < g_simDuration)
    {
      dataset.Add(g_simDuration, prev_bin_value);
    }
    plot.AddDataset(dataset);
  }

  std::ofstream plotGenFile((filename + ".plt").c_str());
  plot.GenerateOutput(plotGenFile);
  plotGenFile.close();

  std::string cmd = "gnuplot \"" + filename + ".plt\"";
  NS_LOG_INFO("Running gnuplot command: " << cmd);
  if (system(cmd.c_str()) != 0)
  {
    std::cerr << "Failed to run gnuplot command. Check if gnuplot is installed "
                 "and in PATH."
              << std::endl;
    std::cerr << "Plot file is: " << filename << ".plt" << std::endl;
  }
  else
  {
    std::cout << "Generated plot: " << filename << ".png" << std::endl;
  }
}

int main(int argc, char* argv[])
{
  // LogComponentEnable("DiffServSimulation", LOG_LEVEL_INFO);
  // LogComponentEnable("SPQ", LOG_LEVEL_DEBUG);
  // LogComponentEnable("DRR", LOG_LEVEL_DEBUG);
  // LogComponentEnable("DiffServ", LOG_LEVEL_DEBUG);
  // LogComponentEnable("TrafficClass", LOG_LEVEL_DEBUG);
  // LogComponentEnable("Filter", LOG_LEVEL_DEBUG);

  std::string mode = "spq";
  std::string configFile = "";
  bool useCiscoConfig = false;

  CommandLine cmd(__FILE__);
  cmd.AddValue("mode", "Simulation mode (spq or drr)", mode);
  cmd.AddValue("config",
               "Configuration file (e.g., for DRR, or optional for SPQ)",
               configFile);
  cmd.AddValue("cisco", "Use Cisco configuration format for SPQ (extra credit)",
               useCiscoConfig);
  cmd.AddValue("simTime", "Total simulation time in seconds", g_simDuration);
  cmd.AddValue("plotInterval", "Interval for collecting plot data in seconds",
               g_plotBinInterval);
  cmd.Parse(argc, argv);

  std::string spq_default_config_content = "2\n0\n1\n";
  std::string drr_default_config_content = "3\n300\n200\n100\n";
  Packet::EnablePrinting();
  PacketMetadata::Enable();
  if (mode == "spq")
  {
    if (configFile.empty() && !useCiscoConfig)
    {
      configFile = "spq_default.conf";
      WriteDefaultConfigFile(configFile, spq_default_config_content);
      NS_LOG_INFO("SPQ mode: no config file specified. Using default '"
                  << configFile << "'.");
    }
    else if (configFile.empty() && useCiscoConfig)
    {
      NS_LOG_ERROR("SPQ Cisco mode: config file must be specified.");
      return 1;
    }
  }
  else if (mode == "drr")
  {
    if (configFile.empty())
    {
      configFile = "drr_default.conf";
      WriteDefaultConfigFile(configFile, drr_default_config_content);
      NS_LOG_WARN("DRR mode: no config file specified. Using default '"
                  << configFile << "'.");
    }
  }

  NodeContainer allNodes;
  NetDeviceContainer p2pDevices;
  InternetStackHelper internetStack;
  Ipv4AddressHelper ipv4Address;

  Ipv4InterfaceContainer routerIfs;
  Ipv4InterfaceContainer sourceHostIf;

  Ipv4InterfaceContainer sinkHostIf;

  CreateTopology(allNodes, p2pDevices, internetStack, ipv4Address, routerIfs,
                 sourceHostIf, sinkHostIf);
  NS_ASSERT_MSG(allNodes.GetN() > 0, "Nodes not created in CreateTopology.");

  PointToPointHelper p2pHelperForPcap;
  Ptr<Node> routerNode = allNodes.Get(1);
  NS_ASSERT_MSG(routerNode, "PCAP Setup: Failed to get router node (Node 1).");

  Ptr<NetDevice> routerIngressNetDevice = routerNode->GetDevice(0);
  Ptr<NetDevice> routerEgressNetDevice = routerNode->GetDevice(1);

  NS_ASSERT_MSG(routerIngressNetDevice,
                "Router ingress NetDevice (for Pre-QoS PCAP) not found.");
  NS_ASSERT_MSG(routerEgressNetDevice,
                "Router egress NetDevice (for Post-QoS PCAP) not found.");

  std::string preQosPcapFilename;
  std::string postQosPcapFilename;

  if (mode == "spq")
  {
    preQosPcapFilename = "PreSPQ";
    postQosPcapFilename = "PostSPQ";
    NS_LOG_INFO("Enabling PCAP for SPQ: " << preQosPcapFilename << ".pcap and "
                                          << postQosPcapFilename << ".pcap");
  }
  else if (mode == "drr")
  {
    preQosPcapFilename = "PreDRR";
    postQosPcapFilename = "PostDRR";
    NS_LOG_INFO("Enabling PCAP for DRR: " << preQosPcapFilename << ".pcap and "
                                          << postQosPcapFilename << ".pcap");
  }
  else
  {
    NS_LOG_WARN("Unknown mode for PCAP setup: "
                << mode << ". PCAP tracing will not be enabled.");
  }

  if (!preQosPcapFilename.empty() && !postQosPcapFilename.empty())
  {

    p2pHelperForPcap.EnablePcap(preQosPcapFilename, routerIngressNetDevice,
                                true, true);
    p2pHelperForPcap.EnablePcap(postQosPcapFilename, routerEgressNetDevice,
                                true, true);
  }

  ApplicationContainer allApps;
  Ptr<FlowMonitor> flowMonInstance;

  if (mode == "spq")
  {
    SetupSPQValidation(allNodes, sinkHostIf, configFile, allApps,
                       flowMonInstance, g_flowHelper, useCiscoConfig);
  }
  else if (mode == "drr")
  {
    SetupDRRValidation(allNodes, sinkHostIf, configFile, allApps,
                       flowMonInstance, g_flowHelper);
  }
  else
  {
    NS_FATAL_ERROR("Unknown mode: " << mode);
    return 1;
  }

  NS_ASSERT_MSG(
      flowMonInstance,
      "FlowMonitor instance is null after setup. Check InstallAll call.");

  Ptr<Ipv4FlowClassifier> classifier =
      DynamicCast<Ipv4FlowClassifier>(g_flowHelper.GetClassifier());
  NS_ASSERT_MSG(classifier, "Main: FlowMonitorHelper does not have an "
                            "Ipv4FlowClassifier after InstallAll.");

  for (double t = g_plotBinInterval;
       t <= g_simDuration + (g_plotBinInterval / 2.0); t += g_plotBinInterval)
  {
    Simulator::Schedule(Seconds(t), &RecordPeriodicStats, flowMonInstance,
                        classifier, (mode == "spq"));
  }

  Simulator::Stop(Seconds(g_simDuration));
  NS_LOG_INFO("Starting simulation for " << g_simDuration
                                         << " seconds with plot interval "
                                         << g_plotBinInterval << "s...");
  Simulator::Run();
  NS_LOG_INFO("Simulation finished.");

  if (flowMonInstance)
  {
    flowMonInstance->CheckForLostPackets();
    flowMonInstance->SerializeToXmlFile("flowmonitor_final.xml", true, true);
  }
  RecordPeriodicStats(flowMonInstance, classifier, (mode == "spq"));

  std::string plotFileTag = mode;
  if (mode == "spq" && useCiscoConfig)
  {
    plotFileTag += "-cisco";
  }
  GenerateThroughputPlot(g_flowHelper, plotFileTag + "-throughput",
                         (mode == "spq"));

  Simulator::Destroy();
  NS_LOG_INFO("Simulation destroyed.");
  return 0;
}
