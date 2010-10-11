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
#ifndef MAEMO_QMLOG_LOG_T_H
#define MAEMO_QMLOG_LOG_T_H

#include <sys/time.h>
#include <ctime>
#include <list>
#include <cstdarg>


class LoggerDev;

struct log_t
{
  static log_t& logger();
  static void log_init(const char *name = NULL);

  static const char* prgName();
  static const char* level_name(int level);
  static const struct timeval& tv();
  static const struct tm& tm();
  static const struct timespec& ts();

  void addLoggerDev(LoggerDev* aLoggerDev);
  void removeLoggerDev(LoggerDev* aLoggerDev);

  void message(int level) ;
  void message(int level, const char *fmt, ...) __attribute__((format(printf,3,4)));
  void message(int level, int line, const char *file, const char *func);
  void message(int level, int line, const char *file, const char *func, const char *fmt, ...)
                                                            __attribute__((format(printf,6,7)));

  void log_failed_assertion(const char *assertion, int line, const char *file, const char *func);
  void log_failed_assertion(const char *assertion, int line, const char *file, const char *func, const char *fmt, ...)
                                                                                    __attribute__((format(printf,6,7)));
private:
  log_t(bool defaultSetup, const char* name = NULL);
  ~log_t();
  void vlog_generic(int level, int line, const char *file, const char *func, const char *fmt, va_list args);
  const char* processName() const;
  void updateTime();

private:
  static const char *prg_name;
  static log_t * iLogger;
  std::list<LoggerDev*> iDevs;
  struct timeval iTv;
  struct tm iTm;
  struct timespec iTs;

  friend class LoggerRemover;
};

#endif
