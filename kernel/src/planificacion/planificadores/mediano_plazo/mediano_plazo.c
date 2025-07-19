#include "mediano_plazo.h"

t_list* lista_susp_blocked;
t_queue* cola_susp_ready;
sem_t semaforo_revisar_susp_ready;
pthread_mutex_t mutex_susp_blocked = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_susp_ready = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_info_mediano_plazo = PTHREAD_MUTEX_INITIALIZER;
t_list* lista_info_procesos_susp;

void iniciar_planificacion_mediano_plazo(void){
  lista_susp_blocked = list_create();
  cola_susp_ready = queue_create();
  lista_info_procesos_susp = list_create();
  sem_init(&semaforo_revisar_susp_ready, 0, 0);
  pthread_mutex_init(&mutex_susp_blocked, NULL);
  pthread_mutex_init(&mutex_susp_ready, NULL);
};

void* revisar_bloqueados(void* arg) {
  uint32_t pid = *(uint32_t*)arg;
  free(arg);

  t_pcb* pcb = obtener_pcb_por_pid(pid);
  usleep(TIEMPO_SUSPENSION * 1000);

  suspender_proceso(pcb);
  sem_post(&semaforo_revisar_susp_ready);
  pthread_exit((void*)EXIT_FAILURE);
  return NULL;
}

void suspender_proceso(t_pcb* pcb){
  pthread_mutex_lock(&mutex_memoria);
  int socket_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA, MODULO_KERNEL);
  if(handshake_kernel(socket_memoria) != 0){
    log_error(logger, "No se pudo conectar a Memoria");
    close(socket_memoria);
    pthread_mutex_unlock(&mutex_memoria);
    return;
  }
  t_paquete* paquete = crear_paquete(SUSPENDER);
  agregar_a_paquete(paquete, &(pcb->pid), sizeof(uint32_t));
  enviar_paquete(paquete, socket_memoria);
  close(socket_memoria);
  pthread_mutex_unlock(&mutex_memoria);

  pthread_mutex_lock(&mutex_susp_blocked);
  list_add(lista_susp_blocked, pcb);
  pthread_mutex_unlock(&mutex_susp_blocked);
  cambiar_estado(pcb, ESTADO_BLOCKED, ESTADO_SUSP_BLOCKED);
};

int esta_suspendido(t_pcb* pcb){
  pthread_mutex_lock(&mutex_susp_blocked);
  if (list_is_empty(lista_susp_blocked)) {
    pthread_mutex_unlock(&mutex_susp_blocked);
    return -1;
  }
  
  for (int i = 0; i < list_size(lista_susp_blocked); i++) {
    t_pcb* pcb_susp = list_get(lista_susp_blocked, i);

    if (pcb_susp->pid == pcb->pid){
      pthread_mutex_unlock(&mutex_susp_blocked);
      return i;
    }
  }
  pthread_mutex_unlock(&mutex_susp_blocked);
  return -1;
}

void encolar_proceso_en_susp_ready(t_pcb* pcb){
  pthread_mutex_lock(&mutex_susp_ready);
  queue_push(cola_susp_ready, pcb);
  pthread_mutex_unlock(&mutex_susp_ready);
  cambiar_estado(pcb, ESTADO_SUSP_BLOCKED, ESTADO_SUSP_READY);
}
/*
t_pcb* elegir_proceso_mas_chico_susp(){
  if(queue_is_empty(cola_susp_ready) || list_is_empty(lista_info_procesos_susp)) return NULL;
  t_informacion_mediano_plazo* candidato = NULL;
  log_warning(logger, "ENTRE");
  log_warning(logger, "%d", lista_info_procesos_susp->elements_count);
  for(int i = 0; i < list_size(lista_info_procesos_susp); i++){
    t_informacion_mediano_plazo* info_susp = list_get(lista_info_procesos_susp, i);
    if((candidato == NULL || info_susp->tamanio < candidato->tamanio) && info_susp->tamanio > 0){
      candidato = info_susp;
    };
  };
  log_warning(logger, "%d", candidato->pid);
  if(candidato == NULL) return NULL;

  t_pcb* pcb_seleccionado = NULL;
  t_queue* nueva_cola_susp_ready = queue_create();

  while(!queue_is_empty(cola_susp_ready)){
    log_warning(logger, "ELEMENTOS EN COLA SUSP READY: %d", cola_susp_ready->elements->elements_count);
    t_pcb* pcb = queue_pop(cola_susp_ready);
    if(pcb->pid == candidato->pid){
      pcb_seleccionado = pcb;
    } else {
      queue_push(nueva_cola_susp_ready, pcb);
    };
  };

  while(!queue_is_empty(nueva_cola_susp_ready)){
    queue_push(cola_susp_ready, queue_pop(nueva_cola_susp_ready));
  };
  queue_destroy(nueva_cola_susp_ready);

  return pcb_seleccionado;
};*/

void desuspender_proceso(void){
  pthread_mutex_lock(&mutex_susp_ready);
  if(queue_is_empty(cola_susp_ready)) {
    pthread_mutex_unlock(&mutex_susp_ready);
    sem_post(&semaforo_revisar_largo_plazo);
    return;
  }
  if (es_PMCP())
    reordenar_cola_susp_ready_pmcp(cola_susp_ready);

  t_pcb*  pcb = queue_peek(cola_susp_ready);
  pthread_mutex_unlock(&mutex_susp_ready);

  if (!pcb)
    return;

  t_paquete* paquete = crear_paquete(DESUSPENDER);
  agregar_a_paquete(paquete, &(pcb->pid), sizeof(uint32_t));
  
  pthread_mutex_lock(&mutex_memoria);
  int socket_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA, MODULO_KERNEL);
  if(handshake_kernel(socket_memoria) != 0){
    log_error(logger, "No se pudo conectar a Memoria");
    close(socket_memoria);
    pthread_mutex_unlock(&mutex_memoria);
    return;
  }
  enviar_paquete(paquete, socket_memoria);

  int32_t resultado;

  recv(socket_memoria, &resultado, sizeof(int32_t), MSG_WAITALL);
  close(socket_memoria);
  pthread_mutex_unlock(&mutex_memoria);

  if (resultado == 0) {
    pthread_mutex_lock(&mutex_susp_ready);
    queue_pop(cola_susp_ready);
    pthread_mutex_unlock(&mutex_susp_ready);
    encolar_proceso_en_ready(pcb);
    cambiar_estado(pcb, ESTADO_SUSP_READY, ESTADO_READY);

    pthread_mutex_lock(&mutex_susp_ready);
    if(!queue_is_empty(cola_susp_ready)){
      sem_post(&semaforo_revisar_susp_ready);
      pthread_mutex_unlock(&mutex_susp_ready);
      return;
    };
    pthread_mutex_unlock(&mutex_susp_ready);
    sem_post(&semaforo_revisar_largo_plazo);
  };
  return;
};

void reordenar_cola_susp_ready_pmcp(t_queue* cola_susp_ready) {
  if (queue_is_empty(cola_susp_ready)) return;
  t_list* lista_susp_ready = cola_susp_ready->elements;
  list_sort(lista_susp_ready, (void*) comparar_tamanios);
};

bool comparar_tamanios(t_pcb* pcb1, t_pcb* pcb2) {
  uint32_t tamanio1 = obtener_tamanio(pcb1->pid);
  uint32_t tamanio2 = obtener_tamanio(pcb2->pid);
  return tamanio1 < tamanio2;
};

uint32_t obtener_tamanio(uint32_t pid) {
  bool tiene_pid_igual(void* elem){
    return((t_informacion_mediano_plazo*) elem)->pid == pid;
  };

  t_informacion_mediano_plazo* info = list_find(lista_info_procesos_susp, tiene_pid_igual);

  return info->tamanio;
}