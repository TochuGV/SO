#include "pmcp.h"

t_pcb* elegir_proceso_mas_chico(){
  if(queue_is_empty(cola_new) || list_is_empty(lista_info_procesos)) return NULL;
  t_informacion_largo_plazo* candidato = NULL;

  for(int i = 0; i < list_size(lista_info_procesos); i++){
    t_informacion_largo_plazo* info = list_get(lista_info_procesos, i);
    if(candidato == NULL || info->tamanio < candidato->tamanio){
      candidato = info;
    };
  };

  if(candidato == NULL) return NULL;

  t_pcb* pcb_seleccionado = NULL;
  t_queue* nueva_cola_new = queue_create();

  while(!queue_is_empty(cola_new)){
    t_pcb* pcb = queue_pop(cola_new);
    if(pcb->pid == candidato->pid){
      pcb_seleccionado = pcb;
    } else {
      queue_push(nueva_cola_new, pcb);
    };
  };

  while(!queue_is_empty(nueva_cola_new)){
    queue_push(cola_new, queue_pop(nueva_cola_new));
  };
  queue_destroy(nueva_cola_new);

  return pcb_seleccionado;
};

void mover_proceso_a_ready_pmcp(void){
  while(!queue_is_empty(cola_new)){
    t_pcb* pcb = elegir_proceso_mas_chico();
    if(!pcb) break;

    if(intentar_enviar_proceso_a_ready(pcb)){
      continue;
    } else {
      queue_push(cola_new, pcb);
      break;
    };
  };
};