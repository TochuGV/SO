#include "fifo.h"

void mover_proceso_a_ready_fifo(void){
  while(!queue_is_empty(cola_new)){
    t_pcb* pcb = queue_peek(cola_new);

    if(intentar_enviar_proceso_a_ready(pcb)){
      queue_pop(cola_new);
      continue;
    } else {
      break;
    };
  };
};