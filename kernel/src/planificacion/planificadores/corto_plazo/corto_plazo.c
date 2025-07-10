#include "corto_plazo.h"

t_queue* cola_ready;
sem_t semaforo_ready;
pthread_mutex_t mutex_ready = PTHREAD_MUTEX_INITIALIZER;

void iniciar_planificacion_corto_plazo() {
  cola_ready = queue_create();
  sem_init(&semaforo_ready, 0, 0);
  pthread_mutex_init(&mutex_ready, NULL);
};

void mover_proceso_a_exec(){
  pthread_mutex_lock(&mutex_ready);

  if(queue_is_empty(cola_ready)){
    log_info(logger, "No hay procesos en la cola READY");
    pthread_mutex_unlock(&mutex_ready); 
    return;
  };
  
  pthread_mutex_unlock(&mutex_ready);

  if(strcmp(ALGORITMO_CORTO_PLAZO, "FIFO") == 0){
    mover_proceso_a_exec_fifo();
  } else if(strcmp(ALGORITMO_CORTO_PLAZO, "SJF") == 0){
    mover_proceso_a_exec_sjf();
  } else if(strcmp(ALGORITMO_CORTO_PLAZO, "SRT") == 0){
    mover_proceso_a_exec_srt();
  } else {
    log_error(logger, "Algoritmo de planificación desconocido: %s", ALGORITMO_CORTO_PLAZO);
  };
};

void enviar_a_cpu(t_pcb* pcb, int socket_cpu_dispatch){
  int bytes;
  void* stream = serializar_pcb(pcb, &bytes);
  t_paquete* paquete = crear_paquete(PCB);
  agregar_a_paquete(paquete, stream, bytes);
  enviar_paquete(paquete, socket_cpu_dispatch);
  free(stream);
};