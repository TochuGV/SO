#include "main.h"

int main(int argc, char* argv[]) 
{
  logger = iniciar_logger("cpu.log", "CPU", LOG_LEVEL_INFO);
  log_info(logger, "Log de CPU iniciado");
  
  config = iniciar_config("cpu.config");

  char* ip_kernel = config_get_string_value(config, "IP_KERNEL");
  //char* ip_memoria = config_get_string_value(config, "IP_MEMORIA");

  int32_t identificador_cpu = atoi(argv[1]);

  char* puerto_kernel_dispatch = config_get_string_value(config, "PUERTO_KERNEL_DISPATCH");
  //char* puerto_kernel_interrupt = config_get_string_value(config, "PUERTO_KERNEL_INTERRUPT");
  //char* puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");

  int conexion_kernel_dispatch = crear_conexion(ip_kernel, puerto_kernel_dispatch, CPU);
  //int conexion_kernel_interrupt = crear_conexion(ip_kernel, puerto_kernel_interrupt, CPU);
  //int conexion_memoria = crear_conexion(ip_memoria, puerto_memoria, CPU);

  if (handshake_cpu(identificador_cpu, conexion_kernel_dispatch) == 0) {
    paquete(conexion_kernel_dispatch);
  }

  
  //terminar_programa(conexion, logger, config);

  return EXIT_SUCCESS;

}

