
#include <Ecore.h>
#include <Ecore_Con.h>

int
main(void)
{
  ecore_mqtt_init();
  ecore_mqtt_sn_server_add("::", 1883);

  ecore_main_loop_begin();

  ecore_mqtt_shutdown();

  return 0;
}
