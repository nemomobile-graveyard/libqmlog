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
#ifndef MAEMO_QMLOG_LOG_INTERFACE_H
#define MAEMO_QMLOG_LOG_INTERFACE_H

#include <memory>
#include <qm/log-declarations.h>


#if LOG_MAX_LEVEL > LOG_LEVEL_NO_LOGS
  #define INIT_LOGGER(...) \
    log_t::log_init(__VA_ARGS__)

  #define RESTORE_DEFAULT_DEVS() \
    log_t::logger().setRestoreDefaultDevs()

  #define DO_NOT_RESTORE_DEFAULT_DEVS() \
    log_t::logger().clearRestoreDefaultDevs()

  #define ADD_PERMANENT_SYSLOG(...) \
    log_t::logger().addPermanentLoggerDev(new SysLogDev(__VA_ARGS__))

  #define ADD_SYSLOG(...) \
    std::auto_ptr<SysLogDev> sys_log_ptr(new SysLogDev(__VA_ARGS__)); \
    log_t::logger().addLoggerDev(sys_log_ptr.get())

  #define ADD_DEBUG_SYSLOG() \
    ADD_SYSLOG(LOG_LEVEL_DEBUG, LOG_MAX_LOCATION, \
              SysLogDev::DefaultFormat | LoggerSettings::EDebugInfo| LoggerSettings::EWordWrap)

  #define ADD_PERMANENT_STDERR_LOG(...) \
    log_t::logger().addPermanentLoggerDev(new StdErrLoggerDev(__VA_ARGS__))

  #define ADD_STDERR_LOG(...) \
    std::auto_ptr<StdErrLoggerDev> stderr_log_ptr(new StdErrLoggerDev(__VA_ARGS__)); \
    log_t::logger().addLoggerDev(stderr_log_ptr.get())

  #define ADD_DEBUG_STDERR_LOG() \
    ADD_STDERR_LOG(LOG_LEVEL_DEBUG, LOG_MAX_LOCATION, \
              StdErrLoggerDev::DefaultFormat | LoggerSettings::EDebugInfo| LoggerSettings::EWordWrap)

  #define ADD_PERMANENT_STDOUT_LOG(...) \
    log_t::logger().addPermanentLoggerDev(new StdOutLoggerDev(__VA_ARGS__))

  #define ADD_STDOUT_LOG(...) \
    std::auto_ptr<StdOutLoggerDev> stdout_log_ptr(new StdOutLoggerDev(__VA_ARGS__)); \
    log_t::logger().addLoggerDev(stdout_log_ptr.get())

  #define ADD_PERMANENT_FILE_LOG(NAME, ...) \
    log_t::logger().addPermanentLoggerDev(new FileLoggerDev(NAME, ## __VA_ARGS__))

  #define ADD_FILE_LOG(NAME, ...) \
    std::auto_ptr<FileLoggerDev> file_log_ptr(new FileLoggerDev(NAME, ## __VA_ARGS__)); \
    log_t::logger().addLoggerDev(file_log_ptr.get())

  #define SET_TEMP_LOG_SETTINGS(VERBOSITY, LOCATION_MASK, FORMAT) \
    std::auto_ptr<LoggerSettings> settings_log_ptr(new LoggerSettings(VERBOSITY, LOCATION_MASK, FORMAT)); \
    log_t::logger().setTempSettings(settings_log_ptr.get())

  #define SET_TEMP_LOG_SETTINGS_MAX_DEBUG() \
    SET_TEMP_LOG_SETTINGS(LOG_LEVEL_DEBUG, LOG_MAX_LOCATION, \
        LoggerSettings::EMTimerNs     | LoggerSettings::ETzAbbr     | LoggerSettings::EDate \
      | LoggerSettings::ETimeMicS     | LoggerSettings::ETzSymLink \
      | LoggerSettings::EProcessInfo  | LoggerSettings::EDebugInfo  | LoggerSettings::EMessage \
      | LoggerSettings::EWordWrap)

  #define SET_TEMP_LOG_SETTINGS_SUPPRESS_LOGGING() \
    SET_TEMP_LOG_SETTINGS(LOG_LEVEL_INTERNAL, LOG_MAX_LOCATION, \
        LoggerSettings::EMTimerNs     | LoggerSettings::ETzAbbr     | LoggerSettings::EDate \
      | LoggerSettings::ETimeMicS     | LoggerSettings::ETzSymLink \
      | LoggerSettings::EProcessInfo  | LoggerSettings::EDebugInfo  | LoggerSettings::EMessage \
      | LoggerSettings::EWordWrap)

  #define LOG_LOCATION __LINE__,__FILE__,__PRETTY_FUNCTION__
#else
  #define INIT_LOGGER(...)
  #define RESTORE_DEFAULT_DEVS()
  #define DO_NOT_RESTORE_DEFAULT_DEVS()
  #define ADD_PERMANENT_SYSLOG(...)
  #define ADD_SYSLOG(...)
  #define ADD_DEBUG_SYSLOG()
  #define ADD_PERMANENT_STDERR_LOG(...)
  #define ADD_STDERR_LOG(...)
  #define ADD_DEBUG_STDERR_LOG()
  #define ADD_PERMANENT_STDOUT_LOG(...)
  #define ADD_STDOUT_LOG(...)
  #define ADD_PERMANENT_FILE_LOG(NAME, ...)
  #define ADD_FILE_LOG(NAME, ...)
  #define SET_TEMP_LOG_SETTINGS(VERBOSITY, LOCATION_MASK, FORMAT)
  #define SET_TEMP_LOG_SETTINGS_MAX_DEBUG()
  #define SET_TEMP_LOG_SETTINGS_SUPPRESS_LOGGING()
  #define LOG_LOCATION
#endif


#if LOG_MAX_LEVEL >= LOG_LEVEL_CRITICAL
# if (LOG_MAX_LOCATION) & (1<<LOG_LEVEL_CRITICAL)
#  define log_critical(...) log_t::logger().message(LOG_LEVEL_CRITICAL, LOG_LOCATION, ## __VA_ARGS__)
# else
#  define log_critical(...) log_t::logger().message(LOG_LEVEL_CRITICAL, ## __VA_ARGS__)
# endif
#else
# define log_critical(...) (void)(0)
#endif

#if LOG_MAX_LEVEL >= LOG_LEVEL_ERROR
# if (LOG_MAX_LOCATION) & (1<<LOG_LEVEL_ERROR)
#  define log_error(...) log_t::logger().message(LOG_LEVEL_ERROR, LOG_LOCATION, ## __VA_ARGS__)
# else
#  define log_error(...) log_t::logger().message(LOG_LEVEL_ERROR, ## __VA_ARGS__)
# endif
#else
# define log_error(...) (void)(0)
#endif

#if LOG_MAX_LEVEL >= LOG_LEVEL_WARNING
# if (LOG_MAX_LOCATION) & (1<<LOG_LEVEL_WARNING)
#  define log_warning(...) log_t::logger().message(LOG_LEVEL_WARNING, LOG_LOCATION, ## __VA_ARGS__)
# else
#  define log_warning(...) log_t::logger().message(LOG_LEVEL_WARNING, ## __VA_ARGS__)
# endif
#else
# define log_warning(...) (void)(0)
#endif

#if LOG_MAX_LEVEL >= LOG_LEVEL_NOTICE
# if (LOG_MAX_LOCATION) & (1<<LOG_LEVEL_NOTICE)
#  define log_notice(...) log_t::logger().message(LOG_LEVEL_NOTICE, LOG_LOCATION, ## __VA_ARGS__)
# else
#  define log_notice(...) log_t::logger().message(LOG_LEVEL_NOTICE, ## __VA_ARGS__)
# endif
#else
# define log_notice(...) (void)(0)
#endif


#if LOG_MAX_LEVEL >= LOG_LEVEL_INFO
# if (LOG_MAX_LOCATION) & (1<<LOG_LEVEL_INFO)
#  define log_info(...) log_t::logger().message(LOG_LEVEL_INFO, LOG_LOCATION, ## __VA_ARGS__)
# else
#  define log_info(...) log_t::logger().message(LOG_LEVEL_INFO, ## __VA_ARGS__)
# endif
#else
# define log_info(...) (void)(0)
#endif

#if LOG_MAX_LEVEL >= LOG_LEVEL_DEBUG
# if (LOG_MAX_LOCATION) & (1<<LOG_LEVEL_DEBUG)
#  define log_debug(...) log_t::logger().message(LOG_LEVEL_DEBUG, LOG_LOCATION, ## __VA_ARGS__)
# else
#  define log_debug(...) log_t::logger().message(LOG_LEVEL_DEBUG, ## __VA_ARGS__)
# endif
#else
# define log_debug(...) (void)(0)
#endif

#if LOG_ASSERTION
# define log_assert(x, ...) do { if(!(x)) log_t::logger().log_failed_assertion(#x, LOG_LOCATION, ## __VA_ARGS__) ; } while(0)
#else
# define log_assert(...) (void)(0)
#endif

#endif
