// drr.h
#ifndef DRR_H
#define DRR_H

#include "diffserv.h"
#include <string>
#include <vector>

namespace ns3 {

/**
 * \ingroup queue
 * \brief A Deficit Round Robin (DRR) implementation
 *
 * This class implements Deficit Round Robin scheduling discipline
 * on top of the DiffServ base class.
 */
class DRR : public DiffServ
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
  DRR ();
  
  /**
   * \brief Destructor
   */
  virtual ~DRR ();
  
  /**
   * \brief Schedule which queue to serve next
   * \return The selected packet
   *
   * This method implements the Deficit Round Robin discipline.
   */
  virtual Ptr<Packet> Schedule (void);
  
  /**
   * \brief Set configuration from file
   * \param filename The configuration file
   * \return true if successful, false otherwise
   */
  bool SetConfigFile (std::string filename);

protected:
  /**
   * \brief Dispose of the object
   */
  virtual void DoDispose (void);

private:
  std::vector<uint32_t> m_deficits; //!< Deficit counters for each queue
  uint32_t m_lastServed;           //!< Index of the last served queue
};

} // namespace ns3

#endif /* DRR_H */

