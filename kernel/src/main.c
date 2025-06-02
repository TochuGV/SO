#include "main.h"

int main(int argc, char* argv[]){
  inicializar_kernel();

  //char* archivo_pseudocodigo = argv[1];
  //int32_t tamanio_proceso = atoi(argv[2]);

  //pthread_t hilo_conexion_cpu_dispatch;
  //pthread_t hilo_conexion_cpu_interrupt; --> Ver si estos hilos los declaro en 'init' o acá.
  //pthread_t hilo_conexion_io;
  
  pthread_create(&hilo_conexion_io, NULL, conectar_io, PUERTO_ESCUCHA_IO);
  pthread_create(&hilo_conexion_cpu_dispatch, NULL, conectar_cpu_dispatch, PUERTO_ESCUCHA_DISPATCH);
  pthread_create(&hilo_conexion_cpu_interrupt, NULL, conectar_cpu_interrupt, PUERTO_ESCUCHA_INTERRUPT);
  
  int conexion_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA, KERNEL);
  
  if (handshake_kernel(conexion_memoria) == 0){
    //mover_proceso_a_ready(archivo_pseudocodigo, tamanio_proceso);
    t_pcb* pcb_nuevo = crear_pcb();
    inicializar_proceso(pcb_nuevo);
    log_info(logger, "Proceso <%d> inicializado manualmente desde 'main.c'", pcb_nuevo->pid);
  };

  pthread_join(hilo_conexion_cpu_dispatch, NULL);
  pthread_join(hilo_conexion_cpu_interrupt, NULL);
  pthread_join(hilo_conexion_io, NULL);
  
  while(1){
    sleep(1);
  };

  terminar_programa(conexion_memoria, logger, config);

  //Se pueden destruir logs, configs, conexiones, listas con elementos, semáforos, diccionarios, etc.

	return EXIT_SUCCESS;
};