#include "main.h"

int main(int argc, char* argv[]) 
{

  logger = iniciar_logger("io.log", "IO", LOG_LEVEL_INFO);
  log_info(logger, "Log de IO iniciado");

  config = iniciar_config("io.config");

  char* nombre_io = argv[1];
  
  char* IP_KERNEL = config_get_string_value(config, "IP_KERNEL");
  char* PUERTO_KERNEL = config_get_string_value(config, "PUERTO_KERNEL");
  
  int conexion_kernel = crear_conexion(IP_KERNEL, PUERTO_KERNEL, IO);

  if (handshake_io(nombre_io, conexion_kernel) == 0) {
    atender_interrupcion(conexion_kernel);
  }

  terminar_programa(conexion_kernel, logger, config);

  return EXIT_SUCCESS;
}

