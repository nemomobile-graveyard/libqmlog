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
#ifndef MAEMO_QMLOG_LOG_DECLARATIONS_H
#define MAEMO_QMLOG_LOG_DECLARATIONSH

/* Verbosity levels, the upper boundary could be set at compile time.
 *
 * -1  NO LOGS --- all the log macros are disabled
 *  0  INTERNAL --- produced by failing log_assert(...) (mostly with file:line function)
 *  1  CRITICAL --- programm can continue, but some stuff is lost/can't be done
 *  2  ERROR --- incorrect input
 *  3  WARNING --- tolerable input, should be corrected
 *  4  NOTICE --- just some blah blah
 *  5  INFO --- debugging info (usually with file:line function)
 *  6  DEBUG --- verbose info (usually with file:line function)
 *
 */

#define LOG_LEVEL_NO_LOGS  -1
#define LOG_LEVEL_INTERNAL  0
#define LOG_LEVEL_CRITICAL  1
#define LOG_LEVEL_ERROR     2
#define LOG_LEVEL_WARNING   3
#define LOG_LEVEL_NOTICE    4
#define LOG_LEVEL_INFO      5
#define LOG_LEVEL_DEBUG     6

#ifndef LOG_MAX_LEVEL
#define LOG_MAX_LEVEL LOG_LEVEL_DEBUG
#endif

#define LOG_BIT_MASK(bit) (1 << (bit))

#ifndef LOG_MAX_LOCATION
#define LOG_MAX_LOCATION (LOG_BIT_MASK(LOG_LEVEL_DEBUG)|LOG_BIT_MASK(LOG_LEVEL_INFO)|LOG_BIT_MASK(LOG_LEVEL_INTERNAL))
#endif

#ifndef LOG_ASSERTION
#define LOG_ASSERTION 1
#endif

#if LOG_MAX_LEVEL<LOG_LEVEL_NO_LOGS || LOG_MAX_LEVEL>LOG_LEVEL_DEBUG
#error LOG_MAX_LEVEL outside of [-1..6]
#endif

#endif
