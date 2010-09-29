#include <log-declarations.h>

void log_change_settings_locally()
{
}

void log_without_init()
{
  //TODO pseudo code
  // NO log_init!
  //some logging
  log_debug("log message 1");

  log_change_settings_locally();
  //some logging to check that settings reset back
}

void log_with_init()
{
  //TODO pseudo code
  // log_init (current init for a while)
  const char *log_file = "logtest.log" ;
  log_init("logtest", log_file, true, true) ;

  log_without_init();
}

int main(void)
{
  log_debug("======= start of logtest =======");

  log_with_init();
  log_without_init();

  log_debug("======= end of logtest =======");
  return 0 ;
}
