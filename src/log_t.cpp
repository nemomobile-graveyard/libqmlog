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
#include <cstring>
#include <unistd.h>
#include <algorithm>
#include <memory>

#include "log_t.h"
#include "log-declarations.h"
#include "LoggerDev.h"
#include "SmartBuffer.h"
#include "StdErrLoggerDev.h"
#include "SysLogDev.h"


class LoggerRemover
{
public:
  static void create()
  {
    if(!log_t::iLogger)
    {
      static std::auto_ptr<LoggerRemover> remover(new LoggerRemover);
    }
  }

  LoggerRemover(){}

  ~LoggerRemover()
  {
    delete log_t::iLogger;
  }
};

const char *log_t::prg_name = NULL;
log_t *log_t::iLogger = NULL;

log_t& log_t::logger()
{
  if(!iLogger)
  {
    LoggerRemover::create();
    iLogger = new log_t(true, true);
  }
  return *iLogger;
}

log_t::log_t(bool defaultSetup, bool aRestoreDefaultDevs, const char* name)
  : iRestoreDefaultDevs(aRestoreDefaultDevs)
{
  log_t::prg_name = name?: iLogger->processName();

  if(defaultSetup)
  {
    createDefaultDevs();
  }
}

log_t::~log_t()
{
  for(  std::list<LoggerDev*>::iterator it = iDevs.begin();
        it != iDevs.end(); ++it)
  {
    if((*it) && (*it)->isPermanent())
    {
      delete (*it);
    }
  }

  iLogger = NULL;
}

void log_t::log_init(const char* name)
{
  bool notFirstInstance = (iLogger);
  LoggerRemover::create();
  delete iLogger;
  iLogger = new log_t(false, notFirstInstance, name);
}

void log_t::setRestoreDefaultDevs()
{
  iRestoreDefaultDevs = true;
}

void log_t::clearRestoreDefaultDevs()
{
  iRestoreDefaultDevs = false;
}

void log_t::createDefaultDevs()
{
  addPermanentLoggerDev(new StdErrLoggerDev);
  addPermanentLoggerDev(new SysLogDev);
}

void log_t::addPermanentLoggerDev(LoggerDev* aLoggerDev)
{
  assert(aLoggerDev);

  aLoggerDev->setPermanent(true);
  addLoggerDev(aLoggerDev);
}

void log_t::addLoggerDev(LoggerDev* aLoggerDev)
{
  assert(aLoggerDev);

  for(  std::list<LoggerSettings*>::reverse_iterator it = iTempSettings.rbegin();
        it != iTempSettings.rend(); ++it)
  {
    aLoggerDev->setTempSettings(**it);
  }

  iDevs.push_front(aLoggerDev);
}

void log_t::removeLoggerDev(LoggerDev* aLoggerDev)
{
  assert(aLoggerDev);
  assert(!aLoggerDev->isPermanent());

  for(  std::list<LoggerSettings*>::reverse_iterator it = iTempSettings.rbegin();
        it != iTempSettings.rend(); ++it)
  {
    aLoggerDev->removeTempSettings(**it);
  }

  iDevs.remove(aLoggerDev);

  if(iRestoreDefaultDevs && iDevs.size() == 0)
  {
    createDefaultDevs();
  }
}

void log_t::setTempSettings(LoggerSettings* aSettings)
{
  assert(aSettings);

  for(  std::list<LoggerDev*>::iterator it = iDevs.begin();
        it != iDevs.end(); ++it)
  {
    (*it)->setTempSettings(*aSettings);
  }

  iTempSettings.push_front(aSettings);
}

void log_t::removeTempSettings(LoggerSettings* aSettings)
{
  assert(aSettings);
  iTempSettings.remove(aSettings);
}

const char* log_t::processName() const
{
  const int procNameLen = 64;
  static SmartBuffer<procNameLen> procName;
  procName.print("default");

  const int cmdFileNameLen = 256;
  SmartBuffer<cmdFileNameLen> cmdFileName;
  cmdFileName.print("/proc/%d/cmdline", getpid());

  FILE* cmdLineFile = fopen(cmdFileName(), "r");

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
      procName.print(++lastSlash);
    }
  }

  return procName();
}

const char* log_t::prgName()
{
  return prg_name;
}

const char *log_t::level_name(int level)
{
  static const char *names[] =
  {
    "INTERNAL ERROR", "CRITICAL ERROR", "ERROR", "WARNING", "NOTICE", "INFO", "DEBUG"
  } ;
  assert(0<=level) ;
  assert((unsigned)level<sizeof(names)/sizeof(*names)) ;
  return names[level] ;
}

const struct timeval& log_t::tv()
{
  assert(iLogger);
  return iLogger->iTv;
}

const struct tm& log_t::tm()
{
  assert(iLogger);
  return iLogger->iTm;
}

const struct timespec& log_t::ts()
{
  assert(iLogger);
  return iLogger->iTs;
}

void log_t::message(int level)
{
  message(level, "\x01");
}

void log_t::message(int level, const char *fmt, ...)
{
  assert(0<=level) ;
  assert(level<=LOG_MAX_LEVEL) ;

  if(fmt[0] == '\x01' && fmt[1] == '\0')
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
  updateTime();

  for(  std::list<LoggerDev*>::iterator it = iDevs.begin();
        it != iDevs.end(); ++it)
  {
    (*it)->vlogGeneric(level, line, file, func, fmt, args);
  }
}

void log_t::updateTime()
{
  clock_gettime(CLOCK_MONOTONIC, &iTs);
  gettimeofday(&iTv, NULL) ;
  time_t t = iTv.tv_sec ;
  localtime_r(&t, &iTm) ;
}
