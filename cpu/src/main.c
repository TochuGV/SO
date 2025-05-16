#include "main.h"

int main(int argc, char* argv[]) 
{
  logger = iniciar_logger("cpu.log", "CPU", LOG_LEVEL_INFO);
  config = iniciar_config("cpu.config");

  int32_t identificador_cpu = atoi(argv[1]);

  iniciar_cpu();

  //Hilos
  pthread_t hilo_kernel_dispatch;
  pthread_t hilo_kernel_interrupt;
  pthread_t hilo_memoria;

  pthread_create(&hilo_kernel_dispatch, NULL, conectar, datos_dispatch);
  pthread_create(&hilo_kernel_interrupt, NULL, conectar, datos_interrupt);
  pthread_create(&hilo_memoria, NULL, conectar, datos_memoria);

  pthread_join(hilo_kernel_dispatch, NULL);
  pthread_join(hilo_kernel_interrupt, NULL);
  pthread_join(hilo_memoria, NULL);

  //void* dispatch = manejar_dispatch(conexion_kernel_dispatch,conexion_memoria);
  
  //terminar_programa(conexion, logger, config);
  //free (dispatch);
  free(datos_dispatch);
  free(datos_interrupt);
  free(datos_memoria);
  return EXIT_SUCCESS;
}

