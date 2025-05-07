// spq.h
#ifndef SPQ_H
#define SPQ_H

#include "diffserv.h"
#include <string>

namespace ns3 {

/**
 * \ingroup queue
 * \brief A Strict Priority Queueing (SPQ) implementation
 *
 * This class implements Strict Priority Queueing discipline
 * on top of the DiffServ base class.
 */
class SPQ : public DiffServ
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  
  /**
   * \brief Constructor
   */
  SPQ ();
  
  /**
   * \brief Destructor
   */
  virtual ~SPQ ();
  
  /**
   * \brief Schedule which queue to serve next
   * \return The selected packet
   *
   * This method implements the Strict Priority Queueing discipline.
   * It always serves the highest priority queue that has packets.
   */
  virtual Ptr<Packet> Schedule (void);
  
  /**
   * \brief Set configuration from file
   * \param filename The configuration file
   * \return true if successful, false otherwise
   */
  bool SetConfigFile (std::string filename);
  
  /**
   * \brief Set configuration from Cisco CLI configuration file
   * \param filename The Cisco CLI configuration file
   * \return true if successful, false otherwise
   */
  bool SetCiscoConfigFile (std::string filename);

protected:
  /**
   * \brief Dispose of the object
   */
  virtual void DoDispose (void);
};

} // namespace ns3

#endif /* SPQ_H */

// spq.cc
#include "spq.h"
#include "traffic-class.h"
#include "ns3/log.h"
#include "ns3/string.h"
#include <fstream>
#include <sstream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SPQ");
NS_OBJECT_ENSURE_REGISTERED (SPQ);

TypeId
SPQ::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SPQ")
    .SetParent<DiffServ> ()
    .SetGroupName ("Network")
    .AddConstructor<SPQ> ()
    .AddAttribute ("ConfigFile",
                   "The configuration file for SPQ",
                   StringValue (""),
                   MakeStringAccessor (&SPQ::SetConfigFile),
                   MakeStringChecker ())
  ;
  return tid;
}

SPQ::SPQ () :
  DiffServ ()
{
  NS_LOG_FUNCTION (this);
}

SPQ::~SPQ ()
{
  NS_LOG_FUNCTION (this);
}

void
SPQ::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  DiffServ::DoDispose ();
}

Ptr<Packet>
SPQ::Schedule (void)
{
  NS_LOG_FUNCTION (this);
  
  // Serve the highest priority queue that has packets
  // Lower value of priority means higher priority
  uint32_t highestPriority = std::numeric_limits<uint32_t>::max ();
  int32_t selectedIndex = -1;
  
  // Find the highest priority queue with packets
  for (uint32_t i = 0; i < GetNTrafficClasses (); i++)
    {
      Ptr<TrafficClass> tClass = GetTrafficClass (i);
      
      if (!tClass->IsEmpty ())
        {
          uint32_t priority = tClass->GetPriorityLevel ();
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
      NS_LOG_LOGIC ("Serving traffic class " << selectedIndex << " with priority " << highestPriority);
      return GetTrafficClass (selectedIndex)->Dequeue ();
    }
  
  NS_LOG_LOGIC ("No packet found in scheduling");
  return 0;
}

bool
SPQ::SetConfigFile (std::string filename)
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
  
  // Read the priority level for each queue
  for (uint32_t i = 0; i < numQueues; i++)
    {
      if (std::getline (file, line))
        {
          std::istringstream iss (line);
          uint32_t priority;
          
          if (!(iss >> priority))
            {
              NS_LOG_ERROR ("Invalid priority level for queue " << i);
              file.close ();
              return false;
            }
          
          // Create a new traffic class with the specified priority
          Ptr<TrafficClass> tClass = CreateObject<TrafficClass> ();
          tClass->SetPriorityLevel (priority);
          
          // Add the traffic class to the DiffServ queue
          AddTrafficClass (tClass);
          
          NS_LOG_INFO ("Added traffic class " << i << " with priority " << priority);
        }
      else
        {
          NS_LOG_ERROR ("Not enough priority levels specified");
          file.close ();
          return false;
        }
    }
  
  file.close ();
  return true;
}

} // namespace ns3