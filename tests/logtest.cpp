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

#include <qm/log>

void test_very_long_string()
{
  log_info("=== very log string testing ===");
  log_critical("%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
              "30 long strings 100 chars each:",
              "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
              "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
              "cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc",
              "dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd",
              "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee",
              "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
              "gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg",
              "hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh",
              "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii",
              "jjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjj",
              "kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk",
              "llllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllll",
              "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm",
              "nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn",
              "oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo",
              "pppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppp",
              "qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq",
              "rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr",
              "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss",
              "tttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt",
              "uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu",
              "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv",
              "wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww",
              "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",
              "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy",
              "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz",
              "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
              "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
              "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC",
              "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD");
  log_info("===============================");
}

void test_empty_log_macro()
{
  log_info("===== empty macro testing =====");
  log_debug();
  log_info();
  log_warning();
  log_error();
  log_critical();
  log_info("===============================");
}

#define test_log_macro_with_fmt(fmt, ...) \
do {                                                  \
  log_info("====== fmt macro testing =====");         \
  log_debug(fmt, ## __VA_ARGS__);                     \
  log_info(fmt, ## __VA_ARGS__);                      \
  log_warning(fmt, ## __VA_ARGS__);                   \
  log_error(fmt, ## __VA_ARGS__);                     \
  log_critical(fmt, ## __VA_ARGS__);                  \
  log_info("===============================");        \
} while(0)                                            \

void do_log()
{
  test_empty_log_macro();
  test_log_macro_with_fmt("string only");
  test_log_macro_with_fmt("\"x = %d\" == \"x = 5\"", 5);

  const char * format = "\"fmt\" is in \"const char * format\"";
  test_log_macro_with_fmt(format);
}

void log_change_settings_locally_suppress()
{
  log_critical("After this log you shall not see any log from log_change_settings_locally_suppress()");

  SET_TEMP_LOG_SETTINGS_SUPPRESS_LOGGING();

  log_critical("==== temp suppress output settings ====");
  do_log();
  log_critical("== temp suppress output settings done =");
}

void log_change_settings_locally_second()
{
  ADD_FILE_LOG("my-logtest-minor.log");
  log_debug("Inital level for my-logtest-minor.log shall be debug");

  SET_TEMP_LOG_SETTINGS(LOG_LEVEL_CRITICAL, LOG_MAX_LOCATION, LoggerSettings::EMessage);
  ADD_STDOUT_LOG(LOG_LEVEL_INFO);

  log_critical("==== temp minor output settings ====");
  do_log();
  log_change_settings_locally_suppress();
  log_critical("== temp minor output settings done =");
}

void log_change_settings_locally()
{
  SET_TEMP_LOG_SETTINGS_MAX_DEBUG();

  log_warning("===== temp max debug settings ======");
  do_log();
  log_change_settings_locally_second();
  log_warning("=== temp max debug settings done ===");
}

void log_without_init()
{
  ADD_PERMANENT_FILE_LOG("my-permanent-logtest.log");
  log_warning("============== no init =============");

  do_log();
  log_change_settings_locally();

  log_warning("=========== no init done ===========");
}

void log_with_init()
{
  INIT_LOGGER("my-logtest");
  ADD_SYSLOG();
  ADD_STDERR_LOG();
  ADD_FILE_LOG("my-logtest.log");

  log_warning("=========== with init ===========");
  do_log();
  log_change_settings_locally();
  log_warning("========== with init done ==========");
}

int main(void)
{
  // TODO: make a multi-call binary, to have 3 different test cases
  log_warning("============= start of logtest =============");

  log_with_init();
  log_without_init();
  test_very_long_string();

  log_warning("============= end of logtest ==============");
  return 0;
}
