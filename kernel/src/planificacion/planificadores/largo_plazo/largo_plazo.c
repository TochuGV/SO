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

  inicializar_estimacion_rafaga(pcb->pid);
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

  if(es_PMCP()){
    mover_proceso_a_ready_pmcp();
  } else {
    mover_proceso_a_ready_fifo();
  };
  
  pthread_mutex_unlock(&mutex_new);
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

  pthread_mutex_lock(&mutex_memoria);
  int socket_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA, MODULO_KERNEL);
  if (handshake_kernel(socket_memoria) != 0) {
    log_error(logger, "No se pudo conectar a Memoria para finalizar el proceso <%d>", pcb->pid);
    return;
  };

  enviar_paquete(paquete, socket_memoria);

  int32_t resultado;
  if (recv(socket_memoria, &resultado, sizeof(int32_t), MSG_WAITALL) <= 0) {
    log_error(logger, "Error al recibir confirmación de finalización de Memoria");
    close(socket_memoria);
    return;
  };

  close(socket_memoria);
  pthread_mutex_unlock(&mutex_memoria);
  
  if (resultado == 0) {
    dictionary_remove_and_destroy(diccionario_contextos_io, clave_pid, destruir_contexto_io);

    //eliminamos la estimacion del proceso terminado
    char* clave_estimacion = string_itoa(pcb->pid);
    dictionary_remove_and_destroy(diccionario_estimaciones, clave_estimacion, free);
    free(clave_estimacion);

    free(clave_pid);
    log_fin_proceso(pcb->pid); //Agregar en todos los que se vaya a EXIT
    char* buffer = crear_cadena_metricas_estado(pcb);
    log_metricas_estado(buffer);
    free(buffer);

    bool es_pid(void* elem){
      return ((t_informacion_largo_plazo*)elem)->pid == pcb->pid;
    };

    pthread_mutex_lock(&mutex_pcbs);
    list_remove_element(lista_pcbs, pcb);
    pthread_mutex_unlock(&mutex_pcbs);

    t_informacion_largo_plazo* info = list_remove_by_condition(lista_info_procesos, es_pid);
    if(info) free(info);

    destruir_pcb(pcb);
    sem_post(&semaforo_revisar_susp_ready);
      
    return;
  };
  log_error(logger, "Error al eliminar el proceso");
  return;
};