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

#include <cassert>
#include <syslog.h>
#include <memory>

#include "SysLogDev.h"
#include "log_t.h"
#include "SmartBuffer.h"


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
  SmartBuffer<fmtLen> fmt;

  const int secondFmtLen = 32;
  SmartBuffer<secondFmtLen> secondFmt;


  bool addSpace = false;
  bool hasDateTimeInfo = (aDateTimeInfo && aDateTimeInfo[0] != 0);
  bool hasProcessInfo = (aProcessInfo && aProcessInfo[0] != 0);
  bool hasDebugInfo = (aDebugInfo && aDebugInfo[0] != 0);
  bool hasMessage = (aMessage && aMessage[0] != 0);

  if(hasDateTimeInfo)
  {
    fmt.append("[%%s]");
    addSpace = true;
  }
  else
  {
    fmt.append("%%s");
  }

  if(hasProcessInfo)
  {
    fmt.append(addSpace? " [%%s]": "%%s:");
    addSpace = true;
  }
  else
  {
    fmt.append("%%s");
  }

  secondFmt.copy(fmt);

  if(hasDebugInfo)
  {
    //TODO is it possible to move "at" into LoggerDev::logGeneric() now?
    fmt.append(settings().isFileLine()?  (addSpace? " %%s at %%s": "%%s at %%s"): (addSpace? " %%s %%s": "%%s %%s"));
    fmt.append(hasMessage? ((aIsFullDebugInfo && settings().isWordWrap())? ":": ": %%s"): ".");
  }
  else
  {
    fmt.append("%%s%%s");
    fmt.append((hasMessage && addSpace)? " %%s":"%%s");
  }

  if(!(hasDateTimeInfo || hasProcessInfo || hasDebugInfo || hasMessage))
  {
    syslog(LOG_DAEMON | syslog_level_id(aLevel), "%s", log_t::level_name(aLevel));
  }
  else if(aIsFullDebugInfo && settings().isWordWrap())
  {
    syslog(LOG_DAEMON | syslog_level_id(aLevel), fmt(),  aDateTimeInfo, aProcessInfo,
                                                            log_t::level_name(aLevel), aDebugInfo);
    if(hasMessage)
    {
      secondFmt.append("-> %%s");
      syslog(LOG_DAEMON | syslog_level_id(aLevel), secondFmt(), aDateTimeInfo, aProcessInfo, aMessage);
    }
  }
  else
  {
    syslog(LOG_DAEMON | syslog_level_id(aLevel), fmt(),  aDateTimeInfo, aProcessInfo,
                                 hasDebugInfo? log_t::level_name(aLevel): "", aDebugInfo, aMessage);
  }
}
