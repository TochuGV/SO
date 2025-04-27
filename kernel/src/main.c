#include "main.h"

int main(int argc, char* argv[]) 
{
  logger = iniciar_logger("kernel.log", "Kernel", LOG_LEVEL_DEBUG);
  log_info(logger, "Log de Kernel iniciado");

  config = iniciar_config("kernel.config");
	
  t_list* lista_puertos = list_create();
  agregar_puertos_a_lista(KERNEL, config, lista_puertos);


  pthread_t thread_conexiones;
  pthread_create(&thread_conexiones, NULL, conectar_puertos_a_servidor, lista_puertos);

  // Intenta conectarse con memoria y si falla continua funcionando como servidor
  int conexion = conectarse_a(MEMORIA, KERNEL, config);

  // Le envia el proceso a ejecutar a memoria
  char* path = argv[1];
  int32_t tamanio_proceso = atoi(argv[2]);
  enviar_proceso_a_memoria(path,tamanio_proceso,conexion);

  pthread_join(thread_conexiones, NULL);



  terminar_programa(conexion, logger, config);

	return EXIT_SUCCESS;

}
