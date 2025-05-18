#include "planificacion.h"

t_queue* cola_new;
//t_queue* cola_ready;

//pthread_mutex_t mutex_new = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t mutex_ready = PTHREAD_MUTEX_INITIALIZER;
//sem_t hay_procesos_en_new;

void iniciar_planificacion_largo_plazo(void* args){
  cola_new = queue_create();
};

void agregar_proceso_a_new(t_pcb *pcb){
  queue_push(cola_new, pcb);
  log_info(logger, "## (<%d>) Se crea el proceso - Estado: NEW", pcb->pid);
};

void finalizar_proceso(t_pcb* pcb){
  log_info(logger, "## (<%d>) - Finaliza el proceso", pcb->pid);
  //Métricas de estado...
  destruir_pcb(pcb);
};