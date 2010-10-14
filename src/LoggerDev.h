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
#ifndef MAEMO_QMLOG_LOGGGER_DEV_H
#define MAEMO_QMLOG_LOGGGER_DEV_H

#include <cstdarg>

#include <qm/LoggerSettings.h>


class LoggerDev
{
public:
  virtual ~LoggerDev();

  bool isPermanent() const;
  void setPermanent(bool aIsPermanent);

  void vlogGeneric( int aLevel, int aLine, const char *aFile, const char *aFunc,
                    const char *aFmt, va_list anArgs) const;
  void setTempSettings(LoggerSettings& aSettings);
  void removeTempSettings(LoggerSettings& aSettings);

protected:
  LoggerDev(int aVerbosityLevel, int aLocationMask, int aMessageFormat);
  virtual void printLog(int aLevel, const char *aDateTimeInfo, const char* aProcessInfo,
                        const char *aDebugInfo, bool aIsLongDebugInfo, const char *aMessage) const = 0;
  const LoggerSettings& settings() const;

private:
  LoggerSettings iSettings;
  bool iIsPermanent;
};

#endif
