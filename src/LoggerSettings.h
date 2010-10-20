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
#ifndef MAEMO_QMLOG_LOGGER_SETTINGS_H
#define MAEMO_QMLOG_LOGGER_SETTINGS_H

#include <map>

#include <qm/log-declarations.h>


#define LOG_OUTPUT_WORD_WRAP       0
#define LOG_OUTPUT_MESSAGE         1
#define LOG_OUTPUT_FILE_LINE       2
#define LOG_OUTPUT_FUNC            3
#define LOG_OUTPUT_PID             4
#define LOG_OUTPUT_NAME            5
#define LOG_OUTPUT_TZ_SYM_LINK     6
#define LOG_OUTPUT_TIME_LOW_BIT    7
#define LOG_OUTPUT_TIME_HIGH_BIT   8
#define LOG_OUTPUT_DATE            9
#define LOG_OUTPUT_TZ_ABBR         10
#define LOG_OUTPUT_MTIMER_LOW_BIT  11
#define LOG_OUTPUT_MTIMER_HIGH_BIT 12
// TODO bit #13: output tm.tm_gmtoff like
// [108.151: 2010-10-13 17:02:52.831 'Europe/Helsinki' (EEST) GMT+3]
// [108.151: 2010-10-13 19:32:52.831 'Asia/Kolkata' (IST) GMT+5:30]
// [108.151: 2010-10-13 14:02:52.831 'Europe/London' (BST) GMT+0)]
// or like this ?
// [108.151: 2010-10-13 17:02:52.831 'Europe/Helsinki' (EEST, GMT+3)]
// [108.151: 2010-10-13 19:32:52.831 'Asia/Kolkata' (IST, GMT+5:30)]
// [108.151: 2010-10-13 14:02:52.831 'Europe/London' (BST, GMT+0)]
// or like this ?
// [108.151 2010-10-13 17:02:52.831 (EEST, GMT+3) 'Europe/Helsinki']
// [108.151 2010-10-13 19:32:52.831 (IST, GMT+5:30) 'Asia/Kolkata']
// [108.151 2010-10-13 14:02:52.831 (BST, GMT+0) 'Europe/London']
// or like this ?
// [108.151 (EEST) 2010-10-13 17:02:52.831 GMT+3 'Europe/Helsinki']
// [108.151 (IST) 2010-10-13 19:32:52.831 GMT+5:30 'Asia/Kolkata']
// [108.151 (BST) 2010-10-13 14:02:52.831 GMT+0 'Europe/London']
// or like this ?
// [108.151 'Europe/Helsinki' 2010-10-13 17:02:52.831 (EEST, GMT+3)]
// [108.151 'Asia/Kolkata' 2010-10-13 19:32:52.831 (IST, GMT+5:30)]
// [108.151 'Europe/London' 2010-10-13 14:02:52.831 (BST, GMT+0)]
// or like this ?
// [108.151 (EEST,GMT+3) 2010-10-13 17:02:52.831 'Europe/Helsinki'] !!!
// [108.151 (IST,GMT+5:30) 2010-10-13 19:32:52.831 'Asia/Kolkata'] !!!
// [108.151 (BST,GMT+0) 2010-10-13 14:02:52.831 'Europe/London'] !!!
// or like this ?
// [108.151 (GMT+3,EEST) 2010-10-13 17:02:52.831 'Europe/Helsinki']
// [108.151 (GMT+5:30,IST) 2010-10-13 19:32:52.831 'Asia/Kolkata']
// [108.151 (GMT+0,BST) 2010-10-13 14:02:52.831 'Europe/London']

#define LOG_OUTPUT_MAX_BIT         13


class LoggerSettings
{
public:
  enum
  {
      EMTimerMs = LOG_BIT_MASK(LOG_OUTPUT_MTIMER_HIGH_BIT)
    , EMTimerNs = LOG_BIT_MASK(LOG_OUTPUT_MTIMER_LOW_BIT)
    , EMTimer = EMTimerMs | EMTimerNs //both ms and ns bits means no ms and ns
    , ETzAbbr = LOG_BIT_MASK(LOG_OUTPUT_TZ_ABBR)
    , EDate = LOG_BIT_MASK(LOG_OUTPUT_DATE)
    , ETimeMs = LOG_BIT_MASK(LOG_OUTPUT_TIME_HIGH_BIT)
    , ETimeMicS = LOG_BIT_MASK(LOG_OUTPUT_TIME_LOW_BIT)
    , ETime = ETimeMs | ETimeMicS //both ms and MicS bits means no ms and MicS
    , ETzSymLink = LOG_BIT_MASK(LOG_OUTPUT_TZ_SYM_LINK)
    , EDateTimeInfo = EMTimer | ETzAbbr | EDate | ETime | ETzSymLink

    , EName = LOG_BIT_MASK(LOG_OUTPUT_NAME)
    , EPid = LOG_BIT_MASK(LOG_OUTPUT_PID)
    , EProcessInfo = EName | EPid

    , EFunc = LOG_BIT_MASK(LOG_OUTPUT_FUNC)
    , EFileLine = LOG_BIT_MASK(LOG_OUTPUT_FILE_LINE)
    , EDebugInfo = EFunc | EFileLine

    , EMessage = LOG_BIT_MASK(LOG_OUTPUT_MESSAGE)

    , EWordWrap = LOG_BIT_MASK(LOG_OUTPUT_WORD_WRAP)
  };

public:
  LoggerSettings(int new_verbosity_level = 0, int new_location_mask = 0, int new_message_format = 0);
  LoggerSettings(const LoggerSettings& aLoggerSettings);
  virtual ~LoggerSettings();

  const LoggerSettings& operator= (const LoggerSettings& aLoggerSettings);

  void restore();
  void addToRestoreList(LoggerSettings* aSettings);
  void removeFromRestoreList(LoggerSettings* aSettings);

  bool isLogShown(int aLevel) const;
  bool isLocationShown(int aLevel) const;

  bool isMTimerMs() const;
  bool isMTimerNs() const;
  bool isMTimer() const;
  bool isTzAbbr() const;
  bool isDate() const;
  bool isTimeMs() const;
  bool isTimeMicS() const;
  bool isTime() const;
  bool isTzSymLink() const;
  bool isDateTimeInfo() const;

  bool isName() const;
  bool isPid() const;
  bool isProcessInfo() const;

  bool isFunc() const;
  bool isFileLine() const;
  bool isDebugInfo() const;

  bool isMessage() const;

  bool isWordWrap() const;

private:
  typedef std::map<LoggerSettings*, LoggerSettings> RestoreList;

private:
  int verbosity_level;
  int location_mask;
  int message_format;
  RestoreList iRestoreList;
};

#endif
