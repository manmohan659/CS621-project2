#ifndef DRR_H
#define DRR_H

#include "diffserv.h"
#include <string>
#include <vector>

namespace ns3
{

class DRR : public DiffServ
{
public:
  static TypeId GetTypeId(void);
  DRR();
  virtual ~DRR();
  virtual Ptr<Packet> Schedule(void) override;

  /**
   * \brief Set the configuration file for DRR.
   * \param filename The path to the configuration file.
   * \return True if configuration was successful, false otherwise.
   */
  bool SetConfigFile(std::string filename);

protected:
  virtual void DoDispose(void) override;

private:
  std::vector<uint32_t>
      m_deficits;
  std::vector<uint32_t> m_quantums;
  uint32_t m_lastQueueServed;
  std::string m_configFile;
};
}
#endif
