// traffic-class.h
#ifndef TRAFFIC_CLASS_H
#define TRAFFIC_CLASS_H

#include "ns3/packet.h"
#include "ns3/object.h"
#include "ns3/queue.h"
#include <vector>
#include <queue>

namespace ns3 {

class Filter;

/**
 * \brief Traffic class for differentiated services
 *
 * This class represents a traffic class in DiffServ architecture.
 * It contains a queue of packets and filters for classification.
 */
class TrafficClass : public Object
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
  TrafficClass ();
  
  /**
   * \brief Destructor
   */
  virtual ~TrafficClass ();
  
  /**
   * \brief Check if a packet matches this traffic class
   * \param p The packet to check
   * \return True if the packet matches this traffic class
   */
  bool Match (Ptr<Packet> p);
  
  /**
   * \brief Enqueue a packet
   * \param p The packet to enqueue
   * \return True if the packet was enqueued
   */
  bool Enqueue (Ptr<Packet> p);
  
  /**
   * \brief Dequeue a packet
   * \return The dequeued packet
   */
  Ptr<Packet> Dequeue (void);
  
  /**
   * \brief Peek at the next packet
   * \return The packet at the front of the queue
   */
  Ptr<Packet> Peek (void) const;
  
  /**
   * \brief Check if the queue is empty
   * \return True if the queue is empty
   */
  bool IsEmpty (void) const;
  
  /**
   * \brief Add a filter to this traffic class
   * \param filter The filter to add
   */
  void AddFilter (Ptr<Filter> filter);
  
  /**
   * \brief Set the priority level
   * \param level The priority level
   */
  void SetPriorityLevel (uint32_t level);
  
  /**
   * \brief Get the priority level
   * \return The priority level
   */
  uint32_t GetPriorityLevel (void) const;
  
  /**
   * \brief Set the weight
   * \param weight The weight
   */
  void SetWeight (double weight);
  
  /**
   * \brief Get the weight
   * \return The weight
   */
  double GetWeight (void) const;
  
  /**
   * \brief Set the maximum number of packets
   * \param maxPackets The maximum number of packets
   */
  void SetMaxPackets (uint32_t maxPackets);
  
  /**
   * \brief Get the maximum number of packets
   * \return The maximum number of packets
   */
  uint32_t GetMaxPackets (void) const;
  
  /**
   * \brief Get the number of packets
   * \return The number of packets
   */
  uint32_t GetNPackets (void) const;

protected:
  /**
   * \brief Dispose of the object
   */
  virtual void DoDispose (void);

private:
  std::vector<Ptr<Filter> > m_filters; //!< Vector of filters
  uint32_t m_mode;                    //!< Queue mode (FIFO, etc.)
  uint32_t m_packets;                 //!< Current number of packets
  uint32_t m_maxPackets;              //!< Maximum number of packets
  double m_weight;                    //!< Weight (for WFQ, DRR, etc.)
  uint32_t m_priorityLevel;           //!< Priority level (for SPQ)
  std::queue<Ptr<Packet> > m_queue;   //!< Internal packet queue
};

} // namespace ns3

#endif /* TRAFFIC_CLASS_H */

