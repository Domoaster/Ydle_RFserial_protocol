
#include "Kernel.h"
#include "Float.h"

namespace domoaster {

class node_ydle_serial : public INodeType 
{
  public:
    std::string Name () { return "Node Ydle over serial" ; }
    std::string Class () { return "node_ydle_serial" ; }
    std::string Protocol () { return "ydle_serial" ; }
    
    void Init () {} ;
    void Start () {} ;
} ;

} ; // namespace domoaster

extern "C" int LoadPlugins (domoaster::Kernel &) ;