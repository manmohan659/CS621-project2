#include "dest-ip-address.h"
#include "ns3/ipv4-header.h"
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("DestIpAddress");
NS_OBJECT_ENSURE_REGISTERED(DestIpAddress);

TypeId DestIpAddress::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::DestIpAddress")
                          .SetParent<FilterElement>()
                          .SetGroupName("Network")
                          .AddConstructor<DestIpAddress>();
  return tid;
}

DestIpAddress::DestIpAddress() : m_address(Ipv4Address::GetAny())
{
  NS_LOG_FUNCTION(this);
}

DestIpAddress::DestIpAddress(Ipv4Address addr) : m_address(addr)
{
  NS_LOG_FUNCTION(this << addr);
}

DestIpAddress::~DestIpAddress()
{
  NS_LOG_FUNCTION(this);
}

void DestIpAddress::DoDispose(void)
{
  NS_LOG_FUNCTION(this);
  FilterElement::DoDispose();
}

bool DestIpAddress::Match(Ptr<Packet> p)
{
  NS_LOG_FUNCTION(this << p);

  Ptr<Packet> copy = p->Copy();

  Ipv4Header ipv4Header;
  if (copy->PeekHeader(ipv4Header))
  {
    bool match = (ipv4Header.GetDestination() == m_address);
    NS_LOG_LOGIC("Destination IP address "
                 << ipv4Header.GetDestination() << " "
                 << (match ? "matches" : "doesn't match") << " filter "
                 << m_address);
    return match;
  }

  NS_LOG_LOGIC("Packet doesn't have an IPv4 header");
  return false;
}

void DestIpAddress::SetAddress(Ipv4Address addr)
{
  NS_LOG_FUNCTION(this << addr);
  m_address = addr;
}

Ipv4Address DestIpAddress::GetAddress(void) const
{
  NS_LOG_FUNCTION(this);
  return m_address;
}

}
