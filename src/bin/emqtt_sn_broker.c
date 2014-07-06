
#include <EMqtt.h>

int
main(void)
{
  emqtt_init();
  emqtt_sn_server_add("::", 1883, 0x01);

  ecore_main_loop_begin();

  emqtt_shutdown();

  return 0;
}
