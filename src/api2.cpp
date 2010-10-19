#include "api2.h"

namespace qmlog
{
  dispatcher_t *object_t::current_dispatcher = NULL ;
  dispatcher_t *object_t::master = NULL ;
  set<slave_dispatcher_t *> object_t::slaves ;

  object_t::object_t()
  {
    static bool first = true ;
    if(! first)
    {
      // TODO: Log this as an error somehow....
      return ; // ... and do nothing else
    }

    first = false ;

    slave_dispatcher_t *d = new slave_dispatcher_t ;
    // TODO: set default options and loggers...
    register_slave(d) ;
    object_t::current_dispatcher = d ;
  }

  void object::register_slave(slave_dispatcher_t *d)
  {
    slaves.insert(d) ;
  }

}

