#include "main.h"

int main(int argc, char* argv[]) 
{

  logger = iniciar_logger("cpu.log", "CPU", LOG_LEVEL_INFO);
  log_info(logger, "Log de CPU iniciado");

  config = iniciar_config("cpu.config");
  int conexion = conectarse_a(KERNEL, CPU, config);

  terminar_programa(conexion, logger, config);

  return EXIT_SUCCESS;

}