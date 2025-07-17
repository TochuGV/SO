#include "algoritmos.h"

bool intentar_enviar_proceso_a_ready(t_pcb* pcb){
  uint32_t pid_buscado = pcb->pid;
  
  bool tiene_pid_igual(void* elem){
    return((t_informacion_largo_plazo*) elem)->pid == pid_buscado;
  };

  t_informacion_largo_plazo* info = list_find(lista_info_procesos, tiene_pid_igual);
  if(!info) return false;

  if(enviar_proceso_a_memoria(info->archivo_pseudocodigo, info->tamanio, pcb->pid) == 0){
    encolar_proceso_en_ready(pcb);
    cambiar_estado(pcb, ESTADO_NEW, ESTADO_READY);
    list_remove_by_condition(lista_info_procesos, tiene_pid_igual);
    sem_post(&semaforo_revisar_susp_ready);
    return true;
  };
  return false;
};