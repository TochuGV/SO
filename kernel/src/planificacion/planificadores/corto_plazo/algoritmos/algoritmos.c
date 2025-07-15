#include "algoritmos.h"

t_pcb* obtener_proximo_proceso_ready(t_queue* cola_ready){
  if(strcmp(ALGORITMO_CORTO_PLAZO, "SJF") == 0 || strcmp(ALGORITMO_CORTO_PLAZO, "SRT") == 0){
    reordenar_cola_ready_por_estimacion(cola_ready);
    return queue_pop(cola_ready);
  } else {
    return queue_pop(cola_ready);
  };
};

void asignar_y_enviar_a_cpu(t_pcb* pcb, t_cpu* cpu){
  log_debug(logger, "Asignando proceso %d a CPU %d (%s)", pcb->pid, cpu->id_cpu, ALGORITMO_CORTO_PLAZO);
  asignar_proceso_a_cpu(cpu, pcb);
  enviar_a_cpu(pcb, cpu->socket_dispatch);
  cambiar_estado(pcb, ESTADO_READY, ESTADO_EXEC);
};