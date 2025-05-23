#ifndef SPQ_H
#define SPQ_H

#include "diffserv.h"
#include <string>

namespace ns3
{

/**
 * \ingroup queue
 * \brief A Strict Priority Queueing (SPQ) implementation
 */
class SPQ : public DiffServ
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
  SPQ();

  /**
   * \brief Destructor
   */
  virtual ~SPQ();

  /**
   * \brief Schedule which queue to serve next
   * \return The selected packet
   */
  virtual Ptr<Packet> Schedule(void) override;

  /**
   * \brief Get the configuration file
   * \return The configuration file
   */
  std::string GetConfigFile(void) const;

  /**
   * \brief Set configuration from file
   * \param filename The configuration file
   * \return true if successful, false otherwise
   */
  bool SetConfigFile(std::string filename);

  /**
   * \brief Get the Cisco configuration file
   * \return The Cisco configuration file
   */
  std::string GetCiscoConfigFile(void) const;

  /**
   * \brief Set configuration from Cisco CLI configuration file
   * \param filename The Cisco CLI configuration file
   * \return true if successful, false otherwise
   */
  bool SetCiscoConfigFile(std::string filename);

protected:
  /**
   * \brief Dispose of the object
   */
  virtual void DoDispose(void) override;

private:
  std::string m_configFile;
  std::string m_ciscoConfigFile;
};

}

#endif