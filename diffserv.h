#ifndef DIFFSERV_H
#define DIFFSERV_H

#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/queue.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/traced-value.h"
#include <vector>

namespace ns3
{

class TrafficClass;

/**
 * \ingroup queue
 * \brief A DiffServ queue
 */
class DiffServ : public Queue<Packet>
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId(void);

  /**
   * \brief Constructor
   */
  DiffServ();

  /**
   * \brief Destructor
   */
  virtual ~DiffServ();

  /**
   * \brief Schedule which queue to serve next
   * \return The selected packet
   */
  virtual Ptr<Packet> Schedule(void);

  /**
   * \brief Classify incoming packet to the appropriate queue
   * \param p The packet to classify
   * \return Queue index where the packet belongs
   */
  virtual uint32_t Classify(Ptr<Packet> p);

  /**
   * \brief Add a traffic class to this DiffServ queue
   * \param tClass The traffic class to add
   */
  void AddTrafficClass(Ptr<TrafficClass> tClass);

  /**
   * \brief Get traffic class by index
   * \param index The index of the traffic class
   * \return The traffic class at the specified index
   */
  Ptr<TrafficClass> GetTrafficClass(uint32_t index) const;

  virtual bool Enqueue(Ptr<Packet> p) override;
  virtual Ptr<Packet> Dequeue(void) override;
  virtual Ptr<Packet> Remove(void) override;
  virtual Ptr<const Packet> Peek(void) const override;
  /**
   * \brief Get the number of traffic classes
   * \return The number of traffic classes
   */
  uint32_t GetNTrafficClasses(void) const;

protected:
  /**
   * \brief Dispose of the object
   */
  virtual void DoDispose(void);

  /**
   * \brief Check if the queue is empty
   * \return True if the queue is empty, false otherwise
   */
  virtual bool DoEnqueue(Ptr<Packet> p);

  /**
   * \brief Dequeue a packet from the queue
   * \return The dequeued packet
   */
  virtual Ptr<Packet> DoDequeue(void);

  /**
   * \brief Peek at the next packet to be dequeued
   * \return The packet that would be dequeued next
   */
  virtual Ptr<Packet> DoPeek(void) const;

  /**
   * \brief Check if the queue is empty
   * \return True if the queue is empty, false otherwise
   */
  virtual bool IsEmpty(void) const;

  std::vector<Ptr<TrafficClass>> m_classes;
};

}

#endif
