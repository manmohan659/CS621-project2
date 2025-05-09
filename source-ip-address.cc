// source-ip-address.cc
#include "source-ip-address.h"
#include "ns3/ipv4-header.h"
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SourceIpAddress");
NS_OBJECT_ENSURE_REGISTERED(SourceIpAddress);

TypeId SourceIpAddress::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::SourceIpAddress")
                          .SetParent<FilterElement>()
                          .SetGroupName("Network")
                          .AddConstructor<SourceIpAddress>();
  return tid;
}

SourceIpAddress::SourceIpAddress() : m_address(Ipv4Address::GetAny())
{
  NS_LOG_FUNCTION(this);
}

SourceIpAddress::SourceIpAddress(Ipv4Address addr) : m_address(addr)
{
  NS_LOG_FUNCTION(this << addr);
}

SourceIpAddress::~SourceIpAddress()
{
  NS_LOG_FUNCTION(this);
}

void SourceIpAddress::DoDispose(void)
{
  NS_LOG_FUNCTION(this);
  FilterElement::DoDispose();
}

bool SourceIpAddress::Match(Ptr<Packet> p)
{
  NS_LOG_FUNCTION(this << p);

  // Create a copy of the packet
  Ptr<Packet> copy = p->Copy();

  // Try to remove the IPv4 header
  Ipv4Header ipHeader;
  if (copy->PeekHeader(ipHeader))
  {
    NS_LOG_LOGIC("Found IPv4 header, source IP = " << ipHeader.GetSource());
    return ipHeader.GetSource() == m_address;
  }

  NS_LOG_LOGIC("No IPv4 header found");
  return false;
}

void SourceIpAddress::SetAddress(Ipv4Address addr)
{
  NS_LOG_FUNCTION(this << addr);
  m_address = addr;
}

Ipv4Address SourceIpAddress::GetAddress(void) const
{
  NS_LOG_FUNCTION(this);
  return m_address;
}

} // namespace ns3