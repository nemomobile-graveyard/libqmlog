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

//TODO I use stringstream to make output...
// there is faster way to do this but I need
// something like snprintfapend()
// It is not hard to create it... but is it necessary?
#include <sstream>

#include <algorithm>
using namespace std ;

#include <qm/log>
#include <QDebug> //TODO remove

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
  qDebug() << "fp =" << fp; //TODO <<<--- remove
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
  iDev = std::auto_ptr<LoggerDev>(aLoggerDev);
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

  if(iDev.get())
  {
    iDev->logGeneric(level, show_level, fmt, args);
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

  fprintf(fp, "[%ld (%s) %s%s] [%s(%d)] %s%s", nano.tv_sec,
    zone, buffer, buffer2, log_t::prg_name, getpid(), str_level1, str_level2) ;
/*disabled MTimerMs amd MtimeMs  fprintf(fp, "[%ld.%03ld (%s) %s.%03ld%s] [%s(%d)] %s%s", nano.tv_sec,
    nano.tv_nsec/1000000, zone, buffer, tv.tv_usec/1000, buffer2, log_t::prg_name, getpid(), str_level1, str_level2) ;*/
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

  if(have_a_message)
  {
    va_list args ;
    va_start(args, fmt) ;
    vlog_generic(level, !location_shown, fmt, args) ;
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

void LoggerDev::logGeneric(int aLevel, bool aShowLevel, const char *aFmt, va_list anArgs)
{
  //TODO asser below shall be in another place
  assert(0<=aLevel) ;
  assert(aLevel<=LOG_MAX_LEVEL) ;
  if(!settings().isLogShown(aLevel))
    return;

  //TODO remove this hack later
  if(aFmt[0]=='\x01')
    ++aFmt ;

  FILE* fp = fopen("temp_test.log", "aw");

  const char *str_level1 = aShowLevel ? log_t::level_name(aLevel) : "" ;
  const char *str_level2 = aShowLevel ? ": " : "" ;
  bool has_newline = aFmt[strlen(aFmt)-1]=='\n' ;

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

  fprintf(fp, "[%ld (%s) %s%s] [%s(%d)] %s%s", nano.tv_sec,
    zone, buffer, buffer2, log_t::prg_name, getpid(), str_level1, str_level2) ;
/*disabled MTimerMs amd MtimeMs  fprintf(fp, "[%ld.%03ld (%s) %s.%03ld%s] [%s(%d)] %s%s", nano.tv_sec,
    nano.tv_nsec/1000000, zone, buffer, tv.tv_usec/1000, buffer2, log_t::prg_name, getpid(), str_level1, str_level2) ;*/
  vfprintf(fp, aFmt, anArgs) ;
  if(!has_newline)
    fprintf(fp, "\n") ;
  fflush(fp);
  fclose(fp);




#if 0
//TODO is it ok to use std::string and std::stringstream?
// if not, need to follow method below
// need to create something like snprintfappend

  //TODO shall I move it to class attributes?
  char dateInfo[1024];
  memset(dateInfo, '\0', 1024);
  char processInfo[1024];
  memset(processInfo, '\0', 1024);
  char message[1024];
  memset(message, '\0', 1024);

  if(settings().isMTimer())
  {
    struct timespec nano;
    clock_gettime(CLOCK_MONOTONIC, &nano);
    snprintf(dateInfo, 1024, "%ld", nano.tv_sec);

    if(settings().isMTimerMs())
    {
      snprintf(dateInfo, 1024, ".%03ld", nano.tv_nsec/1000000);
    }
    else if(settings().isMTimerNs())
    {
      snprintf(dateInfo, 1024, ".%09ld", nano.tv_nsec);
    }
  }

  vlogGeneric(aLevel, aShowLevel, dateInfo, processInfo, message);
#endif



  std::stringstream dateInfo;
  bool addSpace = false;

  if(settings().isDateTimeInfo())
  {
    if(settings().isMTimer())
    {
      struct timespec nano;
      clock_gettime(CLOCK_MONOTONIC, &nano);
      dateInfo << nano.tv_sec;
      //TODO snprintfappend(dateInfo, 1024, "%ld", nano.tv_sec);

      if(settings().isMTimerMs())
      {
        dateInfo << '.';
        dateInfo.fill('0');
        dateInfo.width(3);
        dateInfo << right << (int)nano.tv_nsec/1000000;
        //TODO snprintfappend(dateInfo, 1024, ".%03ld", nano.tv_nsec/1000000);
      }
      else if(settings().isMTimerNs())
      {
        dateInfo << '.';
        dateInfo.fill('0');
        dateInfo.width(9);
        dateInfo << right << (int)nano.tv_nsec;
        //TODO snprintfappend(dateInfo, 1024, ".%09ld", nano.tv_nsec);
      }
      addSpace = true;
    }

    {//TODO <<<--- remove

    struct timeval tv ;
    gettimeofday(&tv, NULL) ;
    time_t t = tv.tv_sec ;
    struct tm time_tm ;
    localtime_r(&t, &time_tm) ;

    if(settings().isTzAbbr())
    {
      if(addSpace)
      {
        dateInfo << ' ';
      //TODO snprintfappend(dateInfo, 1024, " ");
      }

      char zone[100] ;
      memset(zone, '\0', 100) ;
      strncpy(zone, time_tm.tm_zone, 99); //TODO do I need strncmp now?
      dateInfo << '(' << zone << ')';
      //TODO snprintfappend(dateInfo, 1024, "(%s)", zone);
      addSpace = true;
    }

    if(settings().isDate() || settings().isTime())
    {
      if(addSpace)
      {
        dateInfo << ' ';
      //TODO snprintfappend(dateInfo, 1024, " ");
      }

      const char *fmt = (settings().isDate() && settings().isTime())? "%F %T" : (settings().isDate()? "%F" : "%T" );
      const int time_length = 32 ;
      char buffer[time_length+1] ; //TODO rename
      strftime(buffer, time_length, fmt, &time_tm) ;

      dateInfo << buffer;
      //TODO snprintfappend(dateInfo, 1024, "%s", buffer);

      if(settings().isTime())
      {
        if(settings().isTimeMs())
        {
          dateInfo << '.';
          dateInfo.fill('0');
          dateInfo.width(3);
          dateInfo << right << (int)tv.tv_usec/1000;
          //TODO snprintfappend(dateInfo, 1024, ".%03ld", tv.tv_usec/1000);
        }
        else if(settings().isTimeNs()) //TODO isTimeNs is microseconds... rename later
        {
          dateInfo << '.';
          dateInfo.fill('0');
          dateInfo.width(6);
          dateInfo << right << tv.tv_usec;
          //TODO snprintfappend(dateInfo, 1024, ".%06ld", tv.tv_usec);
        }
      }
      addSpace = true;
    }

    if(settings().isTzSymLink())
    {
      if(addSpace)
      {
        dateInfo << ' ';
      //TODO snprintfappend(dateInfo, 1024, " ");
      }

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

      dateInfo << '\'' << buffer_link << '\'';
      //TODO snprintfappend(dateInfo, 1024, "\'%s\'", buffer_link);
      addSpace = true;
    }

    }//TODO <<<--- remove
  }




  addSpace = false;
  std::stringstream processInfo;

  if(settings().isProcessInfo())
  {
    if(settings().isName())
    {
      processInfo << log_t::prg_name;
      //TODO snprintfappend(dateInfo, 1024, "%s", log_t::prg_name);
    }

    if(settings().isPid())
    {
      processInfo << '(' << getpid() << ')';
      //TODO snprintfappend(dateInfo, 1024, "(%s)", getpid());
    }
  }


  char message[1024];
  memset(message, '\0', 1024);
  vsnprintf(message, 1024, aFmt, anArgs) ;



  vlogGeneric(aLevel, aShowLevel, dateInfo.str().c_str(), processInfo.str().c_str(), message);

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

//TODO aShowLevel is not required probably
//better to add it into format!
void FileLoggerDev::vlogGeneric(int aLevel, bool aShowLevel, const char *aDateTimeInfo,
                                const char* aProcessInfo, const char *aMessage)
{
  bool hasDateTimeInfo = (aDateTimeInfo && aDateTimeInfo[0] != 0);
  bool hasProcessInfo = (aProcessInfo && aProcessInfo[0] != 0);
  bool hasDebugInfo = true; //TODO add real condition
  bool hasFullDebugInfo = false; //TODO add real condition
  bool hasMessage = (aMessage && aMessage[0] != 0);

  if(hasDateTimeInfo)
  {
    fprintf(iFp, "[%s]", aDateTimeInfo);
  }

  if(hasProcessInfo)
  {
    const char * fmt = hasDateTimeInfo? " [%s]": "[%s]";
    fprintf(iFp, fmt, aProcessInfo);
  }

  if(aShowLevel)
  {
    const char * fmt = (hasDateTimeInfo || hasProcessInfo)? " %s:": "%s:";
    fprintf(iFp, fmt, log_t::level_name(aLevel));
  }

  //TODO heh! there is debug info? must add it!

  if(hasMessage)
  {
    const char * fmt =  (hasFullDebugInfo && settings().isWordWrap())? "\n%s"
                        : (hasDateTimeInfo || hasProcessInfo || hasDebugInfo || aShowLevel)? " %s"
                          : "%s";
    fprintf(iFp, fmt, aMessage);
  }

  if(hasDateTimeInfo || hasProcessInfo || aShowLevel || hasMessage)
  {
    fprintf(iFp, "\n");
  }
  fflush(iFp) ;
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
