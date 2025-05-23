#include "main.h"
#include "pcb/pcb.h"
#include "planificacion/planificacion.h"

int main(int argc, char* argv[]) 
{
  logger = iniciar_logger("kernel.log", "Kernel", LOG_LEVEL_DEBUG);
  log_info(logger, "Log de Kernel iniciado");

  config = iniciar_config("kernel.config");
  
  char* PUERTO_ESCUCHA_DISPATCH = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
  //char* PUERTO_ESCUCHA_INTERRUPT = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT");
  char* PUERTO_ESCUCHA_IO = config_get_string_value(config, "PUERTO_ESCUCHA_IO");

  /*
  iniciar_planificacion_largo_plazo();
  iniciar_planificacion_corto_plazo();
  
  t_pcb* pcb1 = crear_pcb();
  t_pcb* pcb2 = crear_pcb();
  
  inicializar_proceso(pcb1);
  inicializar_proceso(pcb2);
  
  mover_proceso_a_ready(pcb1);
  mover_proceso_a_ready(pcb2);

  t_pcb* proceso_ejecutar = mover_proceso_a_exec();

  if(!proceso_ejecutar){
    printf("ERROR: No se seleccionó un proceso de READY");
    return -1;
  };

  printf("Proceso <%d> seleccionado para ejecución (Estado: %s).\n", proceso_ejecutar->pid, obtener_nombre_estado(ESTADO_EXEC));

  finalizar_proceso(pcb1);
  finalizar_proceso(pcb2);
  */

  pthread_t hilo_conexion_cpu_dispatch;
  //pthread_t hilo_conexion_cpu_interrupt;
  pthread_t hilo_conexion_io;

  pthread_create(&hilo_conexion_cpu_dispatch, NULL, conectar_cpu_dispatch, PUERTO_ESCUCHA_DISPATCH);
  pthread_create(&hilo_conexion_io, NULL, conectar_io, PUERTO_ESCUCHA_IO);
  //pthread_create(&hilo_conexion_cpu_interrupt, NULL, conectar_cpu_interrupt, puerto_cpu_interrupt);

  char* IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
  char* PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");
  
  char* archivo_pseudocodigo = argv[1];
  int32_t tamanio_proceso = atoi(argv[2]);

  t_pcb* proceso_inicial = crear_pcb();

  inicializar_proceso(proceso_inicial);

  int conexion_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA, KERNEL);
  
  if (handshake_kernel(conexion_memoria) == 0)
    mover_proceso_a_ready(archivo_pseudocodigo, tamanio_proceso)
  
  pthread_join(hilo_conexion_cpu_dispatch, NULL);
  //pthread_join(hilo_conexion_cpu_interrupt, NULL);
  pthread_join(hilo_conexion_io, NULL);
  
  terminar_programa(conexion_memoria, logger, config);

	return EXIT_SUCCESS;
};