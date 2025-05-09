// cisco-parser.cc
#include "cisco-parser.h"
#include "ns3/log.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("CiscoParser");
NS_OBJECT_ENSURE_REGISTERED (CiscoParser);

TypeId
CiscoParser::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CiscoParser")
    .SetParent<Object> ()
    .SetGroupName ("Network")
    .AddConstructor<CiscoParser> ()
  ;
  return tid;
}

CiscoParser::CiscoParser () :
  m_qosEnabled (false),
  m_priorityQueueEnabled (false),
  m_dscpTrustEnabled (false),
  m_currentInterface (""),
  m_dscpMap ()
{
  NS_LOG_FUNCTION (this);
}

CiscoParser::~CiscoParser ()
{
  NS_LOG_FUNCTION (this);
}

void
CiscoParser::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Object::DoDispose ();
}

bool
CiscoParser::Parse (std::string filename, uint32_t &numQueues, std::vector<uint32_t> &priorities)
{
  NS_LOG_FUNCTION (this << filename);
  
  std::ifstream file (filename.c_str ());
  if (!file.is_open ())
    {
      NS_LOG_ERROR ("Failed to open file " << filename);
      return false;
    }
  
  std::string line;
  
  // Parse each line
  while (std::getline (file, line))
    {
      // Skip empty lines and comments
      if (line.empty () || line[0] == '#' || line[0] == '!')
        {
          continue;
        }
      
      // Remove leading and trailing whitespace
      line.erase (0, line.find_first_not_of (" \t"));
      line.erase (line.find_last_not_of (" \t") + 1);
      
      // Parse the line
      if (!ParseLine (line))
        {
          NS_LOG_ERROR ("Failed to parse line: " << line);
          file.close ();
          return false;
        }
    }
  
  file.close ();
  
  // Check if the configuration is valid
  if (!m_qosEnabled)
    {
      NS_LOG_ERROR ("QoS is not enabled");
      return false;
    }
  
  if (!m_priorityQueueEnabled)
    {
      NS_LOG_ERROR ("Priority queue is not enabled");
      return false;
    }
  
  if (!m_dscpTrustEnabled)
    {
      NS_LOG_ERROR ("DSCP trust is not enabled");
      return false;
    }
  
  if (m_dscpMap.empty ())
    {
      NS_LOG_ERROR ("No DSCP to priority mapping");
      return false;
    }
  
  // Determine the number of queues and their priorities
  numQueues = 0;
  priorities.clear ();
  
  // In Cisco 3750, there are 4 output queues (numbered 1-4)
  // The priority queue is queue 1, and the other queues are used for WRR
  numQueues = 4;
  
  // Initialize priorities (0 = highest, 3 = lowest)
  priorities.resize (numQueues, 3);
  
  // Set queue 1 (priority queue) to highest priority (0)
  priorities[0] = 0;
  
  // If there are dscp-priority mappings, use them
  if (!m_dscpPriorityMap.empty())
    {
      NS_LOG_INFO ("Using DSCP to priority mapping");
      
      // Use the DSCP to priority map to set queue priorities
      // Note: In Cisco, each DSCP value maps to a specific queue and priority
      for (uint32_t i = 1; i < numQueues; i++)
        {
          // Find the minimum priority level for this queue
          uint32_t minPriority = 3; // Default to lowest priority
          
          for (std::map<uint32_t, uint32_t>::const_iterator it = m_dscpPriorityMap.begin (); 
               it != m_dscpPriorityMap.end (); ++it)
            {
              uint32_t priority = it->second;
              
              // Map priority to queue (simplified mapping)
              uint32_t queue = (priority % (numQueues - 1)) + 1; // Ensure queue 0 is not used (reserved for priority queue)
              
              if (queue == i && priority < minPriority)
                {
                  minPriority = priority;
                }
            }
          
          // Set the priority for this queue
          priorities[i] = minPriority;
        }
    }
  else
    {
      NS_LOG_INFO ("Using DSCP to queue mapping");
      
      // Set other queues based on DSCP mapping
      // This is a simplified approach - in reality, Cisco uses more complex mappings
      for (std::map<uint32_t, uint32_t>::const_iterator it = m_dscpMap.begin (); 
           it != m_dscpMap.end (); ++it)
        {
          uint32_t dscp = it->first;
          uint32_t queue = it->second;
          
          if (queue > 0 && queue < numQueues)
            {
              // Set the priority based on DSCP value (higher DSCP = higher priority)
              priorities[queue] = std::min (priorities[queue], dscp % 3 + 1);
            }
        }
    }
  
  NS_LOG_INFO ("Parsed Cisco 3750 configuration: " << numQueues << " queues");
  for (uint32_t i = 0; i < numQueues; i++)
    {
      NS_LOG_INFO ("Queue " << i << " priority: " << priorities[i]);
    }
  
  return true;
}

bool
CiscoParser::ParseLine (std::string line)
{
  NS_LOG_FUNCTION (this << line);
  
  // Split the line into tokens
  std::vector<std::string> tokens = Split (line, ' ');
  
  if (tokens.empty ())
    {
      return true; // Skip empty lines
    }
  
  // Parse based on the first token
  if (tokens[0] == "interface")
    {
      return ParseInterfaceCommand (tokens);
    }
  else if (tokens[0] == "priority-queue")
    {
      return ParsePriorityQueueCommand (tokens);
    }
  else if (tokens[0] == "mls")
    {
      if (tokens.size () > 1 && tokens[1] == "qos")
        {
          if (tokens.size () > 2 && tokens[2] == "trust")
            {
              return ParseMlsQosTrustCommand (tokens);
            }
          else if (tokens.size () > 2 && tokens[2] == "map")
            {
              return ParseMlsQosMapCommand (tokens);
            }
          else
            {
              return ParseMlsQosCommand (tokens);
            }
        }
    }
  
  NS_LOG_WARN ("Unknown command: " << line);
  return true; // Ignore unknown commands
}

bool
CiscoParser::ParseInterfaceCommand (std::vector<std::string> tokens)
{
  NS_LOG_FUNCTION (this);
  
  if (tokens.size () < 2)
    {
      NS_LOG_ERROR ("Invalid interface command");
      return false;
    }
  
  m_currentInterface = tokens[1];
  NS_LOG_INFO ("Set current interface to " << m_currentInterface);
  
  return true;
}

bool
CiscoParser::ParsePriorityQueueCommand (std::vector<std::string> tokens)
{
  NS_LOG_FUNCTION (this);
  
  if (tokens.size () < 2)
    {
      NS_LOG_ERROR ("Invalid priority-queue command");
      return false;
    }
  
  if (tokens[1] == "out")
    {
      m_priorityQueueEnabled = true;
      NS_LOG_INFO ("Enabled priority queue on interface " << m_currentInterface);
    }
  else
    {
      NS_LOG_ERROR ("Unknown priority-queue command");
      return false;
    }
  
  return true;
}

bool
CiscoParser::ParseMlsQosCommand (std::vector<std::string> tokens)
{
  NS_LOG_FUNCTION (this);
  
  if (tokens.size () == 2 && tokens[1] == "qos")
    {
      m_qosEnabled = true;
      NS_LOG_INFO ("Enabled QoS");
    }
  else
    {
      NS_LOG_WARN ("Unknown mls qos command");
    }
  
  return true;
}

bool
CiscoParser::ParseMlsQosTrustCommand (std::vector<std::string> tokens)
{
  NS_LOG_FUNCTION (this);
  
  if (tokens.size () < 4)
    {
      NS_LOG_ERROR ("Invalid mls qos trust command");
      return false;
    }
  
  if (tokens[3] == "dscp")
    {
      m_dscpTrustEnabled = true;
      NS_LOG_INFO ("Enabled DSCP trust on interface " << m_currentInterface);
    }
  else
    {
      NS_LOG_WARN ("Unknown trust type: " << tokens[3]);
    }
  
  return true;
}

bool
CiscoParser::ParseMlsQosMapCommand (std::vector<std::string> tokens)
{
  NS_LOG_FUNCTION (this);
  
  if (tokens.size () < 6)
    {
      NS_LOG_ERROR ("Invalid mls qos map command");
      return false;
    }
  
  // Check which type of map command it is
  if (tokens[2] == "map" && tokens[3] == "dscp-queue")
    {
      // Handle dscp-queue mapping (original code)
      
      // Find the "to" token
      size_t toIndex = 0;
      for (size_t i = 4; i < tokens.size (); i++)
        {
          if (tokens[i] == "to")
            {
              toIndex = i;
              break;
            }
        }
      
      if (toIndex == 0 || toIndex == tokens.size () - 1)
        {
          NS_LOG_ERROR ("Invalid mls qos map command: missing 'to' keyword or queue value");
          return false;
        }
      
      // Parse DSCP values
      std::vector<uint32_t> dscpValues;
      for (size_t i = 4; i < toIndex; i++)
        {
          std::istringstream iss (tokens[i]);
          uint32_t dscp;
          
          if (!(iss >> dscp) || dscp > 63)
            {
              NS_LOG_ERROR ("Invalid DSCP value: " << tokens[i]);
              return false;
            }
          
          dscpValues.push_back (dscp);
        }
      
      // Parse queue value
      std::istringstream iss (tokens[toIndex + 1]);
      uint32_t queue;
      
      if (!(iss >> queue) || queue > 3)
        {
          NS_LOG_ERROR ("Invalid queue value: " << tokens[toIndex + 1]);
          return false;
        }
      
      // Map DSCP values to queue
      for (size_t i = 0; i < dscpValues.size (); i++)
        {
          m_dscpMap[dscpValues[i]] = queue;
          NS_LOG_INFO ("Mapped DSCP " << dscpValues[i] << " to queue " << queue);
        }
      
      return true;
    }
  else if (tokens[2] == "map" && tokens[3] == "dscp-priority")
    {
      // Handle dscp-priority mapping - delegating to the new method
      return ParseMlsQosDscpPriorityCommand(tokens);
    }
  else
    {
      NS_LOG_WARN ("Unknown mls qos map command: " << tokens[3]);
      return true; // Ignore unknown commands
    }
}

bool
CiscoParser::ParseMlsQosDscpPriorityCommand (std::vector<std::string> tokens)
{
  NS_LOG_FUNCTION (this);
  
  // Find the "to" token
  size_t toIndex = 0;
  for (size_t i = 4; i < tokens.size (); i++)
    {
      if (tokens[i] == "to")
        {
          toIndex = i;
          break;
        }
    }
  
  if (toIndex == 0 || toIndex == tokens.size () - 1)
    {
      NS_LOG_ERROR ("Invalid mls qos map dscp-priority command: missing 'to' keyword or priority value");
      return false;
    }
  
  // Parse DSCP values
  std::vector<uint32_t> dscpValues;
  for (size_t i = 4; i < toIndex; i++)
    {
      std::istringstream iss (tokens[i]);
      uint32_t dscp;
      
      if (!(iss >> dscp) || dscp > 63)
        {
          NS_LOG_ERROR ("Invalid DSCP value: " << tokens[i]);
          return false;
        }
      
      dscpValues.push_back (dscp);
    }
  
  // Parse priority value
  std::istringstream iss (tokens[toIndex + 1]);
  uint32_t priority;
  
  if (!(iss >> priority))
    {
      NS_LOG_ERROR ("Invalid priority value: " << tokens[toIndex + 1]);
      return false;
    }
  
  // Map DSCP values to priority level
  for (size_t i = 0; i < dscpValues.size (); i++)
    {
      m_dscpPriorityMap[dscpValues[i]] = priority;
      NS_LOG_INFO ("Mapped DSCP " << dscpValues[i] << " to priority level " << priority);
    }
  
  return true;
}

std::vector<std::string>
CiscoParser::Split (std::string str, char delimiter)
{
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream tokenStream (str);
  
  while (std::getline (tokenStream, token, delimiter))
    {
      if (!token.empty ())
        {
          tokens.push_back (token);
        }
    }
  
  return tokens;
}

} // namespace ns3