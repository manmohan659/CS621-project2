// spq.cc
#include "spq.h"
#include "cisco-parser.h"
#include "ns3/log.h"
#include "ns3/string.h"
#include "traffic-class.h"
#include <fstream>
#include <limits>
#include <sstream>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SPQ");
NS_OBJECT_ENSURE_REGISTERED(SPQ);

TypeId SPQ::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::SPQ")
                          .SetParent<DiffServ>()
                          .SetGroupName("Network")
                          .AddConstructor<SPQ>();
  return tid;
}

SPQ::SPQ() : DiffServ(), m_configFile(""), m_ciscoConfigFile("")
{
  NS_LOG_FUNCTION(this);
}

SPQ::~SPQ()
{
  NS_LOG_FUNCTION(this);
}

void SPQ::DoDispose(void)
{
  NS_LOG_FUNCTION(this);
  DiffServ::DoDispose();
}

std::string SPQ::GetConfigFile(void) const
{
  NS_LOG_FUNCTION(this);
  return m_configFile;
}

std::string SPQ::GetCiscoConfigFile(void) const
{
  NS_LOG_FUNCTION(this);
  return m_ciscoConfigFile;
}

Ptr<Packet> SPQ::Schedule(void)
{
  NS_LOG_FUNCTION(this);

  // Serve the highest priority queue that has packets
  // Lower value of priority means higher priority
  uint32_t highestPriority = std::numeric_limits<uint32_t>::max();
  int32_t selectedIndex = -1;

  // Find the highest priority queue with packets
  for (uint32_t i = 0; i < GetNTrafficClasses(); i++)
  {
    Ptr<TrafficClass> tClass = GetTrafficClass(i);

    if (!tClass->IsEmpty())
    {
      uint32_t priority = tClass->GetPriorityLevel();
      if (priority < highestPriority)
      {
        highestPriority = priority;
        selectedIndex = i;
      }
    }
  }

  // Return a packet from the selected queue
  if (selectedIndex >= 0)
  {
    NS_LOG_LOGIC("Serving traffic class " << selectedIndex << " with priority "
                                          << highestPriority);
    return GetTrafficClass(selectedIndex)->Dequeue();
  }

  NS_LOG_LOGIC("No packet found in scheduling");
  return 0;
}

bool SPQ::SetConfigFile(std::string filename)
{
  NS_LOG_FUNCTION(this << filename);
  m_configFile = filename;

  std::ifstream file(filename.c_str());
  if (!file.is_open())
  {
    NS_LOG_ERROR("Failed to open file " << filename);
    return false;
  }

  std::string line;
  uint32_t numQueues = 0;

  // Read the number of queues
  if (std::getline(file, line))
  {
    std::istringstream iss(line);
    if (!(iss >> numQueues))
    {
      NS_LOG_ERROR("Invalid number of queues");
      file.close();
      return false;
    }
  }

  // Read the priority level for each queue
  for (uint32_t i = 0; i < numQueues; i++)
  {
    if (std::getline(file, line))
    {
      std::istringstream iss(line);
      uint32_t priority;

      if (!(iss >> priority))
      {
        NS_LOG_ERROR("Invalid priority level for queue " << i);
        file.close();
        return false;
      }

      // Create a new traffic class with the specified priority
      Ptr<TrafficClass> tClass = CreateObject<TrafficClass>();
      tClass->SetPriorityLevel(priority);

      // Add the traffic class to the DiffServ queue
      AddTrafficClass(tClass);

      NS_LOG_INFO("Added traffic class " << i << " with priority " << priority);
    }
    else
    {
      NS_LOG_ERROR("Not enough priority levels specified");
      file.close();
      return false;
    }
  }

  file.close();
  return true;
}

bool SPQ::SetCiscoConfigFile(std::string filename)
{
  NS_LOG_FUNCTION(this << filename);
  m_ciscoConfigFile = filename;

  // Create the parser
  Ptr<CiscoParser> parser = CreateObject<CiscoParser>();

  // Parse the configuration file
  uint32_t numQueues;
  std::vector<uint32_t> priorities;

  if (!parser->Parse(filename, numQueues, priorities))
  {
    NS_LOG_ERROR("Failed to parse Cisco configuration file " << filename);
    return false;
  }

  // Create traffic classes based on the parsed configuration
  for (uint32_t i = 0; i < numQueues; i++)
  {
    Ptr<TrafficClass> tClass = CreateObject<TrafficClass>();
    tClass->SetPriorityLevel(priorities[i]);

    // Add the traffic class to the DiffServ queue
    AddTrafficClass(tClass);

    NS_LOG_INFO("Added traffic class " << i << " with priority "
                                       << priorities[i]);
  }

  return true;
}

} // namespace ns3