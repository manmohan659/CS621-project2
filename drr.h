#ifndef DRR_H
#define DRR_H

#include "diffserv.h"
#include <string>
#include <vector>

namespace ns3 {

class DRR : public DiffServ
{
public:
  static TypeId GetTypeId (void);
  DRR ();
  virtual ~DRR (); // Ensure destructor is virtual
  virtual Ptr<Packet> Schedule (void) override; // Mark as override

  /**
   * \brief Set the configuration file for DRR.
   *
   * The file format is:
   * Line 1: <number_of_queues>
   * Line 2: <quantum_for_queue_0>
   * Line 3: <quantum_for_queue_1>
   * ...
   * Line N+1: <quantum_for_queue_N-1>
   *
   * \param filename The path to the configuration file.
   * \return True if configuration was successful, false otherwise.
   */
  bool SetConfigFile (std::string filename);

protected:
  virtual void DoDispose (void) override; // Mark as override
  // GetConfigFile is not strictly needed by the simulation but good practice if attribute is used
  // std::string GetConfigFile (void) const; // Already present in provided snippet

private:
  std::vector<uint32_t> m_deficits;    // Corresponds to DeficitCounter_i from the image
  std::vector<uint32_t> m_quantums;    // Corresponds to Quantum_i from the image
  uint32_t m_lastQueueServed;       // Stores the index of the last queue that was processed in the round-robin scan.
                                       // Used to determine where the next scan should begin.
  std::string m_configFile;            // Name of the configuration file
};
} // namespace ns3
#endif // DRR_H
