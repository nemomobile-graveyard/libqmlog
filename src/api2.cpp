#include <syslog.h>

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
    // fprintf(::stderr, "%s\n", __PRETTY_FUNCTION__) ;
    static bool first = true ;
    if (not first)
    {
      // log this as an error using already constructed object...
      // ... and do nothing else
      log_internal("singleton initializer called again") ;
      return ;
    }

    first = false ;

    register_dispatcher(default_dispatcher=new dispatcher_t) ;
    syslog_logger = new qmlog::log_syslog(qmlog::Full, default_dispatcher) ;
    stderr_logger = new qmlog::log_stderr(qmlog::Full, default_dispatcher) ;
    fprintf(::stderr, "syslog_logger=%p, stderr_logger=%p\n", syslog_logger, stderr_logger) ;

    set_process_name(calculate_process_name()) ;
  }

  string object_t::calculate_process_name()
  {
    static const char *anonymous = "<unknown>" ;
    const unsigned int len = 256 ;
    char buf[len+1] ;

    FILE *fp = fopen("/proc/self/cmdline", "r") ;

    if(fp==NULL)
      return anonymous ;

    string name ;

    for(bool word_done=false; not word_done; )
    {
      size_t x = fread(buf, 1, len, fp) ;
      buf[x] = '\0' ;
      word_done = strlen(buf) < len ;
      name += buf ;
    }

    fclose(fp) ;

    const char *p = name.c_str(), *q = strrchr(p, '/') ;

    if(q==NULL) // no slash at all
      return name ;

    if(q[1]=='\0') // trailing slash, again paranoid
      return anonymous ;

    return q+1 ;
  }

  void object_t::set_process_name(const string &new_name)
  {
    process_name = new_name ;
    for(set<dispatcher_t*>::iterator it=dispatchers.begin(); it!=dispatchers.end(); ++it)
      (*it)->set_process_name(process_name) ;
  }

#if 0
  void object_t::init(const char *name)
  {
    static bool first = true ;
    if (not first)
    {
      return ; // XXX: log it
    }
    first = false ;

    current_dispatcher = master = new dispatcher_t(name) ;
  }
#endif

#if 0
  void object_t::register_slave(slave_dispatcher_t *d)
  {
    slaves.insert(d) ;
  }
#endif

  dispatcher_t::dispatcher_t()
  {
    object.register_dispatcher(this) ;
    name = object.get_process_name() ;
    last_pid = (pid_t) 0 ;
    current_level = qmlog::Full ;
    proxy = NULL ;
  }

  dispatcher_t::~dispatcher_t()
  {
    object.unregister_dispatcher(this) ;
    set<dispatcher_t*> slaves_copy = slaves ;
    for(set<dispatcher_t*>::const_iterator it=slaves_copy.begin(); it!=slaves_copy.end(); ++it)
      (*it)->set_proxy(proxy) ;
  }

  void dispatcher_t::set_process_name(const string &new_name)
  {
    name = new_name ;
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

  void dispatcher_t::set_proxy(dispatcher_t *pd)
  {
    if (pd==proxy)
      return ;
    if (proxy) // unregister at current proxy
      proxy->slaves.erase(this) ;
    if (pd) // register at the new one
      pd->slaves.insert(this) ;
    proxy = pd ;
  }

#if 0
  void dispatcher_t::bind_slave(slave_dispatcher_t *d)
  {
    slaves.insert(d) ;
    d->master = this ;
  }

  void dispatcher_t::release_slave(slave_dispatcher_t *d)
  {
    d->master = NULL ;
    slaves.erase(d) ;
  }
#endif

  void dispatcher_t::message(int level)
  {
    const char *empty_format = "" ;
    if (level<=current_level)
      message(level, -1, NULL, NULL, empty_format) ;
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
    const char *empty_format = "" ;
    if (level<=current_level)
      message(level, line, file, func, empty_format) ;
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

  void dispatcher_t::message_failed_assertion(bool abortion, const char *assertion, int line, const char *file, const char *func)
  {
    const char *empty_format = "" ;
    message_failed_assertion(abortion, assertion, line, file, func, empty_format) ;
  }

  void dispatcher_t::message_failed_assertion(bool abortion, const char *assertion, int line, const char *file, const char *func, const char *fmt, ...)
  {
    bool message_follows = fmt[0] != 0 ;

    message(QMLOG_INTERNAL, line, file, func, "Assertion '%s' failed" "%s", assertion, message_follows ? "; detailed info follows:" : ".") ;

    if(message_follows)
    {
      va_list args ;
      va_start(args, fmt) ;
      generic(QMLOG_INTERNAL, -1, "", "", fmt, args) ;
      va_end(args) ;
    }

    message_ndebug(abortion) ;
  }

  void dispatcher_t::message_abortion(bool abortion, int line, const char *file, const char *func)
  {
    message_abortion(abortion, line, file, func) ;
  }

  void dispatcher_t::message_abortion(bool abortion, int line, const char *file, const char *func, const char *fmt, ...)
  {
    bool message_follows = fmt[0] != 0 ;
    message(QMLOG_INTERNAL, line, file, func, "Program is to be aborted" "%s", message_follows ? "; detailed info follows:" : ".") ;

    if(message_follows)
    {
      va_list args ;
      va_start(args, fmt) ;
      generic(QMLOG_INTERNAL, -1, "", "", fmt, args) ;
      va_end(args) ;
    }

    message_ndebug(abortion) ;
  }

  void dispatcher_t::message_ndebug(bool abortion)
  {
    if (abortion)
      message(QMLOG_INTERNAL, "the program will terminate due to internal error above") ;
    else
      message(QMLOG_INTERNAL, "the program execution will be continued as abortion was disabled at compile time") ;
  }

  void dispatcher_t::generic(int level, int line, const char *file, const char *func, const char *fmt, va_list arg)
  {
    if (proxy)
    {
      proxy  -> generic(level, line, file, func, fmt, arg) ;
      return ;
    }

    got_timestamp = got_localtime =
      has_monotonic = has_monotonic_nano = has_monotonic_micro = has_monotonic_milli =
      has_gmt_offset = has_tz_symlink =
      has_date = has_time = has_time_micro = has_time_milli = false ;

    for(set<abstract_log_t*>::iterator it=logs.begin(); it!=logs.end(); ++it)
      if (level<=(*it)->log_level())
        (*it)->compose_message(level, line, file, func, fmt, arg) ;
  }

  void dispatcher_t::get_timestamp()
  {
    if (not got_timestamp)
    {
      // Not checking, if call is successful: nothing can be done even if not
      clock_gettime(CLOCK_MONOTONIC, &monotonic_timestamp) ;
      gettimeofday(&timestamp, NULL) ;
      got_timestamp = true ;
    }
  }

  void dispatcher_t::get_localtime()
  {
    if (not got_localtime)
    {
      get_timestamp() ;
      tzset() ;
      if (not localtime_r(&timestamp.tv_sec, &localtime))
      {
        // theoretically localtime_r() may fail on a 64 bit architecture
        // due to year value overflow, let's fill the structure with zeroes
        // then...
        memset(&localtime, 0, sizeof(struct tm)) ;
      }
      got_localtime = true ;
    }
  }

  const char *dispatcher_t::str_monotonic()
  {
    if (not has_monotonic)
    {
      get_timestamp() ;
      s_mono.rewind(0) ;
      s_mono.printf("%lld", (long long)monotonic_timestamp.tv_sec) ;
      has_monotonic = true ;
    }
    return s_mono.c_str() ;
  }

  const char *dispatcher_t::str_monotonic_nano()
  {
    if (not has_monotonic_nano)
    {
      get_timestamp() ;
      s_mono_nano.rewind(0) ;
      s_mono_nano.printf("%09ld", monotonic_timestamp.tv_nsec) ;
      has_monotonic_nano = true ;
    }
    return s_mono_nano.c_str() ;
  }

  const char *dispatcher_t::str_monotonic_micro()
  {
    if (not has_monotonic_micro)
    {
      get_timestamp() ;
      s_mono_micro.rewind(0) ;
      s_mono_micro.printf("%06ld", monotonic_timestamp.tv_nsec / 1000) ;
      has_monotonic_micro = true ;
    }
    return s_mono_micro.c_str() ;
  }

  const char *dispatcher_t::str_monotonic_milli()
  {
    if (not has_monotonic_milli)
    {
      get_timestamp() ;
      s_mono_milli.rewind(0) ;
      s_mono_milli.printf("%03ld", monotonic_timestamp.tv_nsec / (1000*1000)) ;
      has_monotonic_milli = true ;
    }
    return s_mono_milli.c_str() ;
  }

  const char *dispatcher_t::str_time()
  {
    if (not has_time)
    {
      get_localtime() ;
      s_time.rewind() ;
      s_time.printf("%02d:%02d:%02d", localtime.tm_hour, localtime.tm_min, localtime.tm_sec) ;
      has_time = true ;
    }
    return s_time.c_str() ;
  }

  const char *dispatcher_t::str_time_micro()
  {
    if (not has_time_micro)
    {
      get_timestamp() ;
      s_time_micro.rewind() ;
      s_time_micro.printf("%06d", (int)timestamp.tv_usec) ;
      has_time_micro = true ;
    }
    return s_time_micro.c_str() ;
  }

  const char *dispatcher_t::str_time_milli()
  {
    if (not has_time_milli)
    {
      get_timestamp() ;
      s_time_milli.rewind() ;
      s_time_milli.printf("%03d", (int)timestamp.tv_usec / 1000) ;
      has_time_milli = true ;
    }
    return s_time_milli.c_str() ;
  }

  const char *dispatcher_t::str_gmt_offset()
  {
    if (not has_gmt_offset)
    {
      get_localtime() ;
      s_gmt_offset.rewind() ;
      int sec = localtime.tm_gmtoff ;
      char sign = sec<0 ? (sec = -sec, '-') : '+' ;
      int min = sec/60, hour = min/60 ;
      sec %= 60, min %=60 ;
      // s_gmt_offset.printf("GMT") ;
      if (sec)
        s_gmt_offset.printf("%c" "%d:%02d:%02d", sign, hour, min, sec) ;
      else if(min)
        s_gmt_offset.printf("%c" "%d:%02d", sign, hour, min) ;
      else
        s_gmt_offset.printf("%c" "%d", sign, hour) ;
      has_gmt_offset = true ;
    }
    return s_gmt_offset.c_str() ;
  }

  const char *dispatcher_t::str_tz_symlink()
  {
    if (not has_tz_symlink)
    {
      s_tz_symlink.rewind() ;
      if (s_tz_symlink.readlink("/etc/localtime")<0)
      {
        s_tz_symlink.printf("%m") ;
        tz_symlink_offset = 0 ;
      }
      else
      {
        static const char base[] = "/usr/share/zoneinfo/" ;
        static const int base_len = sizeof(base) - 1 ;
        tz_symlink_offset = strncmp(s_tz_symlink.c_str(), base, base_len) ? 0 : base_len ;
      }
      has_tz_symlink = true ;
    }
    return s_tz_symlink.c_str() + tz_symlink_offset ;
  }

  const char *dispatcher_t::str_date()
  {
    if (not has_date)
    {
      get_localtime() ;
      s_date.rewind() ;
      s_date.printf("%d-%02d-%02d", localtime.tm_year+1900, localtime.tm_mon+1, localtime.tm_mday) ;
      has_date = true ;
    }
    return s_date.c_str() ;
  }

  const char *dispatcher_t::str_tz_abbreviation()
  {
    get_localtime() ;
    return localtime.tm_zone ;
  }

  const char *dispatcher_t::str_name()
  {
    return name.c_str() ;
  }

  const char *dispatcher_t::str_pid()
  {
    pid_t pid = getpid() ;
    if (last_pid != pid)
    {
      s_pid.rewind() ;
      s_pid.printf("%d", pid) ;
      last_pid = pid ;
    }
    return s_pid.c_str() ;
  }

  const char *dispatcher_t::str_level(int level)
  {
    static const char *names[] =
    {
      "INTERNAL ERROR", "CRITICAL ERROR", "ERROR", "WARNING", "NOTICE", "INFO", "DEBUG"
    } ;
    if (qmlog::Internal <= level && (unsigned)level <= qmlog::Debug)
      return names [level-qmlog::Internal] ;
    return "OOPS" ;
  }

  abstract_log_t::abstract_log_t(int maximal_log_level, dispatcher_t *d)
  {
    level = max_level = maximal_log_level ;
    dispatcher = d ?: object.get_default_dispatcher() ;
    dispatcher->attach(this) ;
    fields = 0 ;
    enable_fields(All_Fields) ;
    disable_fields(Time_Micro ^ Time) ;
    disable_fields(Monotonic_Nano ^ Monotonic) ;
  }

  int abstract_log_t::log_level(int new_level)
  {
    if (new_level<=max_level)
      level = new_level ;
    return level ;
  }

  int abstract_log_t::reduce_max_level(int new_max)
  {
    if (new_max > max_level)
      return level ;
    max_level = new_max ;
    if (level > max_level)
      level = max_level ;
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
        buf.printf("%s", dispatcher->str_monotonic()) ;
        if (mono & qmlog::Monotonic_Milli)
        {
          if (mono == qmlog::Monotonic_Nano)
            buf.printf(".%s", dispatcher->str_monotonic_nano()) ;
          else if (mono == qmlog::Monotonic_Micro)
            buf.printf(".%s", dispatcher->str_monotonic_micro()) ;
          else if (mono == qmlog::Monotonic_Milli)
            buf.printf(".%s", dispatcher->str_monotonic_milli()) ;
        }
        ti_separator = " " ;
      }
      if (int tz = fields & qmlog::Timezone_Tm_Block)
      {
        buf.printf("%s(", ti_separator) ;
        const char *tz_separator = "" ;
        if (tz & qmlog::Timezone_Abbreviation)
        {
          buf.printf("%s", dispatcher->str_tz_abbreviation()) ;
          tz_separator = "," ;
        }
        if (tz & qmlog::Timezone_Offset)
          buf.printf("%s" "GMT" "%s", tz_separator, dispatcher->str_gmt_offset()) ;
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
          if (time == qmlog::Time_Micro)
            buf.printf(".%s", dispatcher->str_time_micro()) ;
          else if (time == qmlog::Time_Milli)
            buf.printf(".%s", dispatcher->str_time_milli()) ;
        }
        ti_separator = " " ;
      }
      if (fields & qmlog::Timezone_Symlink)
        buf.printf("%s'%s'", ti_separator, dispatcher->str_tz_symlink()) ;
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
        p_separator = "," ;
      }
      if (fields & Pid)
        buf.printf("%s%s", p_separator, dispatcher->str_pid()) ;
      buf.printf("]") ;
      separator = " " ;
    }
    int prefix = buf.position() ;

    bool message = (*fmt!='\0') && (fields & qmlog::Message) ;
    bool output_line = line>0 && (fields & qmlog::Line) ;
    bool output_func = func!=NULL && (fields & qmlog::Function) ;
    bool location = output_line or output_func ;
    bool wrap = output_func and message and (fields & qmlog::Multiline) ;

    if (fields & qmlog::Level)
    {
      buf.printf("%s%s", separator, dispatcher_t::str_level(level)) ;
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
      separator = " -- " ;
    }

    if (message)
    {
      buf.printf("%s", separator) ;
      buf.vprintf(fmt, args) ;
    }

    submit_message(level, buf.c_str()) ;
  }

  log_file::log_file(const char *path, int maximal_log_level, dispatcher_t *d)
    : abstract_log_t(maximal_log_level, d)
  {
    fp = fopen(path, "a") ;
    to_be_closed = fp != NULL ;
  }

  log_file::log_file(FILE *fp, int maximal_log_level, dispatcher_t *d)
    : abstract_log_t(maximal_log_level, d)
  {
    this->fp = fp ;
    to_be_closed = false ;
  }

  log_file::~log_file()
  {
    if (to_be_closed)
      fclose(fp) ;
  }

  void log_file::submit_message(int /* level */, const char *message)
  {
    if (fp!=NULL)
    {
      fprintf(fp, "%s\n", message) ;
      fflush(fp) ;
    }
  }

  log_stderr::log_stderr(int maximal_log_level, dispatcher_t *d)
    : log_file(::stderr, maximal_log_level, d)
  {
    disable_fields(Timestamp_Mask) ;
    enable_fields(Time) ;
  }

  log_stdout::log_stdout(int maximal_log_level, dispatcher_t *d)
    : log_file(stdout, maximal_log_level, d)
  {
    disable_fields(Timestamp_Mask) ;
    disable_fields(Process_Block) ;
  }

  log_syslog::log_syslog(int maximal_log_level, dispatcher_t *d)
    : abstract_log_t(maximal_log_level, d)
  {
    disable_fields(Timestamp_Mask) ;
    disable_fields(Process_Block) ;
    disable_fields(Multiline) ;
    disable_fields(Timezone_Symlink) ;
    openlog(dispatcher->str_name(), LOG_PID | LOG_NDELAY, LOG_DAEMON);
  }

  log_syslog::~log_syslog()
  {
    closelog() ;
  }

  void log_syslog::submit_message(int level, const char *message)
  {
    static int syslog_names[] =
    {
      LOG_ALERT, LOG_CRIT, LOG_ERR, LOG_WARNING, LOG_NOTICE, LOG_INFO, LOG_DEBUG
    } ;
    assert(qmlog::Internal<=level) ;
    assert(level<=qmlog::Debug) ;
    ::syslog(LOG_DAEMON | syslog_names[level-qmlog::Internal], "%s", message) ;
  }

#if 0
  slave_dispatcher_t::slave_dispatcher_t(const char *name, bool attach_name)
    : dispatcher_t(not attach_name ? name : (string(name)+"|"+object.get_process_name()).c_str())
  {
    master = NULL ;
  }

  slave_dispatcher_t::~slave_dispatcher_t()
  {
    if (master)
      master->release_slave(this) ;
  }

  void slave_dispatcher_t::generic(int level, int line, const char *file, const char *func, const char *fmt, va_list arg)
  {
    dispatcher_t *d = master ? master : this ;
    d->process_message(level, line, file, func, fmt, arg) ;
  }
#endif
}

