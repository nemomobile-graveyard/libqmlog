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
#include <string>
using namespace std ;

#include <qmlog>

void test_very_long_string() ;
void test_growing_length() ;
void test_empty_log_macro() ;
void test_change_log_level() ;
void test_add_and_remove_logfile() ;
void run_all() ;

int main(int argc, char *argv[])
{
  /* First check, that the command line is available */
  if (argc<1)
    log_abort("Program is started without 'argv' array, terminating...") ;

  /* now make an assertion */
  log_assert(argc>0) ;
  /* it can't fail if the program compiled without NDEBUG due to abortion above */

  /* now use an usual call */
  log_notice("started...") ;

  /* this program is a simple multi-call ilbrary */
  for(int i=1; i<argc; ++i)
  {
    if (not true) (void)true ;
#define run_if_match(x) else if((string)argv[i]==#x) x()
    run_if_match(run_all) ;
    run_if_match(test_very_long_string) ;
    run_if_match(test_growing_length) ;
    run_if_match(test_empty_log_macro) ;
    run_if_match(test_change_log_level) ;
    run_if_match(test_add_and_remove_logfile) ;
    else
      /* unknow function, log it as a non-critical error */
      log_error(false, "invalid function name: '%s'", argv[i]) ;
#undef  run_if_match
    }

  /* now we're done, log it... */
  log_notice("done, exiting...") ;

  /* ... and exit: that's it */
  return 0 ;
}

void run_all(void)
{
  /* execution of all test functions */
  log_notice("full test run") ;

  // skip it, these tests is too heavy
  // test_very_long_string() ;
  // test_growing_length() ;

  test_empty_log_macro() ;
  test_change_log_level() ;
  test_add_and_remove_logfile() ;

  log_notice("full test done") ;
}

#define log_all_levels(...) \
do { \
  log_debug(__VA_ARGS__) ; \
  log_info(__VA_ARGS__) ; \
  log_notice(__VA_ARGS__) ; \
  log_warning(__VA_ARGS__) ; \
  log_error(__VA_ARGS__) ; \
  log_critical(__VA_ARGS__) ; \
  log_internal(__VA_ARGS__) ; \
  log_assert(true, ## __VA_ARGS__) ; /* no ouput from this line */ \
} while(0)

void test_very_long_string()
{
  /* the idea here is to produce a huge logging mesage */
  /* who knows, we may have a buffer overflow in the library */

  /* 26 strings a 10000 bytes */
  const unsigned int N=26, LEN=10000 ;
#define FMT "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s"

  /* First check, if the format string has really N "%s" specifications */
  /* Print a message (and abort) if not */
  log_assert(strlen(FMT)==2*N, "wrong length of the format string: %d (%d expected)", strlen(FMT), 2*N) ;

#define ABCDEFGHIJKLMNOPQRSTUVWXYZ a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z
  typedef char *pointer_to_char_t ;
  pointer_to_char_t ABCDEFGHIJKLMNOPQRSTUVWXYZ ;

  /* a funny initialization macro:
   * fill variable x with LEN letters 'x', use xx as the loop counter
   * log it as well, check string length */

#define _(x) \
  do { \
    log_debug("Initializing variable '%s' with %d letters '%c'", #x, LEN, #x[0]) ; \
    x = new char [LEN+1] ; \
    x[LEN] = '\0' ; \
    for(unsigned x##x=0; x##x<LEN; ++x##x) x[x##x] = #x[0] ; \
    log_assert(strlen(x)==LEN, "wrong length of the string variable %s: %d (%d expected)", #x, strlen(x), LEN) ; \
  } while(0)

  /* now execute this macro N=26 times */
  _(a) ; _(b) ; _(c) ; _(d) ; _(e) ; _(f) ; _(g) ; _(h) ; _(i) ; _(j) ; _(k) ; _(l) ; _(m) ;
  _(n) ; _(o) ; _(p) ; _(q) ; _(r) ; _(s) ; _(t) ; _(u) ; _(v) ; _(w) ; _(x) ; _(y) ; _(z) ;

  /* don't need this macro any more */
#undef _

  log_notice("starting every-level logging with a huge message") ;
  log_all_levels(FMT, ABCDEFGHIJKLMNOPQRSTUVWXYZ) ;
  log_notice("success") ;
#undef FMT
#undef ABCDEFGHIJKLMNOPQRSTUVWXYZ
}

void test_growing_length()
{
  /* the idea here is to test the library for buffer oveflow again */
  /* we're increasing the length of the message step by step */

  /* this will produce a huge amount of logging output, about 7x50 MB */
  const unsigned int LEN = 10000 ;
  log_notice("starting every-level logging with a growing message") ;

  string s = "" ;
  for(unsigned i=0; i<=LEN; ++i, s += (string)"x")
    log_all_levels("length=%d, string: '%s'", i, s.c_str()) ;

  log_notice("success") ;
}

void test_empty_log_macro()
{
  /* here we're executing logging methods without any messages */
  log_notice("starting every-level logging without any message") ;
  log_all_levels() ;
  log_notice("success") ;
}

void do_log()
{
  /* here we're executing logging methods with different messages */

  /* empty egain */
  log_all_levels() ;

  /* a single string */
  log_all_levels("Hello, world!") ;

  /* an integer formatting argument */
  log_all_levels("'x=%d' is the same as 'x=239'", 239) ;

  /* as for the 'printf' function,
   * the formatting string can be hidden in a variable */

  const char *format = "formatting string is a 'const char *' variable; int=%d, float=%5.2f, string='%s'" ;
  log_all_levels(format, 239, 3.1415926, "hello again!") ;

  /* But this is very dangerous:
   * the compiler can't check types of trailing parameters!
   *
   * The following line will be compiled without any warnings,
   * and produce some random numbers in output (may be even crash) */
  if (false)
   log_all_levels(format, 3.1415926, 239239239, "third hello!") ;

  /* Do not even try to execute this ! */
  if (false)
    log_all_levels(format, "crash almost for sure!", 239, 3.1415926) ;
}

void test_change_log_level()
{
  /* The log level can be changed at run time */

  /* First get the current level, we want to restore it later */
  int level = qmlog::log_level() ;

  /* Print all kind of messages */
  log_notice("current log level: %d", level) ;
  log_assert(level==qmlog::Full) ;
  log_notice("producing log messages in all levels") ;
  do_log();

  /* Only emergence messages */
  log_notice("reducing log level to 'internal' (%d)", qmlog::Internal) ;
  log_notice("producing log messages in all levels, only 'internal error' should be visible") ;
  qmlog::log_level(qmlog::Internal) ;
  do_log();

  /* All kind of errors and even warnings */
  qmlog::log_level(level) ;
  log_notice("setting log level to 'warning' (%d)", qmlog::Warning) ;
  log_notice("producing log messages in all levels, only all kind of errors and warnings should be visible") ;
  qmlog::log_level(qmlog::Warning) ;
  do_log() ;

  /* no logging at all */
  qmlog::log_level(level) ;
  log_notice("setting log level to 'none' (%d)", qmlog::None) ;
  log_notice("producing log messages in all levels, absolute silence expected") ;
  qmlog::log_level(qmlog::None) ;
  do_log() ;

  /* back to full logging */
  qmlog::log_level(level) ;
  log_notice("restoring original log level (%d)", level) ;
  log_notice("producing log messages in all levels") ;
  do_log();
}

void test_add_and_remove_logfile()
{
  /* It's possible to add and remove again additional logging channel */

  qmlog::log_file *file = new qmlog::log_file("/tmp/test_add_and_remove_logfile.log") ;

  /* Let's print exact time to this log file */
  file->enable_fields(qmlog::Monotonic_Nano | qmlog::Time_Micro) ;

  /* The next logging output will go to usual places and to the new log file */
  log_notice("the next logging is going to the additional log file as well") ;
  do_log() ;

  /* The log level of a channel can be adjusted without any change of global log level */
  log_notice("reducing log level for the file to warnings and errors, no change for other channels") ;
  file->log_level(qmlog::Warning) ;
  do_log() ;

  file->log_level(qmlog::Full) ;

  /* But it's not possible to have a channel with a higher level than the master level */
  log_notice("only errors are visible now") ;
  file->log_level(qmlog::Full) ;
  qmlog::log_level(qmlog::Warning) ;
  do_log() ;

  qmlog::log_level(qmlog::Full) ;
  file->log_level(qmlog::Full) ;

  /* now removing the channel */
  log_notice("removing additional log file channel, this is the last message in this file") ;
  delete file ;
  do_log() ;
}

#if 0
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
#endif

