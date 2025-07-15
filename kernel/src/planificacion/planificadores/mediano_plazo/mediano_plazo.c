#include "mediano_plazo.h"

t_list* lista_susp_blocked;
t_queue* cola_susp_ready;
sem_t semaforo_revisar_bloqueados;
sem_t semaforo_revisar_susp_ready;
pthread_mutex_t mutex_susp_blocked = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_susp_ready = PTHREAD_MUTEX_INITIALIZER;
pthread_t hilo_suspended_blocked;
pthread_t hilo_suspended_ready;
t_list* lista_info_procesos_susp;

void iniciar_planificacion_mediano_plazo(void){
  lista_susp_blocked = list_create();
  cola_susp_ready = queue_create();
  lista_info_procesos_susp = list_create();
  sem_init(&semaforo_revisar_bloqueados, 0, 0);
  sem_init(&semaforo_revisar_susp_ready, 0, 0);
  pthread_mutex_init(&mutex_susp_blocked, NULL);
  pthread_mutex_init(&mutex_susp_ready, NULL);
};

void revisar_bloqueados(void) {
  if (dictionary_is_empty(diccionario_contextos_io))
    return;

  t_list* lista_bloqueados = dictionary_keys(diccionario_contextos_io);
  if (!lista_bloqueados)
    return;

  int32_t tiempo_restante_min = INT32_MAX;
  int32_t tiempo_actual;

  for (int i = 0; i < list_size(lista_bloqueados); i++) {
    char* clave_pid = list_get(lista_bloqueados, i);
    t_temporizadores_estado* cronometros_pid = dictionary_get(diccionario_cronometros, clave_pid);

    if (cronometros_pid && cronometros_pid->cronometros_estado[ESTADO_BLOCKED]) {
      tiempo_actual = temporal_gettime(cronometros_pid->cronometros_estado[ESTADO_BLOCKED]);

      if (tiempo_actual > TIEMPO_SUSPENSION) {
        log_warning(logger, "Tiempo: %d > Tiempo suspendido: %d", tiempo_actual, TIEMPO_SUSPENSION);
        uint32_t pid = atoi(clave_pid);
        t_pcb* pcb = obtener_pcb_por_pid(pid);
        suspender_proceso(pcb);
        sem_post(&semaforo_revisar_largo_plazo);

        list_destroy(lista_bloqueados);
        return;
      }

      int32_t restante = TIEMPO_SUSPENSION - tiempo_actual;
      if (restante < tiempo_restante_min)
        tiempo_restante_min = restante;
    }
  }

  list_destroy(lista_bloqueados);

  if (tiempo_restante_min != INT32_MAX && tiempo_restante_min > 0)
    usleep(tiempo_restante_min * 1000);

  sem_post(&semaforo_revisar_bloqueados);
}

void suspender_proceso(t_pcb* pcb){
  pthread_mutex_lock(&mutex_memoria);
  int socket_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA, MODULO_KERNEL);
  if(handshake_kernel(socket_memoria) != 0){
    log_error(logger, "No se pudo conectar a Memoria");
    return;
  }
  t_paquete* paquete = crear_paquete(SUSPENDER);
  agregar_a_paquete(paquete, &(pcb->pid), sizeof(uint32_t));
  enviar_paquete(paquete, socket_memoria);

  int32_t resultado;
  
  recv(socket_memoria, &resultado, sizeof(int32_t), MSG_WAITALL);
  close(socket_memoria);
  pthread_mutex_unlock(&mutex_memoria);

  if(resultado == 0){
    cambiar_estado(pcb, ESTADO_BLOCKED, ESTADO_SUSP_BLOCKED);
    pthread_mutex_lock(&mutex_susp_blocked);
    list_add(lista_susp_blocked, pcb);
    pthread_mutex_unlock(&mutex_susp_blocked);
  };
};

int esta_suspendido(t_pcb* pcb){
  if (list_is_empty(lista_susp_blocked))
    return -1;
  
  for (int i = 0; i < list_size(lista_susp_blocked); i++) {
    t_pcb* pcb_susp = list_get(lista_susp_blocked, i);

    if (pcb_susp->pid == pcb->pid){
      return i;
    }
  }
  return -1;
}

void encolar_proceso_en_susp_ready(t_pcb* pcb){
    pthread_mutex_lock(&mutex_susp_ready);
    queue_push(cola_susp_ready, pcb);
    pthread_mutex_unlock(&mutex_susp_ready);

    cambiar_estado(pcb, ESTADO_SUSP_BLOCKED, ESTADO_SUSP_READY);
}

void desuspender_proceso(void){
  pthread_mutex_lock(&mutex_susp_ready);
  t_pcb* pcb = queue_peek(cola_susp_ready);
  pthread_mutex_unlock(&mutex_susp_ready);

  t_paquete* paquete = crear_paquete(DESUSPENDER);
  agregar_a_paquete(paquete, &(pcb->pid), sizeof(uint32_t));

  if (!pcb)
    return;
  

  pthread_mutex_lock(&mutex_memoria);
  int socket_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA, MODULO_KERNEL);
  if(handshake_kernel(socket_memoria) != 0){
    log_error(logger, "No se pudo conectar a Memoria");
    return;
  }
  enviar_paquete(paquete, socket_memoria);

  int32_t resultado;

  recv(socket_memoria, &resultado, sizeof(int32_t), MSG_WAITALL);
  close(socket_memoria);
  pthread_mutex_unlock(&mutex_memoria);

  if (resultado == 0) {
    encolar_proceso_en_ready(pcb);
    cambiar_estado(pcb, ESTADO_SUSP_READY, ESTADO_READY);
    queue_pop(cola_susp_ready);
  }

  sem_post(&semaforo_revisar_largo_plazo);

  return;
}