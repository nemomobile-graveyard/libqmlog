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

#include "FileLoggerDev.h"
#include "log_t.h"
#include "log-interface.h"


FileLoggerDev::FileLoggerDev(const char *aFileName, int aVerbosityLevel, int aLocationMask, int aMessageFormat)
  : LoggerDev(aVerbosityLevel, aLocationMask, aMessageFormat)
  , iFp(NULL)
  , iIsFpOwner(false)
{
  assert(aFileName);
  iFp = fopen(aFileName, "aw");
  if(!iFp) log_warning("Failed to create file %s for file logger. Logger disabled.", aFileName);
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
  if(!iFp) log_warning("Null file descriptor is received by file logger protected constructor. Logger disabled.");
}

void FileLoggerDev::printLog( int aLevel, const char *aDateTimeInfo, const char* aProcessInfo,
                              const char *aDebugInfo, bool aIsLongDebugInfo, const char *aMessage) const
{
  if(!iFp) return;
  //TODO rework: create format string for output as in SysLogDev
  bool hasPrefix = vlogPrefixes(aDateTimeInfo, aProcessInfo);
  bool hasDebugInfo = vlogDebugInfo(aLevel, aDebugInfo, hasPrefix);
  bool hasMessage = (aMessage && aMessage[0] != 0);

  if(hasMessage)
  {
    if(aIsLongDebugInfo && settings().isWordWrap())
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
  if(!iFp) return false;

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
  if(!iFp) return false;

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
