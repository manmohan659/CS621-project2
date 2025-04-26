// filter.h
#ifndef FILTER_H
#define FILTER_H

#include "ns3/packet.h"
#include "ns3/object.h"
#include <vector>

namespace ns3 {

class FilterElement;

/**
 * \brief Filter for packet classification
 *
 * This class represents a filter in DiffServ architecture.
 * It contains a collection of FilterElement conditions.
 * A packet matches a filter only if all FilterElement conditions are satisfied.
 */
class Filter : public Object
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
  Filter ();
  
  /**
   * \brief Destructor
   */
  virtual ~Filter ();
  
  /**
   * \brief Check if a packet matches this filter
   * \param p The packet to check
   * \return True if the packet matches this filter
   */
  bool Match (Ptr<Packet> p);
  
  /**
   * \brief Add a filter element to this filter
   * \param element The filter element to add
   */
  void AddFilterElement (Ptr<FilterElement> element);

protected:
  /**
   * \brief Dispose of the object
   */
  virtual void DoDispose (void);

private:
  std::vector<Ptr<FilterElement> > m_elements; //!< Vector of filter elements
};

} // namespace ns3

#endif /* FILTER_H */