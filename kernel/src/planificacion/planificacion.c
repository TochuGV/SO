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
  //log_info(logger, "## (<%d>) Se crea el proceso - Estado: NEW", pcb->pid);
  log_creacion_proceso(pcb->pid);
};

void cambiar_estado(t_pcb* pcb, t_estado actual, t_estado siguiente){
  entrar_estado(pcb, siguiente);
  //log_info(logger, "## (<%d>) Pasa del estado <%s> al estado <%s>", pcb->pid, obtener_nombre_estado(actual), obtener_nombre_estado(siguiente));
  log_cambio_estado(pcb->pid, obtener_nombre_estado(actual), obtener_nombre_estado(siguiente));
};

void finalizar_proceso(t_pcb* pcb){
  //cambiar_estado(pcb, ESTADO_EXEC, ESTADO_EXIT);
  //Revisar que de cualquier estado puede pasar a EXIT.
  //Revisar si tiene sentido los campos 'me' y 'mt' para EXIT.
  
  //log_info(logger, "## (<%d>) - Finaliza el proceso", pcb->pid);
  log_fin_proceso(pcb->pid);

  char* buffer = crear_cadena_metricas_estado(pcb);
  //log_info(logger, "%s", buffer);
  log_metricas_estado(buffer);
  free(buffer);
  destruir_pcb(pcb);
};

/*
void mover_proceso_a_ready(char* archivo_pseudocodigo, int32_t tamanio_proceso) {
  t_pcb* pcb;
  pcb = queue_pop(cola_new);
  //if (enviar_proceso_a_memoria(archivo_pseudocodigo, tamanio_proceso, pcb->pid, conexion_memoria) == 0) {
    pthread_mutex_lock(&mutex_ready);
    queue_push(cola_ready, pcb); 
    pthread_mutex_unlock(&mutex_ready); 
    cambiar_estado(pcb, ESTADO_NEW, ESTADO_READY);
  //}
  //return NULL;
}
*/

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