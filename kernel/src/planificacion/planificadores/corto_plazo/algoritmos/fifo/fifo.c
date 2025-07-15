#include "fifo.h"

void mover_proceso_a_exec_fifo(void){
  pthread_mutex_lock(&mutex_ready);
  if(queue_is_empty(cola_ready)){
    pthread_mutex_unlock(&mutex_ready);
    return;
  };

  t_cpu* cpu = seleccionar_cpu_disponible();
  if(!cpu){
    pthread_mutex_unlock(&mutex_ready);
    return;
  };

  t_pcb* pcb = queue_pop(cola_ready);
  pthread_mutex_unlock(&mutex_ready);
  asignar_y_enviar_a_cpu(pcb, cpu);
};