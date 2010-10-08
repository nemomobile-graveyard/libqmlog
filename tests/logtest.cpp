#include <qm/log>

void log_change_settings_locally()
{
  //TODO implement
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


void log_without_init(bool print_start_end_log = true)
{
  if(print_start_end_log)
  {
    log_info("============== no init =============");
  }

  test_empty_log_macro();
  test_log_macro_with_fmt("string only");
  test_log_macro_with_fmt("\"x = %d\" == \"x = 5\"", 5);

  const char * format = "\"fmt\" is in \"const char * format\"";
  test_log_macro_with_fmt(format);

  log_change_settings_locally();

  if(print_start_end_log)
  {
    log_info("=========== no init done ===========");
  }
}

void log_with_init()
{
  INIT_LOGGER("my-logtest");
  ADD_DEBUG_SYSLOG();
  ADD_STDERR_LOG(FileLoggerDev::DefaultLevel, FileLoggerDev::DefaultLocation, FileLoggerDev::DefaultFormat);
  ADD_STDOUT_LOG(LOG_LEVEL_INFO);
  ADD_FILE_LOG("my-logtest.log");

  log_warning("=========== with init ===========");

  log_without_init(false);
  log_warning("========== with init done ==========");
}

int main(void)
{
  log_warning("============= start of logtest =============");

  log_with_init();
  log_without_init();

  log_warning("============= end of logtest ==============");
  return 0 ;
}
