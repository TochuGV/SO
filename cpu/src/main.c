#include "main.h"

int main(int argc, char* argv[]) 
{
  logger = iniciar_logger("cpu.log", "CPU", LOG_LEVEL_INFO);
  log_info(logger, "Log de CPU iniciado");
  
  config = iniciar_config("cpu.config");

  int32_t identificador_cpu = atoi(argv[1]);

  //IPs
  char* ip_kernel = config_get_string_value(config, "IP_KERNEL");
  char* ip_memoria = config_get_string_value(config, "IP_MEMORIA");

  //Puertos
  char* puerto_kernel_dispatch = config_get_string_value(config, "PUERTO_KERNEL_DISPATCH");
  char* puerto_kernel_interrupt = config_get_string_value(config, "PUERTO_KERNEL_INTERRUPT");
  char* puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");

  //Hilos
  pthread_t hilo_conexion_kernel_dispatch;
  pthread_t hilo_conexion_kernel_interrupt;
  pthread_t hilo_conexion_memoria;

  //Creación de hilos
  pthread_create(&hilo_conexion_kernel_dispatch, NULL, conectar_kernel_dispatch, ip_kernel);
  pthread_create(&hilo_conexion_kernel_interrupt, NULL, conectar_kernel_interrupt, ip_kernel);
  pthread_create(&hilo_conexion_memoria, NULL, conectar_memoria, ip_memoria);

  //Creación de conexiones
  int conexion_kernel_dispatch= crear_conexion(ip_kernel, puerto_kernel_dispatch, CPU);
  int conexion_kernel_interrupt= crear_conexion(ip_kernel, puerto_kernel_interrupt, CPU);
  int conexion_memoria = crear_conexion(ip_memoria, puerto_memoria, CPU);
  
  pthread_join(conexion_kernel_dispatch, NULL);
  pthread_join(conexion_kernel_interrupt, NULL);
  pthread_join(conexion_memoria, NULL);

  terminar_programa(conexion_memoria, logger, config);

  void* dispatch = manejar_dispatch(conexion_kernel_dispatch,conexion_memoria);
  
  //terminar_programa(conexion, logger, config);
  free (dispatch);

  return EXIT_SUCCESS;
}

