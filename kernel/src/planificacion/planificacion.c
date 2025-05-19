#include "planificacion.h"
#include <commons/temporal.h>

t_queue* cola_new;
t_queue* cola_ready;

pthread_mutex_t mutex_new = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_ready = PTHREAD_MUTEX_INITIALIZER;

sem_t hay_procesos_en_new;

void iniciar_planificacion_largo_plazo(void* args){
  cola_new = queue_create();
  sem_init(&hay_procesos_en_new, 0, 0);
  pthread_mutex_init(&mutex_new, NULL); 
};

void iniciar_planificacion_corto_plazo() {
  cola_ready = queue_create();
  pthread_mutex_init(&mutex_ready, NULL);
}

void entrar_estado(t_pcb* pcb, int estado){
  //t_temporal* cronometro1 = temporal_create();
  pcb->me[estado]++;
};

void inicializar_proceso(t_pcb *pcb){
  queue_push(cola_new, pcb);
  entrar_estado(pcb, ESTADO_NEW);
  log_info(logger, "## (<%d>) Se crea el proceso - Estado: NEW", pcb->pid);
};

void cambiar_estado(t_pcb* pcb, t_estado actual, t_estado siguiente){
  log_info(logger, "## (<%d>) Pasa del estado <%s> al estado <%s>", pcb->pid, obtener_nombre_estado(actual), obtener_nombre_estado(siguiente));
  entrar_estado(pcb, siguiente);
};

void finalizar_proceso(t_pcb* pcb){
  log_info(logger, "## (<%d>) - Finaliza el proceso", pcb->pid);

  char* buffer = string_from_format("## (<%d>) - Métricas de estado: ", pcb->pid);
  for(int i = 0; i < CANTIDAD_ESTADOS; i++){
    char* aux = string_from_format("%s (%d) (%d)", obtener_nombre_estado(i), pcb->me[i], pcb->mt[i]);
    string_append(&buffer, aux);
    if(i < CANTIDAD_ESTADOS - 1) string_append(&buffer, ", ");
    free(aux);
  };

  log_info(logger, "%s", buffer);
  free(buffer);
  destruir_pcb(pcb);
};

void mover_proceso_a_ready(t_pcb* pcb) {
  pthread_mutex_lock(&mutex_ready); 
  queue_push(cola_ready, pcb); 
  pthread_mutex_unlock(&mutex_ready); 
  cambiar_estado(pcb, ESTADO_NEW, ESTADO_READY);
};

t_pcb* mover_proceso_a_exec() {
  pthread_mutex_lock(&mutex_ready); 

  if (queue_is_empty(cola_ready)) { 
    pthread_mutex_unlock(&mutex_ready); 
    log_info(logger, "No hay procesos en la cola READY"); 
    return NULL;
  };
  
  t_pcb* pcb = queue_pop(cola_ready);
  pthread_mutex_unlock(&mutex_ready);
  cambiar_estado(pcb, ESTADO_READY, ESTADO_EXEC);
  return pcb;
};