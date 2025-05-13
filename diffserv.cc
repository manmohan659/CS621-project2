#include "diffserv.h"
#include "ns3/enum.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/uinteger.h"
#include "traffic-class.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("DiffServ");
NS_OBJECT_ENSURE_REGISTERED(DiffServ);

TypeId DiffServ::GetTypeId(void)
{
  static TypeId tid =
      TypeId("ns3::DiffServ")
          .SetParent<Queue<Packet>>()
          .SetGroupName("Network")
          .AddAttribute(
              "MaxSize",
              "The maximum number of packets accepted by this DiffServ queue.",
              QueueSizeValue(QueueSize("100p")),
              MakeQueueSizeAccessor(&QueueBase::SetMaxSize,
                                    &QueueBase::GetMaxSize),
              MakeQueueSizeChecker());
  return tid;
}

DiffServ::DiffServ() : Queue<Packet>(), m_classes()
{
  NS_LOG_FUNCTION(this);
}

DiffServ::~DiffServ()
{
  NS_LOG_FUNCTION(this);
}

void DiffServ::DoDispose(void)
{
  NS_LOG_FUNCTION(this);
  m_classes.clear();
  Queue<Packet>::DoDispose();
}

bool DiffServ::DoEnqueue(Ptr<Packet> p)
{
  NS_LOG_FUNCTION(this << p);
  // std::cout << "DiffServ::DoEnqueue" << std::endl;

  if (GetNPackets() >= GetMaxSize().GetValue())
  {
    NS_LOG_LOGIC("Queue full -- dropping packet");
    return false;
  }

  uint32_t classIndex = Classify(p);
  if (classIndex >= m_classes.size())
  {
    NS_LOG_LOGIC("No matching traffic class, using default (0)");
    classIndex = 0;
  }

  if (m_classes[classIndex]->Enqueue(p))
  {
    NS_LOG_LOGIC("Packet enqueued in traffic class " << classIndex);
    return true;
  }
  return false;
}

Ptr<Packet> DiffServ::DoDequeue(void)
{
  NS_LOG_FUNCTION(this);

  if (IsEmpty())
  {
    NS_LOG_LOGIC("Queue empty");
    return 0;
  }

  Ptr<Packet> p = Schedule();
  if (p)
  {
    NS_LOG_LOGIC("Packet dequeued");
  }
  return p;
}

Ptr<Packet> DiffServ::DoPeek(void) const
{
  NS_LOG_FUNCTION(this);

  if (IsEmpty())
  {
    NS_LOG_LOGIC("Queue empty");
    return 0;
  }

  for (uint32_t i = 0; i < m_classes.size(); i++)
  {
    if (!m_classes[i]->IsEmpty())
    {
      NS_LOG_LOGIC("Peeking from traffic class " << i);
      return m_classes[i]->Peek();
    }
  }

  NS_LOG_LOGIC("No packet found in peek");
  return 0;
}

bool DiffServ::IsEmpty(void) const
{
  NS_LOG_FUNCTION(this);

  for (uint32_t i = 0; i < m_classes.size(); i++)
  {
    if (!m_classes[i]->IsEmpty())
    {
      NS_LOG_LOGIC("Traffic class " << i << " is not empty");
      return false;
    }
  }

  NS_LOG_LOGIC("All traffic classes are empty");
  return true;
}

Ptr<Packet> DiffServ::Schedule(void)
{
  // std::cout << "DiffServ::Schedule" << std::endl;
  NS_LOG_FUNCTION(this);

  for (uint32_t i = 0; i < m_classes.size(); i++)
  {
    if (!m_classes[i]->IsEmpty())
    {
      NS_LOG_LOGIC("Scheduling from traffic class " << i);
      return m_classes[i]->Dequeue();
    }
  }

  NS_LOG_LOGIC("No packet found in scheduling");
  return 0;
}

uint32_t DiffServ::Classify(Ptr<Packet> p)
{
  // std::cout << "DiffServ::Classify" << std::endl;
  NS_LOG_FUNCTION(this << p);

  for (uint32_t i = 0; i < m_classes.size(); i++)
  {
    if (m_classes[i]->Match(p))
    {
      NS_LOG_LOGIC("Packet matches traffic class " << i);
      // std::cout << "Packet matches traffic class " << i << std::endl;
      return i;
    }
  }

  NS_LOG_LOGIC("No matching traffic class, using default (0)");
  // std::cout << "No matching traffic class, using default (0)" << std::endl;
  return 0;
}

void DiffServ::AddTrafficClass(Ptr<TrafficClass> tClass)
{
  NS_LOG_FUNCTION(this << tClass);
  m_classes.push_back(tClass);
}

Ptr<TrafficClass> DiffServ::GetTrafficClass(uint32_t index) const
{
  NS_LOG_FUNCTION(this << index);
  if (index < m_classes.size())
  {
    return m_classes[index];
  }
  return 0;
}

uint32_t DiffServ::GetNTrafficClasses(void) const
{
  NS_LOG_FUNCTION(this);
  return m_classes.size();
}

bool DiffServ::Enqueue(Ptr<Packet> p)
{
  NS_LOG_FUNCTION(this << p);
  return DoEnqueue(p);
}

Ptr<Packet> DiffServ::Dequeue(void)
{
  NS_LOG_FUNCTION(this);
  return DoDequeue();
}

Ptr<Packet> DiffServ::Remove(void)
{
  NS_LOG_FUNCTION(this);
  return Schedule();
}

Ptr<const Packet> DiffServ::Peek(void) const
{
  NS_LOG_FUNCTION(this);
  return DoPeek();
}

} // namespace ns3
