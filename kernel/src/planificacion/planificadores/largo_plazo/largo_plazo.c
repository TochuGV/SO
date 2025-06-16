#include "largo_plazo.h"

t_queue* cola_new;
t_list* lista_tamanios;
pthread_mutex_t mutex_new = PTHREAD_MUTEX_INITIALIZER;
sem_t hay_procesos_en_new;

void iniciar_planificacion_largo_plazo(){
  cola_new = queue_create();
  lista_tamanios = list_create();
  sem_init(&hay_procesos_en_new, 0, 0);
  pthread_mutex_init(&mutex_new, NULL);
};

void inicializar_proceso(t_pcb *pcb, uint32_t tamanio){
  pthread_mutex_lock(&mutex_new);
  queue_push(cola_new, pcb);
  list_add(lista_pcbs, pcb);

  t_informacion_largo_plazo* info = malloc(sizeof(t_informacion_largo_plazo));
  info->pid = pcb->pid;
  info->tamanio = tamanio;
  list_add(lista_tamanios, info);

  pthread_mutex_unlock(&mutex_new);
  
  entrar_estado(pcb, ESTADO_NEW);
  log_creacion_proceso(pcb->pid);
};

void mover_proceso_a_ready(char* archivo_pseudocodigo, int32_t tamanio_proceso) {
  if (archivo_pseudocodigo == NULL) {
    archivo_pseudocodigo = "proceso0.txt"; // archivo genérico para pruebas
  };
  
  pthread_mutex_lock(&mutex_new);
  if(queue_is_empty(cola_new)){
    pthread_mutex_unlock(&mutex_new);
    return;
  };

  t_pcb* pcb = NULL;
  if(strcmp(ALGORITMO_INGRESO_A_READY, "PMCP") == 0){
    pcb = elegir_proceso_mas_chico(cola_new, lista_tamanios);
  } else {
    pcb = queue_pop(cola_new);
  };

  pthread_mutex_unlock(&mutex_new);
  if(!pcb) return;

  if(enviar_proceso_a_memoria(archivo_pseudocodigo, tamanio_proceso, pcb->pid, conexion_memoria) == 0) {
    pthread_mutex_lock(&mutex_ready);
    queue_push(cola_ready, pcb); 
    pthread_mutex_unlock(&mutex_ready); 
    cambiar_estado(pcb, ESTADO_NEW, ESTADO_READY);
    sem_post(&semaforo_ready);
  } else {
    cambiar_estado(pcb, ESTADO_NEW, ESTADO_EXIT);
    finalizar_proceso(pcb);
  };
};

void finalizar_proceso(t_pcb* pcb){
  char* clave_pid = string_itoa(pcb->pid);
  t_temporizadores_estado* cronometros_pid = dictionary_get(diccionario_cronometros, clave_pid);
  if(cronometros_pid){
    for(int i = 0; i < CANTIDAD_ESTADOS; i++){
      if(cronometros_pid->cronometros_estado[i]){
        uint32_t tiempo = temporal_gettime(cronometros_pid->cronometros_estado[i]);
        pcb->mt[i] += tiempo;
        temporal_destroy(cronometros_pid->cronometros_estado[i]);
        cronometros_pid->cronometros_estado[i] = NULL;
      };
    };
    dictionary_remove(diccionario_cronometros, clave_pid);
    free(cronometros_pid);
  };
  dictionary_remove_and_destroy(diccionario_contextos_io, clave_pid, destruir_contexto_io);
  free(clave_pid);
  log_fin_proceso(pcb->pid);
  char* buffer = crear_cadena_metricas_estado(pcb);
  log_metricas_estado(buffer);
  free(buffer);

  bool es_pid(void* elem){
    return ((t_informacion_largo_plazo*)elem)->pid == pcb->pid;
  };

  t_informacion_largo_plazo* info = list_remove_by_condition(lista_tamanios, es_pid);
  if(info) free(info);
  destruir_pcb(pcb);
};

