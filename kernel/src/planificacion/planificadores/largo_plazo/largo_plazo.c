#include "largo_plazo.h"

t_queue* cola_new;
pthread_mutex_t mutex_new = PTHREAD_MUTEX_INITIALIZER;
sem_t hay_procesos_en_new;

void iniciar_planificacion_largo_plazo(){
  cola_new = queue_create();
  sem_init(&hay_procesos_en_new, 0, 0);
  pthread_mutex_init(&mutex_new, NULL); 
};

void inicializar_proceso(t_pcb *pcb){
  queue_push(cola_new, pcb);
  entrar_estado(pcb, ESTADO_NEW);
  log_creacion_proceso(pcb->pid);
};

void mover_proceso_a_ready(char* archivo_pseudocodigo, int32_t tamanio_proceso) {
  pthread_mutex_lock(&mutex_new);
  if(queue_is_empty(cola_new)){
    pthread_mutex_unlock(&mutex_new);
    return;
  };

  t_pcb* pcb = queue_pop(cola_new);
  pthread_mutex_unlock(&mutex_new);
  if(enviar_proceso_a_memoria(archivo_pseudocodigo, tamanio_proceso, pcb->pid, conexion_memoria) == 0) {
    pthread_mutex_lock(&mutex_ready);
    queue_push(cola_ready, pcb); 
    pthread_mutex_unlock(&mutex_ready); 
    cambiar_estado(pcb, ESTADO_NEW, ESTADO_READY);
  } else {
    cambiar_estado(pcb, ESTADO_NEW, ESTADO_EXIT); //Por ahora queda así
    finalizar_proceso(pcb);
  };
};