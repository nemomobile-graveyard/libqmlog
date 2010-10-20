#include <cstdio>
#include <cstring>

#include <string>
#include <set>
using namespace std ;

#include "api2.h"

namespace qmlog
{
  object_t object ;

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
    current_dispatcher = d ;
    master = NULL ;
    name = get_process_name() ;
  }

  string object_t::get_process_name()
  {
    static const char *anonymous = "<unknown>" ;
    const unsigned int len = 256 ;
    char buf[len+1] ;

    snprintf(buf, len, "/proc/%d/cmdline", getpid()) ;

    // just being a bit paranoid (a decimal number will be never so long):
    buf[len] = '\0' ;

    FILE *fp = fopen(buf, "r") ;

    if(fp==NULL)
      return anonymous ;

    string name ;

    for(bool word_done=false; not word_done; )
    {
      int x = fread(buf, 1, len, fp) ;
      buf[x] = '\0' ;
      word_done = strlen(buf) < len ;
      name += buf ;
    }

    const char *p = name.c_str(), *q = strrchr(p, '/') ;

    if(q==NULL) // no slash at all
      return name ;

    if(q[1]=='\0') // trailing slash, again paranoid
      return anonymous ;

    return q+1 ;
  }

  void object_t::init()
  {
    static bool first = true ;
    if (not first)
    {
      return ; // XXX: log it
    }
    first = false ;

    current_dispatcher = master = new dispatcher_t ;
  }

  void object_t::register_slave(slave_dispatcher_t *d)
  {
    slaves.insert(d) ;
  }

  dispatcher_t::dispatcher_t(const char *name)
  {
    this->name = (name==NULL) ? qmlog::object.get_name() : string(name) ;
    last_pid = (pid_t) 0 ;
    current_level = qmlog::Full ;
  }

  int dispatcher_t::log_level(int new_level)
  {
    return current_level = new_level ;
  }

  int dispatcher_t::log_level()
  {
    return current_level ;
  }

  void dispatcher_t::attach(abstract_log_t *l)
  {
    logs.insert(l) ;
  }

  void dispatcher_t::detach(abstract_log_t *l)
  {
    logs.erase(l) ;
  }

  void dispatcher_t::message(int level)
  {
    if (level<=current_level)
      message(level, -1, NULL, NULL, "") ;
  }

  void dispatcher_t::message(int level, const char *fmt, ...)
  {
    if (level<=current_level)
    {
      va_list arg ;
      va_start(arg, fmt) ;
      generic(level, -1, NULL, NULL, fmt, arg) ;
      va_end(arg) ;
    }
  }

  void dispatcher_t::message(int level, int line, const char *file, const char *func)
  {
    if (level<=current_level)
      message(level, line, file, func, "") ;
  }

  void dispatcher_t::message(int level, int line, const char *file, const char *func, const char *fmt, ...)
  {
    if (level<=current_level)
    {
      va_list arg ;
      va_start(arg, fmt) ;
      generic(level, line, file, func, fmt, arg) ;
      va_end(arg) ;
    }
  }

  void dispatcher_t::generic(int level, int line, const char *file, const char *func, const char *fmt, va_list arg)
  {
    for(set<abstract_log_t*>::iterator it=logs.begin(); it!=logs.end(); ++it)
      if (level<=(*it)->log_level())
        (*it)->compose_message(level, line, file, func, fmt, arg) ;
  }

  abstract_log_t::abstract_log_t(int maximal_log_level, dispatcher_t *d)
  {
    level = max_level = maximal_log_level ;
    dispatcher = d ;
    dispatcher->attach(this) ;
  }

  int abstract_log_t::log_level(int new_level)
  {
    if(new_level<=max_level)
      level = new_level ;
    return level ;
  }

  int abstract_log_t::log_level()
  {
    return level ;
  }

  int abstract_log_t::set_fields(int mask)
  {
    return fields = mask ;
  }

  int abstract_log_t::get_fields()
  {
    return fields ;
  }

  int abstract_log_t::enable_fields(int mask)
  {
    return fields |= mask ;
  }

  int abstract_log_t::disable_fields(int mask)
  {
    return fields &= ~mask ;
  }

  abstract_log_t::~abstract_log_t()
  {
    dispatcher->detach(this) ;
  }

  void abstract_log_t::compose_message(int level, int line, const char *file, const char *func, const char *fmt, va_list args)
  {
    smart_buffer<1024> buf ;
    const char *separator = "" ;
    if (fields & Time_Info_Block)
    {
      const char *ti_separator = "" ;
      buf.printf("[") ;
      if (int mono = fields & qmlog::Monotonic_Mask)
      {
        dispatcher->get_timestamp() ;
        buf.printf("%s", dispatcher->str_monotonic()) ;
        if (mono & qmlog::Monotonic_Milli)
        {
          if (mono & qmlog::Monotonic_Nano)
            buf.printf(".%s", dispatcher->str_monotonic_nano()) ;
          else if (mono & qmlog::Monotonic_Micro)
            buf.printf(".%s", dispatcher->str_monotonic_micro()) ;
          else
            buf.printf(".%s", dispatcher->str_monotonic_milli()) ;
        }
        ti_separator = " " ;
      }
      if (int tz = fields & qmlog::Timezone_Tm_Block)
      {
        buf.printf("%s(", ti_separator) ;
        dispatcher->get_struct_tm() ;
        const char *tz_separator = "" ;
        if (tz & qmlog::Timezone_Offset)
        {
          buf.printf("%s", dispatcher->str_gmt_offset()) ;
          tz_separator = "," ;
        }
        if (tz & qmlog::Timezone_Abbreviation)
          buf.printf("%s%s", tz_separator, dispatcher->str_tz_abbreviation()) ;
        buf.printf(")") ;
        ti_separator = " " ;
      }
      if (fields & qmlog::Date)
      {
        buf.printf("%s%s", ti_separator, dispatcher->str_date()) ;
        ti_separator = " " ;
      }
      if (int time = fields & qmlog::Time_Mask)
      {
        buf.printf("%s%s", ti_separator, dispatcher->str_time()) ;
        if (time & qmlog::Time_Milli)
        {
          if (time & qmlog::Time_Micro)
            buf.printf(".%s", dispatcher->str_time_micro()) ;
          else
            buf.printf(".%s", dispatcher->str_time_milli()) ;
        }
        ti_separator = " " ;
      }
      if (fields & qmlog::Timezone_Symlink)
        buf.printf("%s%s", ti_separator, dispatcher->str_tz_symlink()) ;
      buf.printf("]") ;
      separator = " " ;
    }
    if (fields & Process_Block) // [program:123] | [program] | [123]
    {
      const char *p_separator = "" ;
      buf.printf("%s[", separator) ;
      if (fields & Name)
      {
        buf.printf("%s", dispatcher->str_name()) ;
        p_separator = ":" ;
      }
      if (fields & Pid)
        buf.printf("%s%s", p_separator, dispatcher->str_pid()) ;
      buf.printf("]") ;
      separator = " " ;
    }
    buf.printf("%s", separator) ;
    int prefix = buf.position() ;

    bool message = (*fmt!='\0') && (fields & qmlog::Message) ;
    bool output_line = line>0 && (fields & qmlog::Line) ;
    bool output_func = func!=NULL && (fields & qmlog::Function) ;
    bool location = output_line or output_func ;
    bool wrap = output_func and (fields & qmlog::Multiline) ;

    if (fields & qmlog::Level)
    {
      buf.printf("%s%s", separator, dispatcher->str_level()) ;
      if (not message and not location)
        buf.printf(".") ;
      else if (not message or not location)
        buf.printf(":") ;
      separator = " " ;
    }

    if (output_line)
    {
      buf.printf("%s" "at %s:%d", separator, file, line) ;
      separator = " " ;
    }

    if (output_func)
    {
      buf.printf("%s" "in %s", separator, func) ;
      separator = " " ;
    }

    if (message and location)
      buf.printf(":") ;

    if (wrap)
    {
      submit_message(level, buf.c_str()) ;
      buf.rewind(prefix) ;
      separator = "-> " ;
    }

    if (message)
    {
      buf.printf("%s", separator) ;
      buf.vprintf(fmt, args) ;
    }

    submit_message(level, buf.c_str()) ;
  }



}

