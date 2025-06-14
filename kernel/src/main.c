#include "main.h"

int main(int argc, char* argv[]){
  inicializar_kernel();

  char* archivo_pseudocodigo = argv[1];
  int32_t tamanio_proceso = atoi(argv[2]);
  
  iniciar_conexiones_entre_modulos();

  conexion_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA, MODULO_KERNEL);
  
  if (handshake_kernel(conexion_memoria) != 0){
    log_error(logger, "No se pudo conectar a Memoria debido a un fallo en el Handshake");
    return EXIT_FAILURE;
  };

  esperar_enter_para_planificar();

  t_pcb* pcb_nuevo = crear_pcb();
  inicializar_proceso(pcb_nuevo);
  log_debug(logger, "Proceso <%d> inicializado manualmente desde 'main.c'", pcb_nuevo->pid);
  mover_proceso_a_ready(archivo_pseudocodigo, tamanio_proceso);
  
  pthread_create(&hilo_planificacion, NULL, planificador_ciclo_general, NULL);

  pthread_join(hilo_conexion_cpu_dispatch, NULL);
  pthread_join(hilo_conexion_cpu_interrupt, NULL);
  pthread_join(hilo_conexion_io, NULL);
  pthread_join(hilo_planificacion, NULL);

  terminar_programa(conexion_memoria, logger, config);
  dictionary_destroy_and_destroy_elements(diccionario_contextos_io, destruir_contexto_io);
  //Se pueden destruir logs, configs, conexiones, listas con elementos, semáforos, diccionarios, etc.
	return EXIT_SUCCESS;
};