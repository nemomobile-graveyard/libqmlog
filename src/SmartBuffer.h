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
#ifndef MAEMO_QMLOG_SMART_BUFFER_H
#define MAEMO_QMLOG_SMART_BUFFER_H

#include <cstdio>
#include <cstdarg>
#include <sys/time.h>
#include <ctime>
#include <cstdarg>
#include <cstring>


template <int LEN>
class SmartBuffer
{
public:
  SmartBuffer() : iCurrentLen(0)
  {
    memset(iBuffer, '\0', LEN);
  }

  int length() const
  {
    return (int)LEN;
  }

  bool isFull() const
  {
    return (iCurrentLen >= (LEN - 1));
  }

  int emptySpace() const
  {
    return (LEN - iCurrentLen - 1);
  }

  const char* operator()() const
  {
    return iBuffer;
  }

  void clear()
  {
    if(iCurrentLen)
    {
      iCurrentLen = 0;
      memset(iBuffer, '\0', LEN);
    }
  }

  bool print(const char * aFmt, ...) __attribute__((format(printf, 2, 3)))
  {
    va_list args ;
    va_start(args, aFmt) ;
    bool ret = vprint(aFmt, args);
    va_end(args) ;
    return ret;
  }

  bool vprint(const char * aFmt, va_list anArgs)
  {
    clear();
    return vappend(aFmt, anArgs);
  }

  bool append(const char * aFmt, ...) __attribute__((format(printf, 2, 3)))
  {
    va_list args ;
    va_start(args, aFmt) ;
    bool ret = vappend(aFmt, args);
    va_end(args) ;
    return ret;
  }

  bool vappend(const char * aFmt, va_list anArgs)
  {
    if(isFull()) return false;

    int written = vsnprintf(ptrToEnd(), emptySpace(), aFmt, anArgs);
    updateCurrentLen(written);
    return (written > 0);
  }

  bool appendTm(const char * aFmt, const struct tm& aTm)
  {
    if(isFull()) return false;

    int written = strftime(ptrToEnd(), emptySpace(), aFmt, &aTm);
    updateCurrentLen(written);
    return (written > 0);
  }

  bool copy(const char * aStr, int aCount)
  {
    clear();
    if(aCount > emptySpace())
      return false;

    strncpy(iBuffer, aStr, aCount);
    updateCurrentLen(aCount);
    return true;
  }

  bool copy(const SmartBuffer& aSmartBuffer)
  {
    return copy(aSmartBuffer(), aSmartBuffer.iCurrentLen);
  }

private:
  char* ptrToEnd()
  {
    return (iBuffer + iCurrentLen);
  }

  void updateCurrentLen(int aLenToAdd)
  {
    int sEmptySpace = emptySpace();
    iCurrentLen += (aLenToAdd < sEmptySpace) ? aLenToAdd : sEmptySpace;
  }

private:
  char iBuffer[LEN];
  int iCurrentLen;
};

#endif
