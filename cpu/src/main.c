#include "main.h"

int main(int argc, char* argv[]) 
{
  logger = iniciar_logger("cpu.log", "CPU", LOG_LEVEL_INFO);
  config = iniciar_config("cpu.config");

   int32_t identificador_cpu = atoi(argv[1]);

  iniciar_cpu(identificador_cpu);

  //Hilos
  pthread_t hilo_kernel_dispatch;
  pthread_t hilo_kernel_interrupt;
  pthread_t hilo_memoria;

  pthread_create(&hilo_kernel_dispatch, NULL, conectar, datos_dispatch);
  //pthread_create(&hilo_kernel_interrupt, NULL, conectar, datos_interrupt); comentada porqque vamos a manejar la conexion manualmente 
  pthread_create(&hilo_memoria, NULL, conectar, datos_memoria);
  int socket_interrupt= crear_conexion (ip_kernel, puerto_kernel_interrupt, CPU);

  if (socket_interrupt == -1) {
    log_error (logger, "No se pudo conectar al kernel por puerto de interrupciones ");
    exit (EXIT_FAILURE);
  
  }
//handshake con interrupt
  int32_t handshake_header = CPU;
  int32_t respuesta ;
  int32_t identificador = identificador_cpu;

  send(socket_interrupt, &handshake_header, sizeof(int32_t), 0);
  recv(socket_interrupt, &respuesta, sizeof(int32_t), MSG_WAITALL);
  send(socket_interrupt, &identificador , sizeof(int32_t), 0);

//lanzar hilo que escuche interrupciones 
  pthread_t hilo_interrupt;
  pthread_create(&hilo_interrupt, NULL, escuchar_interrupt, &socket_interrupt);


  ///pthread_join(hilo_kernel_dispatch, NULL);
  pthread_join(hilo_kernel_interrupt, NULL);
  pthread_join(hilo_memoria, NULL);

  return EXIT_SUCCESS;
}

