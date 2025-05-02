#include "main.h"

int main(int argc, char* argv[]) 
{

  logger = iniciar_logger("io.log", "IO", LOG_LEVEL_INFO);
  log_info(logger, "Log de IO iniciado");

  config = iniciar_config("io.config");

  char* nombre_io = argv[1];
  
  char* ip_kernel = config_get_string_value(config, "IP_KERNEL");
  char* puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");
  
  int conexion_kernel = crear_conexion(ip_kernel, puerto_kernel, IO);

  if (handshake_io(nombre_io, conexion_kernel) == 0) {
    paquete(conexion_kernel);
    //atender_interrupcion();
  }

  terminar_programa(conexion_kernel, logger, config);

  return EXIT_SUCCESS;
}

