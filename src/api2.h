#ifndef QMLOG_DISPATHCER_H
#define QMLOG_DISPATHCER_H

#include "qm..."

#ifndef QMLOG_DEFAULT_LOGGING_DISPATCHER
#define QMLOG_DEFAULT_LOGGING_DISPATCHER (qmlog::object.get_current_dispatcher())
#endif

namespace qmlog
{
  class object_t ;
  class abstract_log_t ;
  class dispatcher_t ;
  class slave_dispatcher_t ;

  class object_t
  {
    static object_t object ;
    static dispatcher_t *current_dispatcher ;
    static dispatcher_t *master ;
    static set<slave_dispatcher_t *> slaves ;
  public:
    object_t() ;
    void register_slave(slave_dispatcher_t *) ;
    static inline dispatcher_t *get_current_dispatcher() { return current_dispatcher ; }
  } ;

  class abstract_log_t
  {
  } ;

  class dispatcher_t
  {

    string name ;
    set<abstract_log_t *> logs ;
    pid_t last_pid ;
    std::string pid_s ;
  public:
    void attach(abstract_log_t) ;

  } ;

  class slave_dispatcher_t : public dispatcher_t
  {
  } ;

  inline void init() { object.init() ; }
}



#endif // QMLOG_DISPATHCER_H
