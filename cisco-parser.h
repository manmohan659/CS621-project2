// cisco-parser.h
#ifndef CISCO_PARSER_H
#define CISCO_PARSER_H

#include "ns3/object.h"
#include <map>
#include <string>
#include <vector>

namespace ns3
{

/**
 * \brief Parser for Cisco 3750 CLI commands for SPQ configuration
 *
 * This class parses Cisco 3750 CLI commands to configure SPQ.
 * It supports the following commands:
 * - mls qos
 * - interface <interface_name>
 * - priority-queue out
 * - mls qos trust dscp
 * - mls qos map dscp-priority <dscp_values> to <priority_level>
 */
class CiscoParser : public Object
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
  CiscoParser();

  /**
   * \brief Destructor
   */
  virtual ~CiscoParser();

  /**
   * \brief Parse a configuration file
   * \param filename The configuration file
   * \param numQueues Output parameter for number of queues
   * \param priorities Output parameter for priority levels
   * \return true if successful, false otherwise
   */
  bool Parse(std::string filename, uint32_t& numQueues,
             std::vector<uint32_t>& priorities);

protected:
  /**
   * \brief Dispose of the object
   */
  virtual void DoDispose(void);

private:
  /**
   * \brief Parse a line of configuration
   * \param line The line to parse
   * \return true if successful, false otherwise
   */
  bool ParseLine(std::string line);

  /**
   * \brief Parse an interface command
   * \param tokens The command tokens
   * \return true if successful, false otherwise
   */
  bool ParseInterfaceCommand(std::vector<std::string> tokens);

  /**
   * \brief Parse a priority-queue command
   * \param tokens The command tokens
   * \return true if successful, false otherwise
   */
  bool ParsePriorityQueueCommand(std::vector<std::string> tokens);

  /**
   * \brief Parse an mls qos command
   * \param tokens The command tokens
   * \return true if successful, false otherwise
   */
  bool ParseMlsQosCommand(std::vector<std::string> tokens);

  /**
   * \brief Parse an mls qos trust command
   * \param tokens The command tokens
   * \return true if successful, false otherwise
   */
  bool ParseMlsQosTrustCommand(std::vector<std::string> tokens);

  /**
   * \brief Parse an mls qos map command
   * \param tokens The command tokens
   * \return true if successful, false otherwise
   */
  bool ParseMlsQosMapCommand(std::vector<std::string> tokens);

  /**
   * \brief Parse an mls qos map dscp-priority command
   * \param tokens The command tokens
   * \return true if successful, false otherwise
   */
  bool ParseMlsQosDscpPriorityCommand(std::vector<std::string> tokens);

  /**
   * \brief Split a string into tokens
   * \param str The string to split
   * \param delimiter The delimiter
   * \return The tokens
   */
  std::vector<std::string> Split(std::string str, char delimiter);

  bool m_qosEnabled;                      //!< Whether QoS is enabled
  bool m_priorityQueueEnabled;            //!< Whether priority queue is enabled
  bool m_dscpTrustEnabled;                //!< Whether DSCP trust is enabled
  std::string m_currentInterface;         //!< Current interface
  std::map<uint32_t, uint32_t> m_dscpMap; //!< DSCP to queue mapping
  std::map<uint32_t, uint32_t>
      m_dscpPriorityMap; //!< DSCP to priority level mapping
};

} // namespace ns3

#endif /* CISCO_PARSER_H */