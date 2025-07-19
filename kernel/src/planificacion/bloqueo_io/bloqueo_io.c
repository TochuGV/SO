#include "bloqueo_io.h"

void manejar_respuesta_io(uint32_t pid, int socket_io){
  t_pcb* pcb = obtener_pcb_por_pid(pid);
  if(!pcb){
    return;
  };
  
  char* clave_pid_actual = string_itoa(pid);

  pthread_mutex_lock(&mutex_diccionario_contextos);
  t_contexto_io* contexto = dictionary_get(diccionario_contextos_io, clave_pid_actual);
  pthread_mutex_unlock(&mutex_diccionario_contextos);

  if(!contexto){
    log_error(logger, "Proceso <%d> no tiene un dispositivo asignado", pid);
    free(clave_pid_actual);
    return;
  };

  pthread_mutex_lock(&mutex_diccionario_dispositivos);
  t_list* lista_instancias = dictionary_get(diccionario_dispositivos, contexto->dispositivo_actual);
  pthread_mutex_unlock(&mutex_diccionario_dispositivos);

  if(!lista_instancias || list_size(lista_instancias) == 0){
    log_error(logger, "No se encontraron instancias para el dispositivo <%s>", contexto->dispositivo_actual);
    free(clave_pid_actual);
    return;
  };

  t_instancia_io* instancia_ocupada = NULL;
  for(int i = 0; i < list_size(lista_instancias); i++){
    t_instancia_io* instancia = list_get(lista_instancias, i);
    if(instancia->ocupado && instancia->socket == socket_io){
      instancia_ocupada = instancia;
      break;
    };
  };

  if(!instancia_ocupada){
    free(clave_pid_actual);
    return;
  }

  int index_susp = esta_suspendido(pcb);

  if(index_susp != -1) {
    pthread_mutex_lock(&mutex_susp_blocked);
    list_remove(lista_susp_blocked, index_susp);
    pthread_mutex_unlock(&mutex_susp_blocked);
    encolar_proceso_en_susp_ready(pcb);
    sem_post(&semaforo_revisar_susp_ready);
    log_fin_io(pid, ESTADO_SUSP_READY);
  } else {
    encolar_proceso_en_ready(pcb);
    cambiar_estado(pcb, ESTADO_BLOCKED, ESTADO_READY);
    log_fin_io(pid, ESTADO_READY);
  }
  pthread_mutex_lock(&mutex_diccionario_contextos);
  dictionary_remove(diccionario_contextos_io, clave_pid_actual);
  pthread_mutex_unlock(&mutex_diccionario_contextos);
  free(contexto->dispositivo_actual);
  free(contexto);
  free(clave_pid_actual);

  if(!queue_is_empty(instancia_ocupada->cola_bloqueados)){
    t_pcb* siguiente = queue_pop(instancia_ocupada->cola_bloqueados);
    char* clave_pid_siguiente = string_itoa(siguiente->pid);
    pthread_mutex_lock(&mutex_diccionario_contextos);
    t_contexto_io* contexto_siguiente = dictionary_get(diccionario_contextos_io, clave_pid_siguiente);
    pthread_mutex_unlock(&mutex_diccionario_contextos);
    if(contexto_siguiente){
      enviar_peticion_io(instancia_ocupada->socket, contexto_siguiente->duracion_io, siguiente->pid);
      instancia_ocupada->ocupado = true;
    } else {
      instancia_ocupada->ocupado = false;
    };
    free(clave_pid_siguiente);
  } else {
    instancia_ocupada->ocupado = false;
  };
};