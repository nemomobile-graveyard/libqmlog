/*=======================================================================\
#                                                                        $
#   Copyright (C) 2010 Nokia Corporation.                                $
#                                                                        $
#   Author: Ilya Dogolazky <ilya.dogolazky@nokia.com>                    $
#                                                                        $
#     This file is part of qmlog                                         $
#                                                                        $
#     qmlog is free software; you can redistribute it and/or modify      $
#     it under the terms of the GNU Lesser General Public License        $
#     version 2.1 as published by the Free Software Foundation.          $
#                                                                        $
#     qmlog is distributed in the hope that it will be useful, but       $
#     WITHOUT ANY WARRANTY;  without even the implied warranty  of       $
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.               $
#     See the GNU Lesser General Public License  for more details.       $
#                                                                        $
#   You should have received a copy of the GNU  Lesser General Public    $
#   License along with qmlog. If not, see http://www.gnu.org/licenses/   $
\_______________________________________________________________________*/
#define _BSD_SOURCE

#include <cstdarg>
#include <cassert>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <ctime>

#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <syslog.h>

#include <algorithm>
using namespace std ;

#include <qm/log>

bool log_t::use_syslog=false, log_t::use_stderr=false ;
FILE *log_t::fp=NULL ;
const char *log_t::prg_name = "<anonymous>" ;

log_t *log_t::iLogger = NULL;

log_t& log_t::logger()
{
  if(!iLogger)
  {
    //TODO default settings might be changed here
    //default name can be taken from /proc/<pid>/cmdline
    log_init("default", "default.log", false, false);
    log_info("default init for logger");
  }
  return *iLogger;
}

log_t::~log_t()
{
  for(  std::list<LoggerDev*>::iterator it = iDevs.begin();
        it != iDevs.end(); ++it)
  {
    delete (*it);
    iDevs.erase(it);
  }

  delete prev;
  if(fp)
  {
    fclose(fp);
    fp = NULL;
  }

  iLogger = NULL;
}

void log_t::log_init(const char *name, const char *path, bool sys, bool std)
{
  delete iLogger;
  log_t::use_syslog = sys, log_t::use_stderr = std, log_t::prg_name = name ;
  if(path!=NULL && (log_t::fp = fopen(path, "aw"))==NULL)
  {
    log_t::use_stderr = true ;
    log_critical("Can't write to '%s': %s", path, strerror(errno)) ;
    if(std==false)
      log_warning("Stderr logging enabled") ;
  }
  if(sys)
  {
    openlog(log_t::prg_name, LOG_PID|LOG_NDELAY /*LOG_CONS|LOG_PERROR*/, LOG_DAEMON) ;
  }
  iLogger = new log_t(true, LOG_MAX_LEVEL, LOG_MAX_LOCATION) ;

  iLogger->addLoggerDev(new FileLoggerDev("FileLoggerDev.log")); //TODO debug code
}

void log_t::addLoggerDev(LoggerDev* aLoggerDev)
{
  assert(aLoggerDev);
  iDevs.push_front(aLoggerDev);
}

log_t::log_t(bool global, int new_level, int new_mask)
{
  if(new_level<0)
  {
    log_assert(global) ;
    verbosity_level = iLogger->verbosity_level ;
  }
  else
    verbosity_level = new_level ;
  if(new_mask<0)
    location_mask = global ? iLogger->location_mask : LOG_MAX_LOCATION ;
  else
    location_mask = new_mask ;
  if(global)
  {
    prev = iLogger ;
    iLogger = this ;
  }
  else
    prev = NULL ;
}

const char *log_t::level_name(int level)
{
  static const char *names[] =
  {
    "INTERNAL ERROR", "CRITICAL ERROR", "ERROR", "WARNING", "INFO", "DEBUG"
  } ;
  assert(0<=level) ;
  assert((unsigned)level<sizeof(names)/sizeof(*names)) ;
  return names[level] ;
}

int log_t::syslog_level_id(int level)
{
  static int syslog_names[] =
  {
    LOG_ALERT, LOG_CRIT, LOG_ERR, LOG_WARNING, LOG_INFO, LOG_DEBUG
  } ;
  assert(0<=level) ;
  assert((unsigned)level<sizeof(syslog_names)/sizeof(*syslog_names)) ;
  return syslog_names[level] ;
}

void log_t::vlog_generic(int level, bool show_level, const char *fmt, va_list args)
{
  assert(0<=level) ;
  assert(level<=LOG_MAX_LEVEL) ;
  assert(level<=verbosity_level) ;

  if(fmt[0]=='\x01')
    ++fmt ;

  const char *str_level1 = show_level ? level_name(level) : "" ;
  const char *str_level2 = show_level ? ": " : "" ;
  bool has_newline = fmt[strlen(fmt)-1]=='\n' ;

  if(use_stderr)
  {
    fprintf(stderr, "%s[%d]: ", prg_name, getpid()) ;
    fprintf(stderr, "%s%s", str_level1, str_level2) ;
    vfprintf(stderr, fmt, args) ;
    if(!has_newline)
      fprintf(stderr, "\n") ;
  }

  //TODO below is dup of FileLoggerDev
  //kept for test and debug purposes
  // to be removed before release
  if(fp)
  {
    const int time_length = 32 ;
    char buffer[time_length+1] ;

    struct timeval tv ;
    gettimeofday(&tv, NULL) ;
    struct timespec nano ;
    clock_gettime(CLOCK_MONOTONIC, &nano) ;

    time_t t = tv.tv_sec ;
    struct tm time_tm ;
    localtime_r(&t, &time_tm) ;
    char zone[100] ;
    memset(zone, '\0', 100) ;
    strncpy(zone, time_tm.tm_zone, 99) ;
    strftime(buffer, time_length, "%F %T", &time_tm) ;

    char buffer2[1024] ;
    buffer2[0] = '\0' ;

#define PRINT_ETC_LOCALTIME 1
    if(PRINT_ETC_LOCALTIME)
    {
      char buffer_link[1024] ;
      ssize_t res = readlink("/etc/localtime", buffer_link, 1023) ;
      if(res<0)
        strcpy(buffer_link, strerror(errno)) ; // XXX TODO use strerror_r !!
      else
      {
        buffer_link[res] = '\0' ;
        const char *base = "/usr/share/zoneinfo/" ;
        if(strncmp(buffer_link, base, strlen(base))==0)
          strcpy(buffer_link, buffer_link+strlen(base)) ;
      }
      sprintf(buffer2, " '%s'", buffer_link) ;
    }

    fprintf(fp, "[%ld.%03ld (%s) %s.%03ld%s] [%s(%d)] %s%s", nano.tv_sec,
      nano.tv_nsec/1000000, zone, buffer, tv.tv_usec/1000, buffer2, log_t::prg_name, getpid(), str_level1, str_level2) ;
    vfprintf(fp, fmt, args) ;
    if(!has_newline)
      fprintf(fp, "\n") ;
    fflush(fp) ;
  }


  if(use_syslog)
  {
    vsyslog(LOG_DAEMON|syslog_level_id(level), fmt, args) ;
  }
}

void log_t::log_location(int level, bool message_follows, int line, const char *file, const char *func)
{
  if(message_follows)
    log_generic(level, false, "%s at %s:%d in %s:", level_name(level), file, line, func) ;
  else
    log_generic(level, true, "%s:%d in %s.", file, line, func) ;
}

void log_t::log_generic(int level, bool show_level, const char *fmt, ...)
{
  va_list args ;
  va_start(args, fmt) ;
  vlog_generic(level, show_level, fmt, args) ;
  va_end(args) ;
}

void log_t::message(int level, const char *fmt, ...)
{
  assert(0<=level) ;
  assert(level<=LOG_MAX_LEVEL) ;
  if(level>verbosity_level)
    return ;

  va_list args ;
  va_start(args, fmt) ;
  vlog_generic(level, true, fmt, args) ;

  for(  std::list<LoggerDev*>::iterator it = iDevs.begin();
        it != iDevs.end(); ++it)
  {
    (*it)->logGeneric(level, -1, NULL, NULL, fmt, args);
  }

  va_end(args) ;
}

void log_t::message(int level)
{
  //TODO rework this to remove hack completelly
  message(level, "\x01");
}

void log_t::message(int level, int line, const char *file, const char *func, const char *fmt, ...)
{
  assert((1<<level) & LOG_MAX_LOCATION) ;
  assert(0<=level) ;
  assert(level<=LOG_MAX_LEVEL) ;
  if(level>verbosity_level)
    return ;

  //TODO rework this to remove hack completelly
  bool have_a_message = !(fmt[0] == '\x01' && fmt[1] == 0), location_shown = false ;

  if((1<<level)&location_mask) // show location
  {
    log_location(level, have_a_message, line, file, func) ;
    location_shown = true ;
  }

  if(true)
  {
    va_list args ;
    va_start(args, fmt) ;
    if(have_a_message) vlog_generic(level, !location_shown, fmt, args) ;

    for(  std::list<LoggerDev*>::iterator it = iDevs.begin();
          it != iDevs.end(); ++it)
    {
      (*it)->logGeneric(level, line, file, func, fmt, args);
    }

    va_end(args) ;
  }
}

void log_t::message(int level, int line, const char *file, const char *func)
{
  //TODO rework this to remove hack completelly
  message(level, line, file, func, "\x01");
}

void log_t::log_failed_assertion(const char *assertion, int line, const char *file, const char *func, const char *fmt, ...)
{
  log_location(LOG_LEVEL_INTERNAL, true, line, file, func) ;

  //TODO rework this to remove hack completelly
  bool message_follows = !(fmt[0] == '\x01' && fmt[1] == 0);

  log_generic(LOG_LEVEL_INTERNAL, false, "Assertion failed: %s%s.", assertion, message_follows?". Detailed info follows":"") ;
  if(message_follows)
  {
    va_list args ;
    va_start(args, fmt) ;
    vlog_generic(LOG_LEVEL_INTERNAL, true, fmt, args) ;
    va_end(args) ;
  }
  abort() ; do { sleep(1) ; } while (true) ;
}

void log_t::log_failed_assertion(const char *assertion, int line, const char *file, const char *func)
{
  //TODO rework this to remove hack completelly
  log_failed_assertion(assertion, line, file, func, "\x01");
}


















//========== Logger Settings ==============
//TODO move to separate file

LoggerSettings::LoggerSettings(int new_verbosity_level, int new_location_mask, int new_message_format)
  : verbosity_level(new_verbosity_level)
  , location_mask(new_location_mask)
  , message_format(new_message_format)
{
}

void LoggerSettings::setVerbosityLevel(int new_verbosity_level)
{
  //TODO insert assert here
  verbosity_level = new_verbosity_level;
}

void LoggerSettings::setLocationMask(int new_location_mask)
{
  //TODO insert assert here
  location_mask = new_location_mask;
}

void LoggerSettings::setMessageFormat(int new_message_format)
{
  //TODO insert assert here
  message_format = new_message_format;
}

bool LoggerSettings::isLogShown(int aLevel) const
{
  return aLevel <= verbosity_level;
}

bool LoggerSettings::isLocationShown(int aLevel) const
{
  return (1 << aLevel) & location_mask;
}

bool LoggerSettings::isMTimerMs() const
{
  return (message_format & EMTimerMs) && !(message_format & EMTimerNs);
}

bool LoggerSettings::isMTimerNs() const
{
  return !(message_format & EMTimerMs) && (message_format & EMTimerNs);
}

bool LoggerSettings::isMTimer() const
{
  return message_format & EMTimer;
}

bool LoggerSettings::isTzAbbr() const
{
  return message_format & ETzAbbr;
}

bool LoggerSettings::isDate() const
{
  return message_format & EDate;
}

bool LoggerSettings::isTimeMs() const
{
  return (message_format & ETimeMs) && !(message_format & ETimeNs);
}

bool LoggerSettings::isTimeNs() const
{
  return !(message_format & ETimeMs) && (message_format & ETimeNs);
}

bool LoggerSettings::isTime() const
{
  return message_format & ETime;
}

bool LoggerSettings::isTzSymLink() const
{
  return message_format & ETzSymLink;
}

bool LoggerSettings::isDateTimeInfo() const
{
  return message_format & EDateTimeInfo;
}

bool LoggerSettings::isName() const
{
  return message_format & EName;
}

bool LoggerSettings::isPid() const
{
  return message_format & EPid;
}

bool LoggerSettings::isProcessInfo() const
{
  return message_format & EProcessInfo;
}


bool LoggerSettings::isFunc() const
{
  return message_format & EFunc;
}

bool LoggerSettings::isFileLine() const
{
  return message_format & EFileLine;
}

bool LoggerSettings::isMessage() const
{
  return message_format & EMessage;
}


bool LoggerSettings::isDebugInfo() const
{
  return message_format & EDebugInfo;
}

bool LoggerSettings::isWordWrap() const
{
  return message_format & EWordWrap;
}










//======== LoggerDev =============
//TODO move to separate file
LoggerDev::LoggerDev(int aVerbosityLevel, int aLocationMask, int aMessageFormat)
  : iSettings(aVerbosityLevel, aLocationMask, aMessageFormat)
{
}

LoggerDev::~LoggerDev()
{
}

int LoggerDev::snprintfappend(char * str, int buf_len, int current_len, const char * fmt, ...)
{
  va_list args ;
  va_start(args, fmt) ;
  int ret = vsnprintfappend(str, buf_len, current_len, fmt, args);
  va_end(args) ;
  return ret;
}

int LoggerDev::vsnprintfappend(char * str, int buf_len, int current_len, const char * fmt, va_list arg)
{
  return current_len + vsnprintf((char*)(str + current_len), buf_len - current_len - 1, fmt, arg);
}

void LoggerDev::logGeneric(int aLevel, int aLine, const char *aFile, const char *aFunc,
                           const char *aFmt, va_list anArgs)
{
  //TODO asser below shall be in another place
  assert(0<=aLevel) ;
  assert(aLevel<=LOG_MAX_LEVEL) ;
  if(!settings().isLogShown(aLevel))
    return;

  //TODO remove this hack later
  if(aFmt[0]=='\x01')
    ++aFmt ;

  const int dateInfoLen = 256;
  char dateInfo[dateInfoLen];
  memset(dateInfo, '\0', dateInfoLen);
  int dateInfoCurrentLen = 0;
  bool addSpace = false;

  if(settings().isDateTimeInfo())
  {
    if(settings().isMTimer())
    {
      struct timespec nano;
      clock_gettime(CLOCK_MONOTONIC, &nano);
      dateInfoCurrentLen = snprintfappend(dateInfo, dateInfoLen, dateInfoCurrentLen, "%ld", nano.tv_sec);

      if(settings().isMTimerMs())
      {
        dateInfoCurrentLen = snprintfappend(dateInfo, dateInfoLen, dateInfoCurrentLen, ".%03ld", nano.tv_nsec/1000000);
      }
      else if(settings().isMTimerNs())
      {
        dateInfoCurrentLen = snprintfappend(dateInfo, dateInfoLen, dateInfoCurrentLen, ".%09ld", nano.tv_nsec);
      }
      addSpace = true;
    }

    struct timeval tv ;
    gettimeofday(&tv, NULL) ;
    time_t t = tv.tv_sec ;
    struct tm time_tm ;
    localtime_r(&t, &time_tm) ;

    if(settings().isTzAbbr())
    {
      char zone[100] ;
      memset(zone, '\0', 100) ;
      strncpy(zone, time_tm.tm_zone, 99); //TODO do I need strncmp now?
      dateInfoCurrentLen = snprintfappend(dateInfo, dateInfoLen, dateInfoCurrentLen, addSpace? " (%s)": "(%s)", zone);
      addSpace = true;
    }

    if(settings().isDate() || settings().isTime())
    {
      const char *fmt = (settings().isDate() && settings().isTime())? "%F %T" : (settings().isDate()? "%F" : "%T" );
      const int time_length = 32 ;
      char buffer[time_length+1] ; //TODO rename
      strftime(buffer, time_length, fmt, &time_tm) ;

      dateInfoCurrentLen = snprintfappend(dateInfo, dateInfoLen, dateInfoCurrentLen, addSpace? " %s": "%s", buffer);

      if(settings().isTime())
      {
        if(settings().isTimeMs())
        {
          dateInfoCurrentLen = snprintfappend(dateInfo, dateInfoLen, dateInfoCurrentLen, ".%03ld", tv.tv_usec/1000);
        }
        else if(settings().isTimeNs()) //TODO isTimeNs is microseconds... rename later
        {
          dateInfoCurrentLen = snprintfappend(dateInfo, dateInfoLen, dateInfoCurrentLen, ".%06ld", tv.tv_usec);
        }
      }
      addSpace = true;
    }

    if(settings().isTzSymLink())
    {
      char buffer_link[1024] ;
      ssize_t res = readlink("/etc/localtime", buffer_link, 1023) ;
      if(res<0)
        strcpy(buffer_link, strerror(errno)) ; // XXX TODO use strerror_r !!
      else
      {
        buffer_link[res] = '\0' ;
        const char *base = "/usr/share/zoneinfo/" ;
        if(strncmp(buffer_link, base, strlen(base))==0)
          strncpy(buffer_link, buffer_link+strlen(base), 1024) ;
      }

      dateInfoCurrentLen = snprintfappend(dateInfo, dateInfoLen, dateInfoCurrentLen, addSpace?  " \'%s\'":"\'%s\'", buffer_link);
    }
  }
  addSpace = false;

  const int processInfoLen = 64;
  char processInfo[processInfoLen];
  memset(processInfo, '\0', processInfoLen);
  int processInfoCurrentLen = 0;

  if(settings().isProcessInfo())
  {
    if(settings().isName())
    {
      processInfoCurrentLen = snprintfappend(processInfo, processInfoLen, processInfoCurrentLen, "%s", log_t::prg_name);
    }

    if(settings().isPid())
    {
      processInfoCurrentLen = snprintfappend(processInfo, processInfoLen, processInfoCurrentLen, "(%d)", getpid());
    }
  }

  const int debugInfoLen = 1024;
  char debugInfo[debugInfoLen];
  memset(debugInfo, '\0', debugInfoLen);
  int debugInfoCurrentLen = 0;
  bool isFullDebugInfo = false;

  if(settings().isDebugInfo() && settings().isLocationShown(aLevel))
  {
    if(settings().isFileLine() && aLine > 0 && aFile)
    {
      debugInfoCurrentLen = snprintfappend(debugInfo, debugInfoLen, debugInfoCurrentLen, "%s:%d", aFile, aLine);
      addSpace = true;
    }

    if(settings().isFunc() && aFunc)
    {
      debugInfoCurrentLen = snprintfappend(debugInfo, debugInfoLen, debugInfoCurrentLen, addSpace? " in %s": "in %s", aFunc);
    }

    isFullDebugInfo = (aLine > 0 && aFile && aFunc) && (settings().isFileLine() && settings().isFunc());
  }

  const int messageLen = 1024;
  char message[messageLen];
  memset(message, '\0', messageLen);
  vsnprintf(message, messageLen, aFmt, anArgs) ;

  vlogGeneric(aLevel, dateInfo, processInfo, debugInfo, isFullDebugInfo, message);

}

void LoggerDev::setSettings(const LoggerSettings& aSettings)
{
  iSettings = aSettings;
}

const LoggerSettings& LoggerDev::settings() const
{
  return iSettings;
}








//================== FileLoggerDev ===============
//TODO move to separate file
FileLoggerDev::FileLoggerDev(const char *aFileName)
  : LoggerDev(DefaultLevel, DefaultLocation, DefaultFormat)
  , iFp(NULL)
  , iIsFpOwner(false)
{
  assert(aFileName);
  iFp = fopen(aFileName, "aw");
  iIsFpOwner = iFp;
}

FileLoggerDev::~FileLoggerDev()
{
  if(iIsFpOwner && iFp)
  {
    fclose(iFp);
    iFp = NULL;
  }
}

FileLoggerDev::FileLoggerDev(FILE *aFp, bool aTakeOwnership, int aVerbosityLevel, int aLocationMask, int aMessageFormat)
  : LoggerDev(aVerbosityLevel, aLocationMask, aMessageFormat)
  , iFp(aFp)
  , iIsFpOwner(aTakeOwnership)
{
}

void FileLoggerDev::vlogGeneric(int aLevel, const char *aDateTimeInfo, const char* aProcessInfo,
                                const char *aDebugInfo, bool aIsFullDebugInfo, const char *aMessage)
{
  bool hasPrefix = vlogPrefixes(aDateTimeInfo, aProcessInfo);
  bool hasDebugInfo = vlogDebugInfo(aLevel, aDebugInfo, hasPrefix);
  bool hasMessage = (aMessage && aMessage[0] != 0);

  if(hasMessage)
  {
    if(aIsFullDebugInfo && settings().isWordWrap())
    {
      fprintf(iFp, ":\n");
      hasPrefix = vlogPrefixes(aDateTimeInfo, aProcessInfo);
    }

    fprintf(iFp, " %s", aMessage);
  }
  else if(hasDebugInfo)
  {
    fprintf(iFp, ".");
  }

  fprintf(iFp, "\n");
  fflush(iFp);
}

bool FileLoggerDev::vlogPrefixes(const char *aDateTimeInfo, const char* aProcessInfo)
{
  bool hasDateTimeInfo = (aDateTimeInfo && aDateTimeInfo[0] != 0);
  bool hasProcessInfo = (aProcessInfo && aProcessInfo[0] != 0);

  if(hasDateTimeInfo)
  {
    fprintf(iFp, "[%s]", aDateTimeInfo);
  }

  if(hasProcessInfo)
  {
    fprintf(iFp, hasDateTimeInfo? " [%s]": "[%s]", aProcessInfo);
  }

  return (hasDateTimeInfo | hasProcessInfo);
}

bool FileLoggerDev::vlogDebugInfo(int aLevel, const char *aDebugInfo, bool aPrefixExists)
{
  bool hasDebugInfo = (aDebugInfo && aDebugInfo[0] != 0); 

  fprintf(iFp, hasDebugInfo?  (aPrefixExists? " %s at": "%s at"):
                              (aPrefixExists? " %s:":"%s:"), log_t::level_name(aLevel));

  if(hasDebugInfo)
  {
    fprintf(iFp, aPrefixExists? " %s": "%s", aDebugInfo);
  }

  return hasDebugInfo;
}

#if 0
int main()
{
  log_init("log_test", "log.log", true, true) ;
  log_info("info, 2x2=%d", 4) ;
  log_debug("info, 2x2=%d", 5) ;
  log_debug() ;
  log_assert(false, "information: %d", 239*239) ;
  return 0 ;
}
#endif
