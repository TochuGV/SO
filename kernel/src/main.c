#include "main.h"
#include "pcb/pcb.h"
#include "planificacion/planificacion.h"

/*
void probar_pcb(){
  printf("Iniciando la prueba de funcionamiento de PCB...\n");

  t_pcb* pcb = crear_pcb();
  if(pcb == NULL){
    printf("ERROR: No se pudo crear el PCB\n");
    return;
  };

  printf("Creación exitosa");

  printf("PID: %u, PC: %u\n", pcb->pid, pcb->pc);
  for(int i = 0; i < CANTIDAD_ESTADOS; i++){
    printf("Estado %d -> ME: %u, MT: %u\n", i, pcb->me[i], pcb->mt[i]);
  };

  int bytes = sizeof(uint32_t) * (2 + CANTIDAD_ESTADOS * 2);
  void* buffer = serializar_pcb(pcb, bytes);
  if(buffer == NULL){
    printf("ERROR: No se pudo serializar el PCB\n");
    destruir_pcb(pcb);
    return;
  };

  printf("Serialización exitosa. Tamaño: %d Bytes\n", bytes);

  free(buffer);
  destruir_pcb(pcb);

  printf("Destrucción exitosa. Prueba exitosa");
}
*/

int main(int argc, char* argv[]) 
{
  logger = iniciar_logger("kernel.log", "Kernel", LOG_LEVEL_DEBUG);
  log_info(logger, "Log de Kernel iniciado");

  config = iniciar_config("kernel.config");
  
  char* PUERTO_ESCUCHA_DISPATCH = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
  //char* puerto_cpu_interrupt = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT");
  char* PUERTO_ESCUCHA_IO = config_get_string_value(config, "PUERTO_ESCUCHA_IO");

  iniciar_planificacion_largo_plazo();
  
  t_pcb* pcb1 = crear_pcb();
  inicializar_proceso(pcb1);
  cambiar_estado(pcb1, ESTADO_NEW, ESTADO_READY);
  cambiar_estado(pcb1, ESTADO_READY, ESTADO_EXEC);
  cambiar_estado(pcb1, ESTADO_EXEC, ESTADO_BLOCKED);
  cambiar_estado(pcb1, ESTADO_BLOCKED, ESTADO_READY);
  cambiar_estado(pcb1, ESTADO_READY, ESTADO_EXEC);
  cambiar_estado(pcb1, ESTADO_EXEC, ESTADO_EXIT);
  finalizar_proceso(pcb1);

  pthread_t hilo_conexion_cpu_dispatch;
  //pthread_t hilo_conexion_cpu_interrupt;
  pthread_t hilo_conexion_io;

  pthread_create(&hilo_conexion_cpu_dispatch, NULL, conectar_cpu_dispatch, PUERTO_ESCUCHA_DISPATCH);
  pthread_create(&hilo_conexion_io, NULL, conectar_io, PUERTO_ESCUCHA_IO);
  //pthread_create(&hilo_conexion_cpu_interrupt, NULL, conectar_cpu_interrupt, puerto_cpu_interrupt);

  char* IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
  char* PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");
  
  char* path = argv[1];
  int32_t tamanio_proceso = atoi(argv[2]);

  int conexion_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA, KERNEL);
  
  if (handshake_kernel(conexion_memoria) == 0)
    enviar_proceso_a_memoria(path, tamanio_proceso, conexion_memoria);
  
  pthread_join(hilo_conexion_cpu_dispatch, NULL);
  //pthread_join(hilo_conexion_cpu_interrupt, NULL);
  pthread_join(hilo_conexion_io, NULL);
  
  terminar_programa(conexion_memoria, logger, config);

	return EXIT_SUCCESS;
};