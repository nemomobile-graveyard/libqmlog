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
  //TODO pseudo code
  // NO log_init!
  //some logging
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
  //TODO pseudo code
  // log_init (current init for a while)
  const char *log_file = "logtest.log" ;
  log_t::log_init("logtest", log_file, true, true) ;
  log_info("=========== with init ===========");

  log_without_init(false);
  log_info("========== with init done ==========");
}

int main(void)
{
  log_info("============= start of logtest =============");

  log_with_init();
  log_without_init();

  log_info("============= end of logtest ==============");
  return 0 ;
}
