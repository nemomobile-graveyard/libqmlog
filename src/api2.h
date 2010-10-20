#ifndef QMLOG_DISPATHCER_H
#define QMLOG_DISPATHCER_H

// :xa
// #include "qm..."
//
#include <string>
#include <set>

#include <stdarg.h>

#ifndef QMLOG_DEFAULT_LOGGING_DISPATCHER
#define QMLOG_DEFAULT_LOGGING_DISPATCHER (qmlog::object.get_current_dispatcher())
#endif

template<int bytes>
struct smart_buffer
{
  char a[bytes], *p ;
  unsigned len, pos ;

  smart_buffer()
  {
    p = a ;
    len = bytes ;
    pos = 0 ;
  }

  unsigned position()
  {
    return pos ;
  }

  void rewind(unsigned at)
  {
    if (at<=pos)
      pos = at ;
  }

  const char *c_str()
  {
    return p ;
  }

  void grow()
  {
    unsigned new_len = len + len ;
    char *q = new char[new_len] ;
    memcpy(q, p, pos) ;
    if (p!=a)
      delete[] p ;
    p = q ;
  }

 ~smart_buffer()
  {
    if (p!=a)
      delete[] p ;
  }

  void printf(const char *fmt, ...) ;
  void vprintf(const char *fmt, va_list args) ;
} ;

namespace qmlog
{
  enum fields
  {
    Multiline             = 1 << 0,
    Message               = 1 << 1,
    Line                  = 1 << 2,
    Function              = 1 << 3,
    Pid                   = 1 << 4,
    Name                  = 1 << 5,
    Monotonic             = 1 << 6,
    Monotonic2            = 1 << 7,
    Monotonic3            = 1 << 8,
    Monotonic4            = 1 << 9,
    Monotonic_Milli       = Monotonic|Monotonic2,
    Monotonic_Micro       = Monotonic|Monotonic2|Monotonic3,
    Monotonic_Nano        = Monotonic|Monotonic2|Monotonic3|Monotonic4,
    Date                  = 1 << 10,
    Time                  = 1 << 11,
    Time2                 = 1 << 12,
    Time3                 = 1 << 13,
    Time4                 = 1 << 14, // unused
    Time_Milli            = Time|Time2,
    Time_Micro            = Time|Time2|Time3,
    Timezone_Symlink      = 1 << 15,
    Timezone_Abbreviation = 1 << 16,
    Timezone_Offset       = 1 << 17,
    Level                 = 1 << 18,

    Monotonic_Mask        = Monotonic|Monotonic2|Monotonic3|Monotonic4,
    Time_Mask             = Time|Time2|Time3|Time4,
    Timestamp_Mask        = Monotonic_Mask|Time_Mask|Date|Timezone_Abbreviation|Timezone_Offset,
    Time_Info_Block       = Timestamp_Mask|Timezone_Symlink,
    Timezone_Tm_Block     = Timezone_Offset|Timezone_Abbreviation,
    Process_Block         = Name|Pid,
    Location_Block        = Line|Function,
  } ;

  enum levels
  {
    Internal = 0,
    Critical = 1,
    Error = 2,
    Warning = 3,
    Notice = 4,
    Info = 5,
    Debug = 6,

    Full = Debug
  } ;

  class object_t ;
  class dispatcher_t ;
  class slave_dispatcher_t ;
  class abstract_log_t ;
  class log_file ;
  class log_stderr ;
  class log_stdout ;
  class log_syslog ;
  class settings_modifier ;

  extern object_t object ;

  class object_t
  {
    static object_t object ;
    static std::string get_process_name() ;
    std::string name ;
    dispatcher_t *current_dispatcher ;
    dispatcher_t *master ;
    std::set<slave_dispatcher_t *> slaves ;
  public:
    void init() ;
    string get_name() ;
    object_t() ;
    void register_slave(slave_dispatcher_t *) ;
    static inline dispatcher_t *get_current_dispatcher() { return object.current_dispatcher ; }
  } ;

  class dispatcher_t
  {
    std::string name ;
    std::set<abstract_log_t *> logs ;
    int current_level ;
    pid_t last_pid ;
    std::string pid_s ;
  public:
    dispatcher_t(const char *name=NULL) ;
    int log_level(int new_level) ;
    int log_level() ;
    void attach(abstract_log_t *);
    void detach(abstract_log_t *);
    void message(int level) ;
    void message(int level, const char *fmt, ...) ;
    void message(int level, int line, const char *file, const char *func) ;
    void message(int level, int line, const char *file, const char *func, const char *fmt, ...) ;
    void generic(int level, int line, const char *file, const char *func, const char *fmt, va_list arg) ;
    void get_timestamp() ;
    void get_struct_tm() ;
    const char *str_monotonic() ;
    const char *str_monotonic_nano() ;
    const char *str_monotonic_micro() ;
    const char *str_monotonic_milli() ;
    const char *str_gmt_offset() ;
    const char *str_tz_abbreviation() ;
    const char *str_date() ;
    const char *str_time() ;
    const char *str_time_micro() ;
    const char *str_time_milli() ;
    const char *str_tz_symlink() ;
    const char *str_name() ;
    const char *str_pid() ;
    const char *str_level() ;
  } ;

  class abstract_log_t
  {
    dispatcher_t *dispatcher ;
    int level, max_level ;
    int fields ;
  public:
    abstract_log_t(int maximal_log_level, dispatcher_t *d) ;
    int log_level(int new_level) ;
    int log_level() ;
    int set_fields(int mask) ;
    int get_fields() ;
    int enable_fields(int mask) ;
    int disable_fields(int mask) ;
    virtual ~abstract_log_t() ;
    virtual void compose_message(int level, int line, const char *file, const char *func, const char *fmt, va_list args) ;
    virtual void submit_message(int level, const char *message) = 0 ;
  } ;

  class log_file : public abstract_log_t
  {
    bool to_be_closed ;
    log_file(const char *path, int maximal_log_level=qmlog::Debug, dispatcher_t *dispatcher=NULL) ;
   ~log_file() ;
    void submit_message(int level, const char *message) ;
  } ;

  class log_stderr : public abstract_log_t
  {
    log_stderr(int maximal_log_level=qmlog::Debug, dispatcher_t *dispatcher=NULL) ;
   ~log_stderr() ;
  } ;

  class log_stdout : public abstract_log_t
  {
    log_stdout(int maximal_log_level=qmlog::Debug, dispatcher_t *dispatcher=NULL) ;
   ~log_stdout() ;
  } ;

  class log_syslog : public abstract_log_t
  {
    log_syslog() ;
   ~log_syslog() ;
    void submit_message(int level, const char *message) ;
  } ;

  class slave_dispatcher_t : public dispatcher_t
  {
  } ;

  class settings_modifier
  {
    settings_modifier() ;
   ~settings_modifier() ;
  } ;

  inline void init() { object.init() ; }
}

#endif // QMLOG_DISPATHCER_H
