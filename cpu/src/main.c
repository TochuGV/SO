#include "main.h"

int main(int argc, char* argv[]) {

  int conexion;
  char* ip_kernel;
  char* puerto_kernel_dispatch;
  t_log* logger;
  t_config* config;

  logger = iniciar_logger("cpu.log", "CPU", LOG_LEVEL_INFO);
  log_info(logger, "Log de CPU iniciado");

  config = iniciar_config("cpu.config");
  ip_kernel = config_get_string_value(config, "IP_KERNEL");
  puerto_kernel_dispatch = config_get_string_value(config, "PUERTO_KERNEL_DISPATCH");

  leer_consola(logger);
  
  conexion = crear_conexion(ip_kernel, puerto_kernel_dispatch);

  enviar_mensaje("Modulo CPU conectado.", conexion);

  paquete(conexion);

  terminar_programa(conexion, logger, config);

  return 0;
}