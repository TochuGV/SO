#include "main.h"

int main(int argc, char* argv[]) 
{

  logger = iniciar_logger("memoria.log", "Memoria", LOG_LEVEL_DEBUG);
  log_info(logger, "Log de Memoria iniciado");

  config = iniciar_config("memoria.config");
	
  pthread_t hilo_conexion_kernel;
  //pthread_t hilo_conexion_cpu;

  char* puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
  
  pthread_create(&hilo_conexion_kernel, NULL, conectar_kernel, puerto_escucha);
  //pthread_create(&hilo_conexion_cpu, NULL, conectar_cpu, puerto_escucha);

  pthread_join(hilo_conexion_kernel,NULL);
  //pthread_join(hilo_conexion_cpu,NULL);


  //terminar_programa(conexion, logger, config);

  return EXIT_SUCCESS;

}
