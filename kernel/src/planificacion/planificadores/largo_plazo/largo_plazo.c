#include "largo_plazo.h"

t_queue* cola_new;
t_list* lista_info_procesos;
pthread_mutex_t mutex_new = PTHREAD_MUTEX_INITIALIZER;
//sem_t hay_procesos_en_new;
sem_t semaforo_revisar_largo_plazo;

void iniciar_planificacion_largo_plazo(){
  cola_new = queue_create();
  lista_info_procesos = list_create();
  //sem_init(&hay_procesos_en_new, 0, 0);
  sem_init(&semaforo_revisar_largo_plazo, 0, 0);
  pthread_mutex_init(&mutex_new, NULL);
};

void inicializar_proceso(t_pcb* pcb, char* archivo_pseudocodigo, uint32_t tamanio){
  pthread_mutex_lock(&mutex_new);
  queue_push(cola_new, pcb);
  list_add(lista_pcbs, pcb);

  t_informacion_largo_plazo* info = malloc(sizeof(t_informacion_largo_plazo));
  info->pid = pcb->pid;
  info->archivo_pseudocodigo = archivo_pseudocodigo;
  info->tamanio = tamanio;
  list_add(lista_info_procesos, info);
  pthread_mutex_unlock(&mutex_new);
  
  entrar_estado(pcb, ESTADO_NEW);
  log_creacion_proceso(pcb->pid);
  sem_post(&semaforo_revisar_largo_plazo);
};

bool es_PMCP(void){
  return strcmp(ALGORITMO_INGRESO_A_READY, "PMCP") == 0;
};

void mover_proceso_a_ready(void){
  pthread_mutex_lock(&mutex_new);
  if(queue_is_empty(cola_new)){
    pthread_mutex_unlock(&mutex_new);
    return;
  };

  t_pcb* pcb = es_PMCP() ? elegir_proceso_mas_chico(cola_new, lista_info_procesos) : queue_peek(cola_new);
  if(!pcb){
    pthread_mutex_unlock(&mutex_new);
    return;
  };

  uint32_t pid_buscado = pcb->pid;
  bool tiene_pid_igual(void* elem){
    return((t_informacion_largo_plazo*) elem)->pid == pid_buscado;
  };

  t_informacion_largo_plazo* info = list_find(lista_info_procesos, tiene_pid_igual);

  pthread_mutex_unlock(&mutex_new);

  if(!info){
    log_error(logger, "No se encontró información del proceso <%d>", pcb->pid);
    return;
  };

  if(enviar_proceso_a_memoria(info->archivo_pseudocodigo, info->tamanio, pcb->pid, conexion_memoria) == 0) {
    pthread_mutex_lock(&mutex_ready);
    queue_push(cola_ready, pcb);
    pthread_mutex_unlock(&mutex_ready); 
    cambiar_estado(pcb, ESTADO_NEW, ESTADO_READY);
    sem_post(&semaforo_ready);

    bool es_pid(void* elem){ return ((t_pcb*)elem)->pid == pcb->pid; };

    pthread_mutex_lock(&mutex_new);
    if(es_PMCP()){
      list_remove_by_condition(cola_new->elements, es_pid); //--> Revisar de crear una función para remover el elemento de la cola sin acceder directamente al campo 'elements'
    } else {
      queue_pop(cola_new);
    }
    pthread_mutex_unlock(&mutex_new);
  } else {
    log_info(logger, "Proceso <%d> no pudo entrar a memoria. Sigue en NEW", pcb->pid);
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

  t_paquete* paquete = crear_paquete(FINALIZAR_PROCESO);
  agregar_a_paquete(paquete, &pcb->pid, sizeof(uint32_t));
  enviar_paquete(paquete, conexion_memoria);

  int32_t resultado;
  recv(conexion_memoria, &resultado, sizeof(int32_t), MSG_WAITALL);

  if (resultado == 0) {
    dictionary_remove_and_destroy(diccionario_contextos_io, clave_pid, destruir_contexto_io);
    free(clave_pid);
    log_fin_proceso(pcb->pid); //Agregar en todos los que se vaya a EXIT
    char* buffer = crear_cadena_metricas_estado(pcb);
    log_metricas_estado(buffer);
    free(buffer);

    bool es_pid(void* elem){
      return ((t_informacion_largo_plazo*)elem)->pid == pcb->pid;
    };

    t_informacion_largo_plazo* info = list_remove_by_condition(lista_info_procesos, es_pid);
    if(info) free(info);

    destruir_pcb(pcb);
    sem_post(&semaforo_revisar_largo_plazo);
    return;
  }
  log_error(logger, "Error al eliminar el proceso");
  return;
};