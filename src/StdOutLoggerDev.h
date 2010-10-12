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
#ifndef MAEMO_QMLOG_STDOUT_LOGGER_DEV_H
#define MAEMO_QMLOG_STDOUT_LOGGER_DEV_H

#include <qm/FileLoggerDev.h>


class StdOutLoggerDev : public FileLoggerDev
{
public:
  enum
  {
      DefaultLevel = LOG_LEVEL_DEBUG
    , DefaultLocation = LOG_MAX_LOCATION
    , DefaultFormat =   LoggerSettings::EProcessInfo | LoggerSettings::EDebugInfo
                      | LoggerSettings::EMessage | LoggerSettings::EWordWrap
  };

public:
  StdOutLoggerDev(int aVerbosityLevel = DefaultLevel,
                  int aLocationMask = DefaultLocation,
                  int aMessageFormat = DefaultFormat);
};

#endif
