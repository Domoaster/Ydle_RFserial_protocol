
#include "ydle_serial.h"


using namespace domoaster ;

int LoadPlugins (Kernel & k)
{
	k.RegisterProtocol (new ydle_serial) ;
	k.RegisterNode (new node_ydle_serial) ;
	return 1 ;
}