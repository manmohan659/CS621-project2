#include "filter-element.h"
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("FilterElement");
NS_OBJECT_ENSURE_REGISTERED(FilterElement);

TypeId FilterElement::GetTypeId(void)
{
  static TypeId tid =
      TypeId("ns3::FilterElement").SetParent<Object>().SetGroupName("Network");
  return tid;
}

FilterElement::FilterElement()
{
  NS_LOG_FUNCTION(this);
}

FilterElement::~FilterElement()
{
  NS_LOG_FUNCTION(this);
}

void FilterElement::DoDispose(void)
{
  NS_LOG_FUNCTION(this);
  Object::DoDispose();
}

}
