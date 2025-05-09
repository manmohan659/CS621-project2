// traffic-class.cc
#include "traffic-class.h"
#include "filter.h"
#include "ns3/log.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TrafficClass");
NS_OBJECT_ENSURE_REGISTERED (TrafficClass);

TypeId
TrafficClass::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TrafficClass")
    .SetParent<Object> ()
    .SetGroupName ("Network")
    .AddConstructor<TrafficClass> ()
    .AddAttribute ("Weight",
                   "The weight of this traffic class (for WFQ, DRR, etc.)",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&TrafficClass::m_weight),
                   MakeDoubleChecker<double> (0.0))
    .AddAttribute ("PriorityLevel",
                   "The priority level of this traffic class (for SPQ)",
                   UintegerValue (0),
                   MakeUintegerAccessor (&TrafficClass::m_priorityLevel),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MaxPackets",
                   "The maximum number of packets allowed in this traffic class",
                   UintegerValue (100),
                   MakeUintegerAccessor (&TrafficClass::m_maxPackets),
                   MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}

TrafficClass::TrafficClass () :
  m_filters (),
  m_mode (0), // Packet mode (was QUEUE_MODE_PACKETS)
  m_packets (0),
  m_maxPackets (100),
  m_weight (1.0),
  m_priorityLevel (0)
{
  NS_LOG_FUNCTION (this);
}

TrafficClass::~TrafficClass ()
{
  NS_LOG_FUNCTION (this);
}

void
TrafficClass::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  
  // Empty the queue
  while (!m_queue.empty ())
    {
      m_queue.pop ();
    }
  
  // Clear the filters
  m_filters.clear ();
  
  Object::DoDispose ();
}

bool
TrafficClass::Match (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);
  
  // If no filters, always match
  if (m_filters.empty ())
    {
      NS_LOG_LOGIC ("No filters, default match");
      return true;
    }
  
  // Check if the packet matches any filter
  for (uint32_t i = 0; i < m_filters.size (); i++)
    {
      if (m_filters[i]->Match (p))
        {
          NS_LOG_LOGIC ("Packet matches filter " << i);
          return true;
        }
    }
  
  NS_LOG_LOGIC ("Packet doesn't match any filter");
  return false;
}

bool
TrafficClass::Enqueue (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);
  
  // Check if the queue is full
  if (m_packets >= m_maxPackets)
    {
      NS_LOG_LOGIC ("Queue full, dropping packet");
      return false;
    }
  
  // Enqueue the packet
  m_queue.push (p);
  m_packets++;
  
  NS_LOG_LOGIC ("Packet enqueued, " << m_packets << " packets in queue");
  return true;
}

Ptr<Packet>
TrafficClass::Dequeue (void)
{
  NS_LOG_FUNCTION (this);
  
  // Check if the queue is empty
  if (m_queue.empty ())
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }
  
  // Dequeue the packet
  Ptr<Packet> p = m_queue.front ();
  m_queue.pop ();
  m_packets--;
  
  NS_LOG_LOGIC ("Packet dequeued, " << m_packets << " packets in queue");
  return p;
}

Ptr<Packet>
TrafficClass::Peek (void) const
{
  NS_LOG_FUNCTION (this);
  
  // Check if the queue is empty
  if (m_queue.empty ())
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }
  
  // Return the first packet without removing it
  return m_queue.front ();
}

bool
TrafficClass::IsEmpty (void) const
{
  NS_LOG_FUNCTION (this);
  return m_queue.empty ();
}

void
TrafficClass::AddFilter (Ptr<Filter> filter)
{
  NS_LOG_FUNCTION (this << filter);
  m_filters.push_back (filter);
}

void
TrafficClass::SetPriorityLevel (uint32_t level)
{
  NS_LOG_FUNCTION (this << level);
  m_priorityLevel = level;
}

uint32_t
TrafficClass::GetPriorityLevel (void) const
{
  NS_LOG_FUNCTION (this);
  return m_priorityLevel;
}

void
TrafficClass::SetWeight (double weight)
{
  NS_LOG_FUNCTION (this << weight);
  m_weight = weight;
}

double
TrafficClass::GetWeight (void) const
{
  NS_LOG_FUNCTION (this);
  return m_weight;
}

void
TrafficClass::SetMaxPackets (uint32_t maxPackets)
{
  NS_LOG_FUNCTION (this << maxPackets);
  m_maxPackets = maxPackets;
}

uint32_t
TrafficClass::GetMaxPackets (void) const
{
  NS_LOG_FUNCTION (this);
  return m_maxPackets;
}

uint32_t
TrafficClass::GetNPackets (void) const
{
  NS_LOG_FUNCTION (this);
  return m_packets;
}

} // namespace ns3