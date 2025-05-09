#ifndef DEST_PORT_FILTER_H
#define DEST_PORT_FILTER_H

#include "filter-element.h"
#include "ns3/ipv4-header.h"
#include "ns3/tcp-header.h"

namespace ns3
{

class DestPortFilter : public FilterElement
{
public:
  static TypeId GetTypeId()
  {
    static TypeId tid = TypeId("ns3::DestPortFilter")
                            .SetParent<FilterElement>()
                            .AddConstructor<DestPortFilter>();
    return tid;
  }
  DestPortFilter() : m_port(0)
  {
  }
  explicit DestPortFilter(uint16_t port) : m_port(port)
  {
  }

  bool Match(Ptr<Packet> p) override
  {
    Ptr<Packet> c = p->Copy();
    Ipv4Header ip;
    if (!c->RemoveHeader(ip) || ip.GetProtocol() != 6)
      return false;
    TcpHeader tcp;
    if (!c->PeekHeader(tcp))
      return false;
    return tcp.GetDestinationPort() == m_port;
  }

private:
  uint16_t m_port;
};

}
#endif
