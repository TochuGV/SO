#include "fifo.h"

void mover_proceso_a_ready_fifo(void){
  while(!queue_is_empty(cola_new)){
    t_pcb* pcb = queue_peek(cola_new);

    uint32_t pid_buscado = pcb->pid;
    bool tiene_pid_igual(void* elem){
      return ((t_informacion_largo_plazo*) elem)->pid == pid_buscado;
    };

    t_informacion_largo_plazo* info = list_find(lista_info_procesos, tiene_pid_igual);
    if(!info) return;

    if(enviar_proceso_a_memoria(info->archivo_pseudocodigo, info->tamanio, pcb->pid) == 0){
      encolar_proceso_en_ready(pcb);
      cambiar_estado(pcb, ESTADO_NEW, ESTADO_READY);
      sem_post(&semaforo_ready);
      list_remove_by_condition(lista_info_procesos, tiene_pid_igual); //--> Revisar de crear una función para remover el elemento de la cola sin acceder directamente al campo 'elements'
      queue_pop(cola_new);
      continue;
    } else {
      //log_info(logger, "Proceso <%d> no pudo entrar a memoria. Sigue en NEW...", pcb->pid);
      break;
    };
  };
};