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
#ifndef MAEMO_QMLOG_SYSLOG_DEV_H
#define MAEMO_QMLOG_SYSLOG_DEV_H

#include <qm/LoggerDev.h>


class SysLogDev : public LoggerDev
{
public:
  enum
  {
      DefaultLevel = LOG_LEVEL_WARNING
    , DefaultLocation = LOG_MAX_LOCATION
    , DefaultFormat =   LoggerSettings::EMessage
  };

public:
  SysLogDev(int aVerbosityLevel = DefaultLevel,
            int aLocationMask = DefaultLocation,
            int aMessageFormat = DefaultFormat);
  ~SysLogDev();

protected:
  virtual void printLog(int aLevel, const char *aDateTimeInfo, const char* aProcessInfo,
                        const char *aDebugInfo, bool aIsFullDebugInfo, const char *aMessage) const;

private:
  static int syslog_level_id(int level);
};

#endif
