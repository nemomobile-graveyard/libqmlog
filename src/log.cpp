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

#include <qm/log>


const char *log_t::prg_name = NULL;
log_t *log_t::iLogger = NULL;

log_t& log_t::logger()
{
  if(!iLogger)
  {
    iLogger = new log_t(true);
  }
  return *iLogger;
}

log_t::log_t(bool defaultSetup, const char* name)
{
  log_t::prg_name = name?: iLogger->processName();

  if(defaultSetup)
  {
    //TODO shall it be by default? addLoggerDev(StdErrLoggerDev::getDefault());
    addLoggerDev(StdOutLoggerDev::getDefault()); //TODO switch it off for release
    addLoggerDev(SysLogDev::getDefault());
  }
}

log_t::~log_t()
{
  iLogger = NULL;
}

void log_t::log_init(const char* name)
{
  delete iLogger;
  iLogger = new log_t(false, name);
}

void log_t::addLoggerDev(LoggerDev* aLoggerDev)
{
  assert(aLoggerDev);
  iDevs.push_front(aLoggerDev);
}

void log_t::removeLoggerDev(LoggerDev* aLoggerDev)
{
  assert(aLoggerDev);
  iDevs.remove(aLoggerDev);
}

const char* log_t::processName() const
{
  const int procNameLen = 64;
  static char procName[procNameLen];
  memset(procName, '\0', procNameLen);
  strncpy(procName, "default", procNameLen - 1);

  const int cmdFileNameLen = 256;
  char cmdFileName[cmdFileNameLen];
  memset(cmdFileName, '\0', cmdFileNameLen);
  snprintf(cmdFileName, procNameLen-1, "/proc/%d/cmdline", getpid());

  FILE* cmdLineFile = fopen(cmdFileName, "r");

  if(cmdLineFile)
  {
    const int cmdLen = 256;
    char cmd[cmdLen];
    memset(cmd, '\0', cmdLen);

    fscanf(cmdLineFile, "%s", cmd);
    fclose(cmdLineFile);

    const char* lastSlash = strrchr(cmd, '/');

    if(lastSlash)
    {
      strncpy(procName, ++lastSlash, procNameLen - 1);
    }
  }

  return procName;
}

const char* log_t::prgName()
{
  return prg_name;
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

void log_t::message(int level)
{
  message(level, "\x01");
}

void log_t::message(int level, const char *fmt, ...)
{
  assert(0<=level) ;
  assert(level<=LOG_MAX_LEVEL) ;

  if(fmt[0] == '\x01' && fmt[1] == 0)
    ++fmt;

  va_list args ;
  va_start(args, fmt) ;

  vlog_generic(level, -1, NULL, NULL, fmt, args);

  va_end(args) ;
}

void log_t::message(int level, int line, const char *file, const char *func)
{
  message(level, line, file, func, "\x01");
}

void log_t::message(int level, int line, const char *file, const char *func, const char *fmt, ...)
{
  assert((1<<level) & LOG_MAX_LOCATION) ;
  assert(0<=level) ;
  assert(level<=LOG_MAX_LEVEL) ;

  if(fmt[0] == '\x01' && fmt[1] == 0)
    ++fmt;

  va_list args ;
  va_start(args, fmt) ;

  vlog_generic(level, line, file, func, fmt, args);

  va_end(args) ;
}

void log_t::log_failed_assertion(const char *assertion, int line, const char *file, const char *func)
{
  log_failed_assertion(assertion, line, file, func, "\x01");
}

void log_t::log_failed_assertion(const char *assertion, int line, const char *file, const char *func, const char *fmt, ...)
{
  if(fmt[0] == '\x01' && fmt[1] == 0)
    ++fmt;

  bool message_follows = (fmt[0] != 0);

  message(LOG_LEVEL_INTERNAL, line, file, func, "Assertion failed: %s%s.", 
                            assertion, message_follows? ". Detailed info follows": "");

  if(message_follows)
  {
    va_list args;
    va_start(args, fmt);

    vlog_generic(LOG_LEVEL_INTERNAL, line, file, func, fmt, args);

    va_end(args);
  }
  abort(); do { sleep(1); } while (true);
}

void log_t::vlog_generic(int level, int line, const char *file, const char *func, const char *fmt, va_list args)
{
  for(  std::list<LoggerDev*>::iterator it = iDevs.begin();
        it != iDevs.end(); ++it)
  {
    (*it)->vlogGeneric(level, line, file, func, fmt, args);
  }
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
  return (message_format & ETimeMs) && !(message_format & ETimeMicS);
}

bool LoggerSettings::isTimeMicS() const
{
  return !(message_format & ETimeMs) && (message_format & ETimeMicS);
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
  log_t::logger().removeLoggerDev(this);
}

int LoggerDev::snprintfappend(char * str, int buf_len, int current_len, const char * fmt, ...) const
{
  va_list args ;
  va_start(args, fmt) ;
  int ret = vsnprintfappend(str, buf_len, current_len, fmt, args);
  va_end(args) ;
  return ret;
}

int LoggerDev::vsnprintfappend(char * str, int buf_len, int current_len, const char * fmt, va_list arg) const
{
  return current_len + vsnprintf((char*)(str + current_len), buf_len - current_len - 1, fmt, arg);
}

void LoggerDev::vlogGeneric(int aLevel, int aLine, const char *aFile, const char *aFunc,
                            const char *aFmt, va_list anArgs) const
{
  //TODO asser below shall be in another place
  assert(0<=aLevel) ;
  assert(aLevel<=LOG_MAX_LEVEL) ;
  if(!settings().isLogShown(aLevel))
    return;

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
      dateInfoCurrentLen = snprintfappend(dateInfo, dateInfoLen, dateInfoCurrentLen,
                                                    addSpace? " (%s)": "(%s)", time_tm.tm_zone);
      addSpace = true;
    }

    if(settings().isDate() || settings().isTime())
    {
      const char *fmt = (settings().isDate() && settings().isTime())? "%F %T" : (settings().isDate()? "%F" : "%T" );
      const int time_length = 32;
      char time_buf[time_length+1];
      strftime(time_buf, time_length, fmt, &time_tm);

      dateInfoCurrentLen = snprintfappend(dateInfo, dateInfoLen, dateInfoCurrentLen, addSpace? " %s": "%s", time_buf);

      if(settings().isTime())
      {
        if(settings().isTimeMs())
        {
          dateInfoCurrentLen = snprintfappend(dateInfo, dateInfoLen, dateInfoCurrentLen, ".%03ld", tv.tv_usec/1000);
        }
        else if(settings().isTimeMicS())
        {
          dateInfoCurrentLen = snprintfappend(dateInfo, dateInfoLen, dateInfoCurrentLen, ".%06ld", tv.tv_usec);
        }
      }
      addSpace = true;
    }

    if(settings().isTzSymLink())
    {
      const int tzLinkLen = 1024;
      char tzLink[tzLinkLen];
      memset(tzLink, '\0', tzLinkLen);
      ssize_t res = readlink("/etc/localtime", tzLink, tzLinkLen - 1);

      if(res < 0)
      {
        strncpy(tzLink, strerror(errno), tzLinkLen - 1);
      }
      else
      {
        const char *base = "/usr/share/zoneinfo/";

        if(strncmp(tzLink, base, strlen(base)) == 0)
        {
          strncpy(tzLink, tzLink + strlen(base), tzLinkLen - 1);
        }
      }

      dateInfoCurrentLen = snprintfappend(dateInfo, dateInfoLen, dateInfoCurrentLen, addSpace?  " \'%s\'":"\'%s\'", tzLink);
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
      processInfoCurrentLen = snprintfappend(processInfo, processInfoLen, processInfoCurrentLen, "%s", log_t::prgName());
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

  printLog(aLevel, dateInfo, processInfo, debugInfo, isFullDebugInfo, message);

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
FileLoggerDev::FileLoggerDev(const char *aFileName, int aVerbosityLevel, int aLocationMask, int aMessageFormat)
  : LoggerDev(aVerbosityLevel, aLocationMask, aMessageFormat)
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

void FileLoggerDev::printLog( int aLevel, const char *aDateTimeInfo, const char* aProcessInfo,
                              const char *aDebugInfo, bool aIsFullDebugInfo, const char *aMessage) const
{
  //TODO rework: create format string for output as in SysLogDev
  bool hasPrefix = vlogPrefixes(aDateTimeInfo, aProcessInfo);
  bool hasDebugInfo = vlogDebugInfo(aLevel, aDebugInfo, hasPrefix);
  bool hasMessage = (aMessage && aMessage[0] != 0);

  if(hasMessage)
  {
    //TODO probably better to use (hasDebugInfo && settings().isFunc())
    //instead of aIsFullDebugInfo
    // aIsFullDebugInfo shall be romove everywhere in this case
    if(aIsFullDebugInfo && settings().isWordWrap())
    {
      fprintf(iFp, ":\n");
      hasPrefix = vlogPrefixes(aDateTimeInfo, aProcessInfo);
      fprintf(iFp, " ->");
    }
    else if(hasDebugInfo)
    {
      fprintf(iFp, ":");
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

bool FileLoggerDev::vlogPrefixes(const char *aDateTimeInfo, const char* aProcessInfo) const
{
  bool hasDateTimeInfo = (aDateTimeInfo && aDateTimeInfo[0] != 0);
  bool hasProcessInfo = (aProcessInfo && aProcessInfo[0] != 0);

  if(hasDateTimeInfo)
  {
    fprintf(iFp, "[%s]", aDateTimeInfo);
  }

  if(hasProcessInfo)
  {
    fprintf(iFp, hasDateTimeInfo? " [%s]": "%s:", aProcessInfo);
  }

  return (hasDateTimeInfo | hasProcessInfo);
}

bool FileLoggerDev::vlogDebugInfo(int aLevel, const char *aDebugInfo, bool aPrefixExists) const
{
  bool hasDebugInfo = (aDebugInfo && aDebugInfo[0] != 0); 

  fprintf(iFp, (hasDebugInfo && settings().isFileLine())? (aPrefixExists? " %s at": "%s at"):
                                                          (aPrefixExists? " %s":"%s"), log_t::level_name(aLevel));

  if(hasDebugInfo)
  {
    fprintf(iFp, aPrefixExists? " %s": "%s", aDebugInfo);
  }
  else
  {
    fprintf(iFp, ":");
  }

  return hasDebugInfo;
}



//================== StdErrLoggerDev ===============
//TODO move to separate file
StdErrLoggerDev* StdErrLoggerDev::getDefault()
{
  static std::auto_ptr<StdErrLoggerDev> defaultStdErrLoggerDev(new StdErrLoggerDev);
  return defaultStdErrLoggerDev.get();
}

StdErrLoggerDev::StdErrLoggerDev(int aVerbosityLevel, int aLocationMask, int aMessageFormat)
  : FileLoggerDev(stderr, false, aVerbosityLevel, aLocationMask, aMessageFormat)
{
}

//================== StdOutLoggerDev ===============
//TODO move to separate file
StdOutLoggerDev* StdOutLoggerDev::getDefault()
{
  static std::auto_ptr<StdOutLoggerDev> defaultStdOutLoggerDev(new StdOutLoggerDev);
  return defaultStdOutLoggerDev.get();
}

StdOutLoggerDev::StdOutLoggerDev(int aVerbosityLevel, int aLocationMask, int aMessageFormat)
  : FileLoggerDev(stdout, false, aVerbosityLevel, aLocationMask, aMessageFormat)
{
}

//================== SysLogDev ===============
//TODO move to separate file

SysLogDev* SysLogDev::getDefault()
{
  static std::auto_ptr<SysLogDev> defaultSysLogDev(new SysLogDev);
  return defaultSysLogDev.get();
}

SysLogDev::SysLogDev(int aVerbosityLevel, int aLocationMask, int aMessageFormat)
  : LoggerDev(aVerbosityLevel, aLocationMask, aMessageFormat)
{
  openlog(log_t::prgName(), LOG_PID | LOG_NDELAY, LOG_DAEMON);
}

SysLogDev::~SysLogDev()
{
  closelog();
}

int SysLogDev::syslog_level_id(int level)
{
  static int syslog_names[] =
  {
    LOG_ALERT, LOG_CRIT, LOG_ERR, LOG_WARNING, LOG_INFO, LOG_DEBUG
  } ;
  assert(0<=level) ;
  assert((unsigned)level<sizeof(syslog_names)/sizeof(*syslog_names)) ;
  return syslog_names[level] ;
}

void SysLogDev::printLog( int aLevel, const char* aDateTimeInfo, const char* aProcessInfo,
                          const char* aDebugInfo, bool aIsFullDebugInfo, const char *aMessage) const
{
  const int fmtLen = 32;
  char fmt[fmtLen];
  memset(fmt, '\0', fmtLen);
  int fmtCurrentLen = 0;

  const int secondFmtLen = 32;
  char secondFmt[secondFmtLen];
  memset(secondFmt, '\0', secondFmtLen);
  int secondFmtCurrentLen = 0;


  bool addSpace = false;
  bool hasDateTimeInfo = (aDateTimeInfo && aDateTimeInfo[0] != 0);
  bool hasProcessInfo = (aProcessInfo && aProcessInfo[0] != 0);
  bool hasDebugInfo = (aDebugInfo && aDebugInfo[0] != 0);
  bool hasMessage = (aMessage && aMessage[0] != 0);

  if(hasDateTimeInfo)
  {
    fmtCurrentLen = snprintfappend(fmt, fmtLen, fmtCurrentLen, "[%%s]");
    addSpace = true;
  }
  else
  {
    fmtCurrentLen = snprintfappend(fmt, fmtLen, fmtCurrentLen, "%%s");
  }

  if(hasProcessInfo)
  {
    fmtCurrentLen = snprintfappend(fmt, fmtLen, fmtCurrentLen, addSpace? " [%%s]": "%%s:");
    addSpace = true;
  }
  else
  {
    fmtCurrentLen = snprintfappend(fmt, fmtLen, fmtCurrentLen, "%%s");
  }

  strncpy(secondFmt, fmt, secondFmtLen);
  secondFmtCurrentLen = fmtCurrentLen;

  if(hasDebugInfo)
  {
    //TODO is it possible to move "at" into LoggerDev::logGeneric() now?
    fmtCurrentLen = snprintfappend(fmt, fmtLen, fmtCurrentLen, 
                                        settings().isFileLine()?  (addSpace? " %%s at %%s": "%%s at %%s"):
                                                                  (addSpace? " %%s %%s": "%%s %%s"));
    fmtCurrentLen = snprintfappend( fmt, fmtLen, fmtCurrentLen, 
                                    hasMessage? ((aIsFullDebugInfo && settings().isWordWrap())? ":": ": %%s"): ".");
  }
  else
  {
    fmtCurrentLen = snprintfappend(fmt, fmtLen, fmtCurrentLen, "%%s%%s");
    fmtCurrentLen = snprintfappend(fmt, fmtLen, fmtCurrentLen, (hasMessage && addSpace)? " %%s":"%%s");
  }

  if(!(hasDateTimeInfo || hasProcessInfo || hasDebugInfo || hasMessage))
  {
    syslog(LOG_DAEMON | syslog_level_id(aLevel), "%s", log_t::level_name(aLevel));
  }
  else if(aIsFullDebugInfo && settings().isWordWrap())
  {
    syslog(LOG_DAEMON | syslog_level_id(aLevel), fmt,  aDateTimeInfo, aProcessInfo,
                                                            log_t::level_name(aLevel), aDebugInfo);
    if(hasMessage)
    {
      secondFmtCurrentLen = snprintfappend(secondFmt, secondFmtLen, secondFmtCurrentLen, "-> %%s");
      syslog(LOG_DAEMON | syslog_level_id(aLevel), secondFmt, aDateTimeInfo, aProcessInfo, aMessage);
    }
  }
  else
  {
    syslog(LOG_DAEMON | syslog_level_id(aLevel), fmt,  aDateTimeInfo, aProcessInfo,
                                 hasDebugInfo? log_t::level_name(aLevel): "", aDebugInfo, aMessage);
  }
}
