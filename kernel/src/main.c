#include "main.h"

int main(int argc, char* argv[]) 
{
  logger = iniciar_logger("kernel.log", "Kernel", LOG_LEVEL_DEBUG);
  log_info(logger, "Log de Kernel iniciado");

  config = iniciar_config("kernel.config");

  char* puerto_cpu_dispatch = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
  //char* puerto_cpu_interrupt = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT");
  char* puerto_io = config_get_string_value(config, "PUERTO_ESCUCHA_IO");

  pthread_t hilo_conexion_cpu_dispatch;
  //pthread_t hilo_conexion_cpu_interrupt;
  pthread_t hilo_conexion_io;


  pthread_create(&hilo_conexion_cpu_dispatch, NULL, conectar_cpu_dispatch, puerto_cpu_dispatch);
  pthread_create(&hilo_conexion_io, NULL, conectar_io, puerto_io);
  //pthread_create(&hilo_conexion_cpu_interrupt, NULL, conectar_cpu_interrupt, puerto_cpu_interrupt);

  char* ip_memoria = config_get_string_value(config, "IP_MEMORIA");
  char* puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");

  char* path = argv[1];
  int32_t tamanio_proceso = atoi(argv[2]);

  int conexion_memoria = crear_conexion(ip_memoria, puerto_memoria, KERNEL);
  if (handshake_kernel(conexion_memoria) == 0)
    enviar_proceso_a_memoria(path, tamanio_proceso, conexion_memoria);
  
  pthread_join(hilo_conexion_cpu_dispatch, NULL);
  //pthread_join(hilo_conexion_cpu_interrupt, NULL);
  pthread_join(hilo_conexion_io, NULL);

  terminar_programa(conexion_memoria, logger, config);

	return EXIT_SUCCESS;

}


