// drr.cc
#include "drr.h"
#include "traffic-class.h"
#include "ns3/log.h"
#include "ns3/string.h"
#include <fstream>
#include <sstream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DRR");
NS_OBJECT_ENSURE_REGISTERED (DRR);

TypeId
DRR::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DRR")
    .SetParent<DiffServ> ()
    .SetGroupName ("Network")
    .AddConstructor<DRR> ()
    .AddAttribute ("ConfigFile",
                   "The configuration file for DRR",
                   StringValue (""),
                   MakeStringAccessor (&DRR::SetConfigFile),
                   MakeStringChecker ())
  ;
  return tid;
}

DRR::DRR () :
  DiffServ (),
  m_deficits (),
  m_lastServed (0)
{
  NS_LOG_FUNCTION (this);
}

DRR::~DRR ()
{
  NS_LOG_FUNCTION (this);
}

void
DRR::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_deficits.clear ();
  DiffServ::DoDispose ();
}

Ptr<Packet>
DRR::Schedule (void)
{
  NS_LOG_FUNCTION (this);
  
  uint32_t queueCount = GetNTrafficClasses ();
  
  if (queueCount == 0)
    {
      NS_LOG_LOGIC ("No traffic classes available");
      return 0;
    }
  
  // Check all queues starting from the one after the last served
  for (uint32_t i = 0; i < queueCount; i++)
    {
      // Move to the next queue in a round-robin fashion
      m_lastServed = (m_lastServed + 1) % queueCount;
      
      Ptr<TrafficClass> tClass = GetTrafficClass (m_lastServed);
      
      // Skip empty queues
      if (tClass->IsEmpty ())
        {
          NS_LOG_LOGIC ("Queue " << m_lastServed << " is empty, skipping");
          continue;
        }
      
      // Get the quantum (weight) for this queue
      uint32_t quantum = static_cast<uint32_t>(tClass->GetWeight ());
      
      // Add the quantum to the deficit counter
      m_deficits[m_lastServed] += quantum;
      
      NS_LOG_LOGIC ("Queue " << m_lastServed << ", deficit = " << m_deficits[m_lastServed]);
      
      // Attempt to dequeue packets as long as deficit allows
      while (!tClass->IsEmpty ())
        {
          // Peek at the next packet to get its size
          Ptr<Packet> p = tClass->Peek ();
          uint32_t packetSize = p->GetSize ();
          
          // If the deficit is less than the packet size, move to the next queue
          if (m_deficits[m_lastServed] < packetSize)
            {
              NS_LOG_LOGIC ("Deficit " << m_deficits[m_lastServed] << " < packet size " << packetSize << ", moving to next queue");
              break;
            }
          
          // Dequeue the packet and decrease the deficit
          p = tClass->Dequeue ();
          m_deficits[m_lastServed] -= packetSize;
          
          NS_LOG_LOGIC ("Dequeued packet of size " << packetSize << ", remaining deficit = " << m_deficits[m_lastServed]);
          return p;
        }
    }
  
  NS_LOG_LOGIC ("No packet found in scheduling");
  return 0;
}

bool
DRR::SetConfigFile (std::string filename)
{
  NS_LOG_FUNCTION (this << filename);
  
  std::ifstream file (filename.c_str ());
  if (!file.is_open ())
    {
      NS_LOG_ERROR ("Failed to open file " << filename);
      return false;
    }
  
  std::string line;
  uint32_t numQueues = 0;
  
  // Read the number of queues
  if (std::getline (file, line))
    {
      std::istringstream iss (line);
      if (!(iss >> numQueues))
        {
          NS_LOG_ERROR ("Invalid number of queues");
          file.close ();
          return false;
        }
    }
  
  // Initialize deficit counters
  m_deficits.resize (numQueues, 0);
  
  // Read the quantum (weight) for each queue
  for (uint32_t i = 0; i < numQueues; i++)
    {
      if (std::getline (file, line))
        {
          std::istringstream iss (line);
          uint32_t quantum;
          
          if (!(iss >> quantum))
            {
              NS_LOG_ERROR ("Invalid quantum value for queue " << i);
              file.close ();
              return false;
            }
          
          // Create a new traffic class with the specified quantum as weight
          Ptr<TrafficClass> tClass = CreateObject<TrafficClass> ();
          tClass->SetWeight (quantum);
          
          // Add the traffic class to the DiffServ queue
          AddTrafficClass (tClass);
          
          NS_LOG_INFO ("Added traffic class " << i << " with quantum " << quantum);
        }
      else
        {
          NS_LOG_ERROR ("Not enough quantum values specified");
          file.close ();
          return false;
        }
    }
  
  file.close ();
  return true;