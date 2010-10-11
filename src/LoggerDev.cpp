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
#include <cstring>
#include <cassert>
#include <sys/types.h>
#include <cerrno>
#include <unistd.h>

#include "LoggerDev.h"
#include "log_t.h"
#include "SmartBuffer.h"


LoggerDev::LoggerDev(int aVerbosityLevel, int aLocationMask, int aMessageFormat)
  : iSettings(aVerbosityLevel, aLocationMask, aMessageFormat)
{
}

LoggerDev::~LoggerDev()
{
  log_t::logger().removeLoggerDev(this);
}

//TODO split to shorter functions
void LoggerDev::vlogGeneric(int aLevel, int aLine, const char *aFile, const char *aFunc,
                            const char *aFmt, va_list anArgs) const
{
  assert(0<=aLevel) ;
  assert(aLevel<=LOG_MAX_LEVEL) ;
  if(!settings().isLogShown(aLevel))
    return;

  const int dateInfoLen = 256;
  SmartBuffer<dateInfoLen> dateInfo;
  bool addSpace = false;

  if(settings().isDateTimeInfo())
  {
    if(settings().isMTimer())
    {
      dateInfo.append("%ld", log_t::ts().tv_sec);

      if(settings().isMTimerMs())
      {
        dateInfo.append(".%03ld", log_t::ts().tv_nsec/1000000);
      }
      else if(settings().isMTimerNs())
      {
        dateInfo.append(".%09ld", log_t::ts().tv_nsec);
      }
      addSpace = true;
    }

    if(settings().isTzAbbr())
    {
      dateInfo.append(addSpace? " (%s)": "(%s)", log_t::tm().tm_zone);
      addSpace = true;
    }

    if(settings().isDate() || settings().isTime())
    {
      const char *fmt = (settings().isDate() && settings().isTime())? (addSpace? " %F %T":"%F %T") : 
                        (settings().isDate()? (addSpace? " %F": "%F"): (addSpace? " %T": "%T"));
      dateInfo.appendTm(fmt, log_t::tm());

      if(settings().isTime())
      {
        if(settings().isTimeMs())
        {
          dateInfo.append(".%03ld", log_t::tv().tv_usec/1000);
        }
        else if(settings().isTimeMicS())
        {
          dateInfo.append(".%06ld", log_t::tv().tv_usec);
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

      dateInfo.append(addSpace?  " \'%s\'":"\'%s\'", tzLink);
    }

    addSpace = false;
  }

  const int processInfoLen = 64;
  SmartBuffer<processInfoLen> processInfo;

  if(settings().isProcessInfo())
  {
    if(settings().isName())
    {
      processInfo.append("%s", log_t::prgName());
    }

    if(settings().isPid())
    {
      processInfo.append("(%d)", getpid());
    }
  }

  const int debugInfoLen = 1024;
  SmartBuffer<debugInfoLen> debugInfo;
  bool isFullDebugInfo = false;

  if(settings().isDebugInfo() && settings().isLocationShown(aLevel))
  {
    if(settings().isFileLine() && aLine > 0 && aFile)
    {
      debugInfo.append("%s:%d", aFile, aLine);
      addSpace = true;
    }

    if(settings().isFunc() && aFunc)
    {
      debugInfo.append(addSpace? " in %s": "in %s", aFunc);
    }

    isFullDebugInfo = (aLine > 0 && aFile && aFunc) && (settings().isFileLine() && settings().isFunc());
  }

  const int messageLen = 1024;
  SmartBuffer<messageLen> message;
  message.vprint(aFmt, anArgs);

  printLog(aLevel, dateInfo(), processInfo(), debugInfo(), isFullDebugInfo, message());

}

void LoggerDev::setTempSettings(LoggerSettings& aSettings)
{
  aSettings.addToRestoreList(&iSettings);
  iSettings = aSettings;
}

void LoggerDev::removeTempSettings(LoggerSettings& aSettings)
{
  aSettings.removeFromRestoreList(&iSettings);
}

const LoggerSettings& LoggerDev::settings() const
{
  return iSettings;
}
