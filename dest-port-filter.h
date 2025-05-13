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
    c->Print(std::cout);
    Ipv4Header ip;
    if (!c->RemoveHeader(ip))
    {
      std::cout << "DestPortFilter::Match cant get header" << std::endl;
      return false;
    }
    // handle tcp
    if (ip.GetProtocol() == 6)
    {
      TcpHeader tcp;
      if (!c->PeekHeader(tcp))
        return false;
      return tcp.GetDestinationPort() == m_port;
    }
    if (ip.GetProtocol() == 17)
    {
      UdpHeader udp;
      if (!c->PeekHeader(udp))
        return false;
      return udp.GetDestinationPort() == m_port;
    }
    std::cout << "DestPortFilter::Match not matching" << std::endl;
    return false;
  }

private:
  uint16_t m_port;
};

} // namespace ns3
#endif
