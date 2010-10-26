#ifndef QMLOG_API2_H
#define QMLOG_API2_H

#include <sys/time.h>

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include <string>
#include <set>

#ifndef QMLOG_DEFAULT_DISPATCHER
#define QMLOG_DEFAULT_DISPATCHER qmlog::object.get_current_dispatcher()
#endif

#define QMLOG_NONE     0
#define QMLOG_INTERNAL 1
#define QMLOG_CRITICAL 2
#define QMLOG_ERROR    3
#define QMLOG_WARNING  4
#define QMLOG_NOTICE   5
#define QMLOG_INFO     6
#define QMLOG_DEBUG    7
#define QMLOG_FULL     QMLOG_DEBUG

#ifndef QMLOG_LEVEL
#define QMLOG_LEVEL QMLOG_FULL
#endif

#if ! (QMLOG_NONE <= QMLOG_LEVEL && QMLOG_LEVEL <= QMLOG_FULL)
#error QMLOG_LEVEL is outside of [0..7]
#endif

#ifndef QMLOG_LOCATION_MASK
#define QMLOG_LOCATION_MASK ((1<<QMLOG_INTERNAL)|(1<<QMLOG_INFO)|(1<<QMLOG_DEBUG))
#endif

#define QMLOG_LOCATION __LINE__,__FILE__,__PRETTY_FUNCTION__

#ifdef NDEBUG
#define QMLOG_ABORTION 0
#else
#define QMLOG_ABORTION 1
#endif

#include <cassert>

#if QMLOG_LEVEL >= QMLOG_INTERNAL
# define log_abort(...) do { (QMLOG_DEFAULT_DISPATCHER)->message_abortion(QMLOG_ABORTION, QMLOG_LOCATION, ##__VA_ARGS__) ; assert(0) ; } while(0)
# define log_assert(x, ...) do if(not(x)) { (QMLOG_DEFAULT_DISPATCHER)->message_failed_assertion(QMLOG_ABORTION, #x, QMLOG_LOCATION, ##__VA_ARGS__) ; assert(0) ; } while(0)
#else
# define log_abort(...) assert(0)
# define log_assert(x, ...) assert(x)
#endif

#if QMLOG_LEVEL >= QMLOG_CRITICAL
# if (QMLOG_LOCATION_MASK) & (1<<QMLOG_CRITICAL)
#  define log_critical(...) (QMLOG_DEFAULT_DISPATCHER)->message(QMLOG_CRITICAL, QMLOG_LOCATION, ## __VA_ARGS__)
# else
#  define log_critical(...) (QMLOG_DEFAULT_DISPATCHER)->message(QMLOG_CRITICAL, ## __VA_ARGS__)
# endif
#else
# define log_critical(...) (void)(0)
#endif

#if QMLOG_LEVEL >= QMLOG_ERROR
# if (QMLOG_LOCATION_MASK) & (1<<QMLOG_ERROR)
#  define log_error(...) (QMLOG_DEFAULT_DISPATCHER)->message(QMLOG_ERROR, QMLOG_LOCATION, ## __VA_ARGS__)
# else
#  define log_error(...) (QMLOG_DEFAULT_DISPATCHER)->message(QMLOG_ERROR, ## __VA_ARGS__)
# endif
#else
# define log_error(...) (void)(0)
#endif

#if QMLOG_LEVEL >= QMLOG_WARNING
# if (QMLOG_LOCATION_MASK) & (1<<QMLOG_WARNING)
#  define log_warning(...) (QMLOG_DEFAULT_DISPATCHER)->message(QMLOG_WARNING, QMLOG_LOCATION, ## __VA_ARGS__)
# else
#  define log_warning(...) (QMLOG_DEFAULT_DISPATCHER)->message(QMLOG_WARNING, ## __VA_ARGS__)
# endif
#else
# define log_warning(...) (void)(0)
#endif

#if QMLOG_LEVEL >= QMLOG_NOTICE
# if (QMLOG_LOCATION_MASK) & (1<<QMLOG_NOTICE)
#  define log_notice(...) (QMLOG_DEFAULT_DISPATCHER)->message(QMLOG_NOTICE, QMLOG_LOCATION, ## __VA_ARGS__)
# else
#  define log_notice(...) (QMLOG_DEFAULT_DISPATCHER)->message(QMLOG_NOTICE, ## __VA_ARGS__)
# endif
#else
# define log_notice(...) (void)(0)
#endif

#if QMLOG_LEVEL >= QMLOG_INFO
# if (QMLOG_LOCATION_MASK) & (1<<QMLOG_INFO)
#  define log_info(...) (QMLOG_DEFAULT_DISPATCHER)->message(QMLOG_INFO, QMLOG_LOCATION, ## __VA_ARGS__)
# else
#  define log_info(...) (QMLOG_DEFAULT_DISPATCHER)->message(QMLOG_INFO, ## __VA_ARGS__)
# endif
#else
# define log_info(...) (void)(0)
#endif

#if QMLOG_LEVEL >= QMLOG_DEBUG
# if (QMLOG_LOCATION_MASK) & (1<<QMLOG_DEBUG)
#  define log_debug(...) (QMLOG_DEFAULT_DISPATCHER)->message(QMLOG_DEBUG, QMLOG_LOCATION, ## __VA_ARGS__)
# else
#  define log_debug(...) (QMLOG_DEFAULT_DISPATCHER)->message(QMLOG_DEBUG, ## __VA_ARGS__)
# endif
#else
# define log_debug(...) (void)(0)
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

  void rewind()
  {
    pos = 0 ;
  }

  const char *c_str()
  {
    return p ;
  }

  void grow()
  {
    unsigned new_len = len + len ;
    char *q = new char[new_len] ;
    if (pos>0)
      memcpy(q, p, pos) ;
    if (p!=a)
      delete[] p ;
    p = q ;
    len = new_len ;
  }

 ~smart_buffer()
  {
    if (p!=a)
      delete[] p ;
  }

  void printf(const char *fmt, ...) __attribute__((format(printf, 2, 3)))
  {
    va_list args ;
    va_start(args, fmt) ;
    vprintf(fmt, args) ;
    va_end(args) ;
  }

  void vprintf(const char *fmt, va_list args)
  {
    for(bool written = false; not written; )
    {
      unsigned space = len - pos ;
      unsigned needed = vsnprintf(p+pos, space, fmt, args) ;
      written = needed < space ;
      if (not written)
        grow() ;
      else
        pos += needed ;
    }
  }

  int readlink(const char *path)
  {
    for(bool success = false; not success; )
    {
      unsigned space = len - pos ;
      ssize_t res = ::readlink(path, p+pos, space) ;
      if (res<0) // error! don't retry
        return -1 ;
      unsigned res_u = res ;
      if (res_u < space)
      {
        p[pos+res_u] = '\0' ;
        pos += res_u ;
        success = true ;
      }
      else
        grow() ;
    }
    return 0 ;
  }
} ;

typedef struct smart_buffer<1>  dynamic_buffer ;

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
    All_Fields            = (1<<19)-1
  } ;

  enum levels
  {
    None     = QMLOG_NONE,
    Internal = QMLOG_INTERNAL,
    Critical = QMLOG_CRITICAL,
    Error    = QMLOG_ERROR,
    Warning  = QMLOG_WARNING,
    Notice   = QMLOG_NOTICE,
    Info     = QMLOG_INFO,
    Debug    = QMLOG_DEBUG,

    Full     = QMLOG_FULL
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
    // static object_t object ;
    std::string name ;
    dispatcher_t *current_dispatcher ;
    dispatcher_t *master ;
    std::set<slave_dispatcher_t *> slaves ;
  public:
    void init() ;
    static std::string get_process_name() ;
    object_t() ;
    void register_slave(slave_dispatcher_t *) ;
    static inline dispatcher_t *get_current_dispatcher() { return object.current_dispatcher ; }
  } ;

  class dispatcher_t
  {
    std::string name ;
    std::set<abstract_log_t *> logs ;
    std::set<slave_dispatcher_t*> slaves ;

    void get_timestamp() ;
    bool got_timestamp ;
    struct timespec monotonic_timestamp ;
    struct timeval timestamp ;

    void get_localtime() ;
    bool got_localtime ;
    struct tm localtime ;

    bool has_monotonic ;
    bool has_monotonic_nano ;
    bool has_monotonic_micro ;
    bool has_monotonic_milli ;
    dynamic_buffer s_mono, s_mono_nano, s_mono_micro, s_mono_milli ;

    bool has_gmt_offset ;
    dynamic_buffer s_gmt_offset ;

    bool has_tz_symlink ;
    int tz_symlink_offset ;
    dynamic_buffer s_tz_symlink ;

    bool has_date ;
    dynamic_buffer s_date ;

    bool has_time ;
    bool has_time_micro ;
    bool has_time_milli ;
    dynamic_buffer s_time, s_time_micro, s_time_milli ;

    pid_t last_pid ;
    dynamic_buffer s_pid ;

    int current_level ;
  public:
    dispatcher_t(const char *name=NULL) ;
    virtual ~dispatcher_t() ;
    int log_level(int new_level) ;
    int log_level() ;
    void attach(abstract_log_t *) ;
    void detach(abstract_log_t *) ;
    void bind_slave(slave_dispatcher_t *) ;
    void release_slave(slave_dispatcher_t *) ;
    void message(int level) ;
    void message(int level, const char *fmt, ...) __attribute__((format(printf,3,4))) ;
    void message(int level, int line, const char *file, const char *func) ;
    void message(int level, int line, const char *file, const char *func, const char *fmt, ...) __attribute__((format(printf,6,7))) ;
    void message_failed_assertion(bool abortion, const char *assertion, int line, const char *file, const char *func) ;
    void message_failed_assertion(bool abortion, const char *assertion, int line, const char *file, const char *func, const char *fmt, ...) __attribute__((format(printf,7,8))) ;
    void message_abortion(bool abortion, int line, const char *file, const char *func) ;
    void message_abortion(bool abortion, int line, const char *file, const char *func, const char *fmt, ...) __attribute__((format(printf,6,7))) ;
    void message_ndebug(bool abortion) ;
    virtual void generic(int level, int line, const char *file, const char *func, const char *fmt, va_list arg) ;
    void process_message(int level, int line, const char *file, const char *func, const char *fmt, va_list arg) ;
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
    static const char *str_level(int) ;
  } ;

  class abstract_log_t
  {
  protected:
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
    FILE *fp ;
  public:
    log_file(const char *path, int maximal_log_level=qmlog::Full, dispatcher_t *dispatcher=NULL) ;
    log_file(FILE *fp, int maximal_log_level=qmlog::Full, dispatcher_t *dispatcher=NULL) ;
   ~log_file() ;
    void submit_message(int level, const char *message) ;
  } ;

  class log_stderr : public log_file
  {
  public:
    log_stderr(int maximal_log_level=qmlog::Full, dispatcher_t *dispatcher=NULL) ;
   ~log_stderr() { }
  } ;

  class log_stdout : public log_file
  {
  public:
    log_stdout(int maximal_log_level=qmlog::Full, dispatcher_t *dispatcher=NULL) ;
   ~log_stdout() { }
  } ;

  class log_syslog : public abstract_log_t
  {
  public:
    log_syslog(int maximal_log_level=qmlog::Full, dispatcher_t *dispatcher=NULL) ;
   ~log_syslog() ;
    void submit_message(int level, const char *message) ;
  } ;

  class slave_dispatcher_t : public dispatcher_t
  {
  public:
    dispatcher_t *master ;
    slave_dispatcher_t(const char *name, bool attach_name=true) ;
   ~slave_dispatcher_t() ;
    void generic(int level, int line, const char *file, const char *func, const char *fmt, va_list arg) ;
  } ;

  inline void init() { object.init() ; }
}

#endif // QMLOG_DISPATHCER_H
