// source-ip-address.cc
#include "source-ip-address.h"
#include "ns3/log.h"
#include "ns3/ipv4-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SourceIpAddress");
NS_OBJECT_ENSURE_REGISTERED (SourceIpAddress);

TypeId
SourceIpAddress::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SourceIpAddress")
    .SetParent<FilterElement> ()
    .SetGroupName ("Network")
    .AddConstructor<SourceIpAddress> ()
  ;
  return tid;
}

SourceIpAddress::SourceIpAddress () :
  m_address (Ipv4Address::GetAny ())
{
  NS_LOG_FUNCTION (this);
}

SourceIpAddress::SourceIpAddress (Ipv4Address addr) :
  m_address (addr)
{
  NS_LOG_FUNCTION (this << addr);
}

SourceIpAddress::~SourceIpAddress ()
{
  NS_LOG_FUNCTION (this);
}

bool
SourceIpAddress::Match (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);
  
  // Create a copy of the packet
  Ptr<Packet> copy = p->Copy ();
  
  // Try to remove the IPv4 header
  Ipv4Header ipHeader;
  if (copy->PeekHeader (ipHeader))
    {
      NS_LOG_LOGIC ("Found IPv4 header, source IP = " << ipHeader.GetSource ());
      return ipHeader.GetSource () == m_address;
    }
  
  NS_LOG_LOGIC ("No IPv4 header found");
  return false;
}

void
SourceIpAddress::SetAddress (Ipv4Address addr)
{
  NS_LOG_FUNCTION (this << addr);
  m_address = addr;
}

Ipv4Address
SourceIpAddress::GetAddress (void) const
{
  NS_LOG_FUNCTION (this);
  return m_address;
}

// dest-ip-address.cc
#include "dest-ip-address.h"
#include "ns3/log.h"
#include "ns3/ipv4-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DestIpAddress");
NS_OBJECT_ENSURE_REGISTERED (DestIpAddress);

TypeId
DestIpAddress::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DestIpAddress")
    .SetParent<FilterElement> ()
    .SetGroupName ("Network")
    .AddConstructor<DestIpAddress> ()
  ;
  return tid;
}

DestIpAddress::DestIpAddress () :
  m_address (Ipv4Address::GetAny ())
{
  NS_LOG_FUNCTION (this);
}

DestIpAddress::DestIpAddress (Ipv4Address addr) :
  m_address (addr)
{
  NS_LOG_FUNCTION (this << addr);
}

DestIpAddress::~DestIpAddress ()
{
  NS_LOG_FUNCTION (this);
}

bool
DestIpAddress::Match (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);
  
  // Create a copy of the packet
  Ptr<Packet> copy = p->Copy ();
  
  // Try to remove the IPv4 header
  Ipv4Header ipHeader;
  if (copy->PeekHeader (ipHeader))
    {
      NS_LOG_LOGIC ("Found IPv4 header, destination IP = " << ipHeader.GetDestination ());
      return ipHeader.GetDestination () == m_address;
    }
  
  NS_LOG_LOGIC ("No IPv4 header found");
  return false;
}

void
DestIpAddress::SetAddress (Ipv4Address addr)
{
  NS_LOG_FUNCTION (this << addr);
  m_address = addr;
}

Ipv4Address
DestIpAddress::GetAddress (void) const
{
  NS_LOG_FUNCTION (this);
  return m_address;
}

// source-port.cc
#include "source-port.h"
#include "ns3/log.h"
#include "ns3/ipv4-header.h"
#include "ns3/tcp-header.h"
#include "ns3/udp-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SourcePort");
NS_OBJECT_ENSURE_REGISTERED (SourcePort);

TypeId
SourcePort::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SourcePort")
    .SetParent<FilterElement> ()
    .SetGroupName ("Network")
    .AddConstructor<SourcePort> ()
  ;
  return tid;
}

SourcePort::SourcePort () :
  m_port (0)
{
  NS_LOG_FUNCTION (this);
}

SourcePort::SourcePort (uint16_t port) :
  m_port (port)
{
  NS_LOG_FUNCTION (this << port);
}

SourcePort::~SourcePort ()
{
  NS_LOG_FUNCTION (this);
}

bool
SourcePort::Match (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);
  
  // Create a copy of the packet
  Ptr<Packet> copy = p->Copy ();
  
  // Try to remove the IPv4 header
  Ipv4Header ipHeader;
  if (copy->RemoveHeader (ipHeader))
    {
      // Check the protocol
      uint8_t protocol = ipHeader.GetProtocol ();
      
      if (protocol == 6) // TCP
        {
          TcpHeader tcpHeader;
          if (copy->PeekHeader (tcpHeader))
            {
              NS_LOG_LOGIC ("Found TCP header, source port = " << tcpHeader.GetSourcePort ());
              return tcpHeader.GetSourcePort () == m_port;
            }
        }
      else if (protocol == 17) // UDP
        {
          UdpHeader udpHeader;
          if (copy->PeekHeader (udpHeader))
            {
              NS_LOG_LOGIC ("Found UDP header, source port = " << udpHeader.GetSourcePort ());
              return udpHeader.GetSourcePort () == m_port;
            }
        }
    }
  
  NS_LOG_LOGIC ("No matching transport header found");
  return false;
}

void
SourcePort::SetPort (uint16_t port)
{
  NS_LOG_FUNCTION (this << port);
  m_port = port;
}

uint16_t
SourcePort::GetPort (void) const
{
  NS_LOG_FUNCTION (this);
  return m_port;
}

// dest-port.cc
#include "dest-port.h"
#include "ns3/log.h"
#include "ns3/ipv4-header.h"
#include "ns3/tcp-header.h"
#include "ns3/udp-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DestPort");
NS_OBJECT_ENSURE_REGISTERED (DestPort);

TypeId
DestPort::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DestPort")
    .SetParent<FilterElement> ()
    .SetGroupName ("Network")
    .AddConstructor<DestPort> ()
  ;
  return tid;
}

DestPort::DestPort () :
  m_port (0)
{
  NS_LOG_FUNCTION (this);
}

DestPort::DestPort (uint16_t port) :
  m_port (port)
{
  NS_LOG_FUNCTION (this << port);
}

DestPort::~DestPort ()
{
  NS_LOG_FUNCTION (this);
}

bool
DestPort::Match (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);
  
  // Create a copy of the packet
  Ptr<Packet> copy = p->Copy ();
  
  // Try to remove the IPv4 header
  Ipv4Header ipHeader;
  if (copy->RemoveHeader (ipHeader))
    {
      // Check the protocol
      uint8_t protocol = ipHeader.GetProtocol ();
      
      if (protocol == 6) // TCP
        {
          TcpHeader tcpHeader;
          if (copy->PeekHeader (tcpHeader))
            {
              NS_LOG_LOGIC ("Found TCP header, destination port = " << tcpHeader.GetDestinationPort ());
              return tcpHeader.GetDestinationPort () == m_port;
            }
        }
      else if (protocol == 17) // UDP
        {
          UdpHeader udpHeader;
          if (copy->PeekHeader (udpHeader))
            {
              NS_LOG_LOGIC ("Found UDP header, destination port = " << udpHeader.GetDestinationPort ());
              return udpHeader.GetDestinationPort () == m_port;
            }
        }
    }
  
  NS_LOG_LOGIC ("No matching transport header found");
  return false;
}

void
DestPort::SetPort (uint16_t port)
{
  NS_LOG_FUNCTION (this << port);
  m_port = port;
}

uint16_t
DestPort::GetPort (void) const
{
  NS_LOG_FUNCTION (this);
  return m_port;
}

// protocol-number.cc
#include "protocol-number.h"
#include "ns3/log.h"
#include "ns3/ipv4-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ProtocolNumber");
NS_OBJECT_ENSURE_REGISTERED (ProtocolNumber);

TypeId
ProtocolNumber::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ProtocolNumber")
    .SetParent<FilterElement> ()
    .SetGroupName ("Network")
    .AddConstructor<ProtocolNumber> ()
  ;
  return tid;
}

ProtocolNumber::ProtocolNumber () :
  m_protocol (0)
{
  NS_LOG_FUNCTION (this);
}

ProtocolNumber::ProtocolNumber (uint8_t protocol) :
  m_protocol (protocol)
{
  NS_LOG_FUNCTION (this << (uint32_t) protocol);
}

ProtocolNumber::~ProtocolNumber ()
{
  NS_LOG_FUNCTION (this);
}

bool
ProtocolNumber::Match (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);
  
  // Create a copy of the packet
  Ptr<Packet> copy = p->Copy ();
  
  // Try to remove the IPv4 header
  Ipv4Header ipHeader;
  if (copy->PeekHeader (ipHeader))
    {
      NS_LOG_LOGIC ("Found IPv4 header, protocol = " << (uint32_t) ipHeader.GetProtocol ());
      return ipHeader.GetProtocol () == m_protocol;
    }
  
  NS_LOG_LOGIC ("No IPv4 header found");
  return false;
}

void
ProtocolNumber::SetProtocol (uint8_t protocol)
{
  NS_LOG_FUNCTION (this << (uint32_t) protocol);
  m_protocol = protocol;
}

uint8_t
ProtocolNumber::GetProtocol (void) const
{
  NS_LOG_FUNCTION (this);
  return m_protocol;
}

// tos-field.cc
#include "tos-field.h"
#include "ns3/log.h"
#include "ns3/ipv4-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TosField");
NS_OBJECT_ENSURE_REGISTERED (TosField);

TypeId
TosField::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TosField")
    .SetParent<FilterElement> ()
    .SetGroupName ("Network")
    .AddConstructor<TosField> ()
  ;
  return tid;
}

TosField::TosField () :
  m_tos (0)
{
  NS_LOG_FUNCTION (this);
}

TosField::TosField (uint8_t tos) :
  m_tos (tos)
{
  NS_LOG_FUNCTION (this << (uint32_t) tos);
}

TosField::~TosField ()
{
  NS_LOG_FUNCTION (this);
}

bool
TosField::Match (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);
  
  // Create a copy of the packet
  Ptr<Packet> copy = p->Copy ();
  
  // Try to remove the IPv4 header
  Ipv4Header ipHeader;
  if (copy->PeekHeader (ipHeader))
    {
      NS_LOG_LOGIC ("Found IPv4 header, ToS = " << (uint32_t) ipHeader.GetTos ());
      return ipHeader.GetTos () == m_tos;
    }
  
  NS_LOG_LOGIC ("No IPv4 header found");
  return false;
}

void
TosField::SetTos (uint8_t tos)
{
  NS_LOG_FUNCTION (this << (uint32_t) tos);
  m_tos = tos;
}

uint8_t
TosField::GetTos (void) const
{
  NS_LOG_FUNCTION (this);
  return m_tos;
}

// packet-number.cc
#include "packet-number.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PacketNumber");
NS_OBJECT_ENSURE_REGISTERED (PacketNumber);

// Initialize static packet counter
uint32_t PacketNumber::s_packetCount = 0;

TypeId
PacketNumber::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PacketNumber")
    .SetParent<FilterElement> ()
    .SetGroupName ("Network")
    .AddConstructor<PacketNumber> ()
  ;
  return tid;
}

PacketNumber::PacketNumber () :
  m_number (0)
{
  NS_LOG_FUNCTION (this);
}

PacketNumber::PacketNumber (uint32_t number) :
  m_number (number)
{
  NS_LOG_FUNCTION (this << number);
}

PacketNumber::~PacketNumber ()
{
  NS_LOG_FUNCTION (this);
}

bool
PacketNumber::Match (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);
  
  // Increment the static packet counter for each packet checked
  s_packetCount++;
  
  NS_LOG_LOGIC ("Packet count = " << s_packetCount << ", matching against " << m_number);
  
  // Check if the current packet count matches the specified number
  return (s_packetCount % m_number == 0);
}

void
PacketNumber::SetNumber (uint32_t number)
{
  NS_LOG_FUNCTION (this << number);
  m_number = number;
}

uint32_t
PacketNumber::GetNumber (void) const
{
  NS_LOG_FUNCTION (this);
  return m_number;
}

} // namespace ns3