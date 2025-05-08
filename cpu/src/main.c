#include "main.h"

int main(int argc, char* argv[]) 
{
  logger = iniciar_logger("cpu.log", "CPU", LOG_LEVEL_INFO);
  log_info(logger, "Log de CPU iniciado");
  
  config = iniciar_config("cpu.config");

  int32_t identificador_cpu = atoi(argv[1]);

  //int conexion_kernel_dispatch = crear_conexion(ip_kernel, puerto_kernel_dispatch, CPU);
  //int conexion_kernel_interrupt = crear_conexion(ip_kernel, puerto_kernel_interrupt, CPU);
  //int conexion_memoria = crear_conexion(ip_memoria, puerto_memoria, CPU);

  t_cpu_args* args = malloc(sizeof(t_cpu_args));

  pthread_t cpu_thread;
  pthread_create(&cpu_thread, NULL, conexiones_modulos, (void*) args);


  if (handshake_cpu(identificador_cpu, args->socket_dispatch) == 0) {
    paquete(args->socket_dispatch);
  }

  void* dispatch = manejar_dispatch(args);
  
  //terminar_programa(conexion, logger, config);
  free (dispatch);

  return EXIT_SUCCESS;
}
