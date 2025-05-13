#include "drr.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "traffic-class.h"
#include <fstream>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("DRR");
NS_OBJECT_ENSURE_REGISTERED(DRR);

TypeId DRR::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::DRR")
                          .SetParent<DiffServ>()
                          .SetGroupName("Network")
                          .AddConstructor<DRR>();
  return tid;
}

DRR::DRR() : m_lastQueueServed(0)
{
  NS_LOG_FUNCTION(this);
}

DRR::~DRR()
{
  NS_LOG_FUNCTION(this);
}

void DRR::DoDispose(void)
{
  NS_LOG_FUNCTION(this);
  m_deficits.clear();
  m_quantums.clear();
  DiffServ::DoDispose();
}

bool DRR::SetConfigFile(std::string filename)
{
  NS_LOG_FUNCTION(this << filename);
  LogComponentEnable("DRR", LOG_LEVEL_ALL);
  m_configFile = filename;
  std::ifstream configFileStream(filename.c_str());
  if (!configFileStream.is_open())
  {
    NS_LOG_ERROR("DRR: Can't open DRR config file: " << filename);
    return false;
  }

  uint32_t numQueuesFromFile;
  configFileStream >> numQueuesFromFile;
  if (configFileStream.fail() || numQueuesFromFile == 0)
  {
    NS_LOG_ERROR("DRR: Invalid number of queues in DRR config file: "
                 << filename << ". Read: " << numQueuesFromFile);
    configFileStream.close();
    return false;
  }

  if (GetNTrafficClasses() != 0 && GetNTrafficClasses() != numQueuesFromFile)
  {
    NS_LOG_WARN("DRR: Reconfiguring with a different number of queues ("
                << numQueuesFromFile << " vs " << GetNTrafficClasses()
                << "). "
                   "Behavior depends on DiffServ base class's management of "
                   "TrafficClasses.");
  }

  for (uint32_t i = GetNTrafficClasses(); i < numQueuesFromFile; ++i)
  {
    AddTrafficClass(CreateObject<TrafficClass>());
  }

  if (GetNTrafficClasses() < numQueuesFromFile)
  {
    NS_LOG_ERROR("DRR: Failed to ensure enough TrafficClass objects in base "
                 "DiffServ. Expected "
                 << numQueuesFromFile << ", but base has "
                 << GetNTrafficClasses());
    configFileStream.close();
    return false;
  }

  m_quantums.resize(numQueuesFromFile);
  m_deficits.resize(numQueuesFromFile);

  NS_LOG_INFO("DRR: Configuring " << numQueuesFromFile << " queues.");
  for (uint32_t i = 0; i < numQueuesFromFile; ++i)
  {
    configFileStream >> m_quantums[i];
    std::cout << "m_quantums[i]: " << m_quantums[i] << std::endl;
    if (configFileStream.fail() || m_quantums[i] == 0)
    {
      NS_LOG_ERROR("DRR: Invalid quantum for queue "
                   << i << " in DRR config file: " << filename);
      configFileStream.close();
      return false;
    }
    m_deficits[i] = 0;
    NS_LOG_INFO("DRR: Queue " << i << " - Quantum: " << m_quantums[i]
                              << ", Initial Deficit: " << m_deficits[i]);
  }
  configFileStream.close();

  m_lastQueueServed = (numQueuesFromFile > 0) ? (numQueuesFromFile - 1) : 0;

  NS_LOG_INFO("DRR: Configuration loaded successfully from " << filename);
  return true;
}

Ptr<Packet> DRR::Schedule(void)
{
  std::cout << "DRR::Schedule" << std::endl;
  NS_LOG_FUNCTION(this);
  uint32_t numManagedQueues = m_quantums.size();

  if (numManagedQueues == 0)
  {
    NS_LOG_LOGIC("DRR: No queues managed. Nothing to schedule.");
    return nullptr;
  }
  if (GetNTrafficClasses() < numManagedQueues)
  {
    NS_LOG_WARN("DRR: Mismatch between configured queues ("
                << numManagedQueues
                << ") and available TrafficClasses in base DiffServ ("
                << GetNTrafficClasses() << "). Cannot schedule.");
    return nullptr;
  }

  for (uint32_t i = 0; i < numManagedQueues; ++i)
  {
    uint32_t currentQueueIndex = (m_lastQueueServed + 1 + i) % numManagedQueues;

    Ptr<TrafficClass> tc = GetTrafficClass(currentQueueIndex);
    if (!tc)
    {
      NS_LOG_WARN("DRR: TrafficClass for queue "
                  << currentQueueIndex << " is unexpectedly null. Skipping.");
      continue;
    }

    if (tc->IsEmpty())
    {
      NS_LOG_LOGIC("DRR: Queue " << currentQueueIndex
                                 << " is empty. Skipping.");
      continue;
    }

    // test code. remove later. //todo
    if (!tc->IsEmpty())
    {
      m_lastQueueServed = currentQueueIndex;
      return tc->Dequeue();
    }

    m_deficits[currentQueueIndex] += m_quantums[currentQueueIndex];
    NS_LOG_DEBUG("DRR: Queue " << currentQueueIndex
                               << " gets turn. Prior Deficit: "
                               << (m_deficits[currentQueueIndex] -
                                   m_quantums[currentQueueIndex])
                               << ", Quantum: " << m_quantums[currentQueueIndex]
                               << ". Total Deficit for round: "
                               << m_deficits[currentQueueIndex]);

    if (!tc->IsEmpty() && m_deficits[currentQueueIndex] > 0)
    {
      Ptr<Packet> packetToPeek = tc->Peek();
      NS_ASSERT(packetToPeek);
      uint32_t packetSize = packetToPeek->GetSize();

      NS_LOG_DEBUG("DRR: Queue "
                   << currentQueueIndex << " Peeked packet size: " << packetSize
                   << "B. Deficit: " << m_deficits[currentQueueIndex]);

      if (packetSize <= m_deficits[currentQueueIndex])
      {
        Ptr<Packet> packetToSend = tc->Dequeue();
        m_deficits[currentQueueIndex] -= packetSize;

        NS_LOG_INFO("DRR: Dequeued packet (size "
                    << packetSize << "B) from queue " << currentQueueIndex
                    << ". Deficit remaining: "
                    << m_deficits[currentQueueIndex]);

        m_lastQueueServed = currentQueueIndex;

        // if (tc->IsEmpty())
        // {
        //   NS_LOG_DEBUG("DRR: Queue "
        //                << currentQueueIndex
        //                << " is now empty. Resetting deficit to 0.");
        //   m_deficits[currentQueueIndex] = 0;
        // }
        return packetToSend;
      }
      else
      {

        NS_LOG_DEBUG(
            "DRR: Queue "
            << currentQueueIndex << " head packet (size " << packetSize
            << "B) > deficit (" << m_deficits[currentQueueIndex]
            << "). No packet sent from this queue. Deficit carried over.");
      }
    }
    else
    {
      NS_LOG_DEBUG(
          "DRR: Queue "
          << currentQueueIndex
          << " could not send (e.g. became empty or non-positive deficit "
          << m_deficits[currentQueueIndex] << " before sending).");
    }
  }

  NS_LOG_LOGIC("DRR: No packet could be scheduled in this full scan of "
               << numManagedQueues << " queues.");
  return nullptr;
}

} // namespace ns3
