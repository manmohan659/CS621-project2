#ifndef FILTER_ELEMENT_H
#define FILTER_ELEMENT_H

#include "ns3/object.h"
#include "ns3/packet.h"

namespace ns3
{

/**
 * \ingroup diffserv
 * \brief Base class for filter elements
 */
class FilterElement : public Object
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
  FilterElement();

  /**
   * \brief Destructor
   */
  virtual ~FilterElement();

  /**
   * \brief Check if packet matches this filter element
   * \param p The packet to check
   * \return True if the packet matches, false otherwise
   */
  virtual bool Match(Ptr<Packet> p) = 0;

protected:
  /**
   * \brief Dispose of the object
   */
  virtual void DoDispose(void);
};

}

#endif