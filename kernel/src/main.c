#include "main.h"

int main(int argc, char* argv[]) 
{
  logger = iniciar_logger("kernel.log", "Kernel", LOG_LEVEL_DEBUG);
  log_info(logger, "Log de Kernel iniciado");

  config = iniciar_config("kernel.config");
	
  t_list* lista_puertos = list_create();
  agregar_puertos_a_lista(KERNEL, config, lista_puertos);
  /*
  conexion_cpu_dispatch = iniciar_conexion2(list_get(0)); // PUERTO_ESCUCHA_DISPATCH
  pthread_create(&hilo_cpu_dispatch, NULL, atender_cpu_dispatch, conexion_cpu_dispatch);
  pthread_join(hilo_cpu_dispatch, NULL);

  conexion_cpu_interrupt = iniciar_conexion2(list_get(1));
  pthread_create(&hilo_cpu_dispatch, NULL, comunicacion_cpu_interrupt, conexion_cpu_interrupt);
  pthread_join(hilo_cpu_interrupt, NULL);

  conexion_io = iniciar_conexion2(list_get(2));
  pthread_create(&hilo_cpu_dispatch, NULL, comunicacion_io, conexion_io);
  pthread_join(hilo_io, NULL);*/

  pthread_t thread_conexiones;
  pthread_create(&thread_conexiones, NULL, conectar_puertos_a_servidor, lista_puertos);

  // Intenta conectarse con memoria y si falla continua funcionando como servidor
  int conexion = conectarse_a(MEMORIA, KERNEL, config);

  // Le envia el proceso a ejecutar a memoria
  char* path = argv[1];
  int32_t tamanio_proceso = atoi(argv[2]);
  enviar_proceso_a_memoria(path, tamanio_proceso, conexion);

  pthread_join(thread_conexiones, NULL);



  terminar_programa(conexion, logger, config);

	return EXIT_SUCCESS;

}

/*
void* atender_cpu_dispatch(void* socket_cpu_dispatch)
{
  if (recibir_handshake(socket_cpu_dispatch) == 0) {
    pthread_exit((void*)EXIT_FAILURE);
  } else {

    while (1) {
      int cod_op = recibir_operacion(socket_cpu_dispatch);
      switch (cod_op) {
      case MENSAJE:
        recibir_mensaje(socket_cpu_dispatch);
        break;
      case SYSCALL:
        recibir_syscall()
        break;
      case -1:
        log_error(logger, "El cliente se desconectó. Terminando servidor...");
        pthread_exit((void*)EXIT_FAILURE);
      default:
        log_warning(logger,"Operación desconocida. No quieras meter la pata.");
        break;
      }
    }
    return NULL;
  }
}*/