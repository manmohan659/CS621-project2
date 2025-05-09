// source-ip-address.h
#ifndef SOURCE_IP_ADDRESS_H
#define SOURCE_IP_ADDRESS_H

#include "filter-element.h"
#include "ns3/ipv4-address.h"

namespace ns3
{

/**
 * \brief Filter element for source IP address
 */
class SourceIpAddress : public FilterElement
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
  SourceIpAddress();

  /**
   * \brief Constructor with IP address
   * \param addr The source IP address to match
   */
  SourceIpAddress(Ipv4Address addr);

  /**
   * \brief Destructor
   */
  virtual ~SourceIpAddress();

  /**
   * \brief Check if a packet matches this filter element
   * \param p The packet to check
   * \return True if the packet matches this filter element
   */
  virtual bool Match(Ptr<Packet> p);

  /**
   * \brief Set the source IP address to match
   * \param addr The source IP address
   */
  void SetAddress(Ipv4Address addr);

  /**
   * \brief Get the source IP address to match
   * \return The source IP address
   */
  Ipv4Address GetAddress(void) const;

private:
  Ipv4Address m_address; //!< The source IP address to match

  /**
   * \brief Dispose of the object
   */
  virtual void DoDispose(void);
};

} // namespace ns3

#endif /* SOURCE_IP_ADDRESS_H */