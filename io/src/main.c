#include "main.h"

int main(int argc, char* argv[]) {
  
  int conexion;
  char* ip_kernel;
  char* puerto_kernel;
  t_log* logger;
  t_config* config;

  logger = iniciar_logger("io.log", "IO", LOG_LEVEL_INFO);
  log_info(logger, "Log de IO iniciado");

  config = iniciar_config("io.config");
  ip_kernel = config_get_string_value(config, "IP_KERNEL");
  puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");

  leer_consola(logger);
  
  conexion = crear_conexion(ip_kernel, puerto_kernel);

  enviar_mensaje("Modulo IO conectado.", conexion);

  paquete(conexion);

  terminar_programa(conexion, logger, config);

  return 0;

}