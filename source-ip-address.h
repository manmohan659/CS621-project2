// source-ip-address.h
#ifndef SOURCE_IP_ADDRESS_H
#define SOURCE_IP_ADDRESS_H

#include "filter-element.h"
#include "ns3/ipv4-address.h"

namespace ns3 {

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
  static TypeId GetTypeId (void);
  
  /**
   * \brief Constructor
   */
  SourceIpAddress ();
  
  /**
   * \brief Constructor with IP address
   * \param addr The source IP address to match
   */
  SourceIpAddress (Ipv4Address addr);
  
  /**
   * \brief Destructor
   */
  virtual ~SourceIpAddress ();
  
  /**
   * \brief Check if a packet matches this filter element
   * \param p The packet to check
   * \return True if the packet matches this filter element
   */
  virtual bool Match (Ptr<Packet> p);
  
  /**
   * \brief Set the source IP address to match
   * \param addr The source IP address
   */
  void SetAddress (Ipv4Address addr);
  
  /**
   * \brief Get the source IP address to match
   * \return The source IP address
   */
  Ipv4Address GetAddress (void) const;

private:
  Ipv4Address m_address; //!< The source IP address to match
};

// dest-ip-address.h
/**
 * \brief Filter element for destination IP address
 */
class DestIpAddress : public FilterElement
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
  DestIpAddress ();
  
  /**
   * \brief Constructor with IP address
   * \param addr The destination IP address to match
   */
  DestIpAddress (Ipv4Address addr);
  
  /**
   * \brief Destructor
   */
  virtual ~DestIpAddress ();
  
  /**
   * \brief Check if a packet matches this filter element
   * \param p The packet to check
   * \return True if the packet matches this filter element
   */
  virtual bool Match (Ptr<Packet> p);
  
  /**
   * \brief Set the destination IP address to match
   * \param addr The destination IP address
   */
  void SetAddress (Ipv4Address addr);
  
  /**
   * \brief Get the destination IP address to match
   * \return The destination IP address
   */
  Ipv4Address GetAddress (void) const;

private:
  Ipv4Address m_address; //!< The destination IP address to match
};

// source-port.h
/**
 * \brief Filter element for source port
 */
class SourcePort : public FilterElement
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
  SourcePort ();
  
  /**
   * \brief Constructor with port number
   * \param port The source port to match
   */
  SourcePort (uint16_t port);
  
  /**
   * \brief Destructor
   */
  virtual ~SourcePort ();
  
  /**
   * \brief Check if a packet matches this filter element
   * \param p The packet to check
   * \return True if the packet matches this filter element
   */
  virtual bool Match (Ptr<Packet> p);
  
  /**
   * \brief Set the source port to match
   * \param port The source port
   */
  void SetPort (uint16_t port);
  
  /**
   * \brief Get the source port to match
   * \return The source port
   */
  uint16_t GetPort (void) const;

private:
  uint16_t m_port; //!< The source port to match
};

// dest-port.h
/**
 * \brief Filter element for destination port
 */
class DestPort : public FilterElement
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
  DestPort ();
  
  /**
   * \brief Constructor with port number
   * \param port The destination port to match
   */
  DestPort (uint16_t port);
  
  /**
   * \brief Destructor
   */
  virtual ~DestPort ();
  
  /**
   * \brief Check if a packet matches this filter element
   * \param p The packet to check
   * \return True if the packet matches this filter element
   */
  virtual bool Match (Ptr<Packet> p);
  
  /**
   * \brief Set the destination port to match
   * \param port The destination port
   */
  void SetPort (uint16_t port);
  
  /**
   * \brief Get the destination port to match
   * \return The destination port
   */
  uint16_t GetPort (void) const;

private:
  uint16_t m_port; //!< The destination port to match
};

// protocol-number.h
/**
 * \brief Filter element for protocol number
 */
class ProtocolNumber : public FilterElement
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
  ProtocolNumber ();
  
  /**
   * \brief Constructor with protocol number
   * \param protocol The protocol number to match
   */
  ProtocolNumber (uint8_t protocol);
  
  /**
   * \brief Destructor
   */
  virtual ~ProtocolNumber ();
  
  /**
   * \brief Check if a packet matches this filter element
   * \param p The packet to check
   * \return True if the packet matches this filter element
   */
  virtual bool Match (Ptr<Packet> p);
  
  /**
   * \brief Set the protocol number to match
   * \param protocol The protocol number
   */
  void SetProtocol (uint8_t protocol);
  
  /**
   * \brief Get the protocol number to match
   * \return The protocol number
   */
  uint8_t GetProtocol (void) const;

private:
  uint8_t m_protocol; //!< The protocol number to match
};

// tos-field.h
/**
 * \brief Filter element for ToS (Type of Service) field
 */
class TosField : public FilterElement
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
  TosField ();
  
  /**
   * \brief Constructor with ToS value
   * \param tos The ToS value to match
   */
  TosField (uint8_t tos);
  
  /**
   * \brief Destructor
   */
  virtual ~TosField ();
  
  /**
   * \brief Check if a packet matches this filter element
   * \param p The packet to check
   * \return True if the packet matches this filter element
   */
  virtual bool Match (Ptr<Packet> p);
  
  /**
   * \brief Set the ToS value to match
   * \param tos The ToS value
   */
  void SetTos (uint8_t tos);
  
  /**
   * \brief Get the ToS value to match
   * \return The ToS value
   */
  uint8_t GetTos (void) const;

private:
  uint8_t m_tos; //!< The ToS value to match
};

// packet-number.h
/**
 * \brief Filter element for packet number
 */
class PacketNumber : public FilterElement
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
  PacketNumber ();
  
  /**
   * \brief Constructor with packet number
   * \param number The packet number to match
   */
  PacketNumber (uint32_t number);
  
  /**
   * \brief Destructor
   */
  virtual ~PacketNumber ();
  
  /**
   * \brief Check if a packet matches this filter element
   * \param p The packet to check
   * \return True if the packet matches this filter element
   */
  virtual bool Match (Ptr<Packet> p);
  
  /**
   * \brief Set the packet number to match
   * \param number The packet number
   */
  void SetNumber (uint32_t number);
  
  /**
   * \brief Get the packet number to match
   * \return The packet number
   */
  uint32_t GetNumber (void) const;

private:
  uint32_t m_number; //!< The packet number to match
  static uint32_t s_packetCount; //!< Static packet counter
};