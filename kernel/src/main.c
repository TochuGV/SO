#include "main.h"

int main(int argc, char* argv[]) {

  char* puerto_io;
	char* puerto_cpu_dispatch;
  char* ip;
  
  t_config* config;
  
  logger = iniciar_logger("kernel.log", "Kernel", LOG_LEVEL_DEBUG);

  config = iniciar_config("kernel.config");
	
  ip = config_get_string_value(config, "IP_KERNEL");
  puerto_io = config_get_string_value(config, "PUERTO_ESCUCHA_IO");
  puerto_cpu_interrupt = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT");
	puerto_cpu_dispatch = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");

  iniciar_conexion(puerto_io);

  crear_conexion(ip, puerto_cpu_dispatch)
  iniciar_conexion(puerto_cpu_dispatch);

	return EXIT_SUCCESS;
}

