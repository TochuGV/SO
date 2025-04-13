#include "main.h"

int main(int argc, char* argv[]) 
{

  logger = iniciar_logger("io.log", "IO", LOG_LEVEL_INFO);
  log_info(logger, "Log de IO iniciado");

  config = iniciar_config("io.config");
  int conexion = conectarse_a(KERNEL, IO, config);

  terminar_programa(conexion, logger, config);

  return EXIT_SUCCESS;

}