#include "bloqueo_io.h"

void manejar_respuesta_io(uint32_t pid){
  t_pcb* pcb = obtener_pcb_por_pid(pid);
  if(!pcb){
    log_warning(logger, "PCB no encontrado para el proceso <%d> al finalizar IO", pid);
    return;
  };
  
  char* clave_pid_actual = string_itoa(pid);
  t_contexto_io* contexto = dictionary_get(diccionario_contextos_io, clave_pid_actual);
  if(!contexto){
    log_error(logger, "Proceso <%d> no tiene un dispositivo asignado", pid);
    free(clave_pid_actual);
    return;
  };
  t_dispositivo_io* dispositivo = dictionary_get(diccionario_dispositivos, contexto->dispositivo_actual);
  if (!dispositivo){
    log_error(logger, "Dispositivo IO <%s> no encontrado", contexto->dispositivo_actual);
    free(clave_pid_actual);
    return;
  };

  int index_susp = esta_suspendido(pcb);

  if(index_susp != -1) {
    list_remove(lista_susp_blocked, index_susp);

    pthread_mutex_lock(&mutex_susp_ready);
    queue_push(cola_susp_ready, pcb);
    pthread_mutex_unlock(&mutex_susp_ready);

    cambiar_estado(pcb, ESTADO_SUSP_BLOCKED, ESTADO_SUSP_READY);
    log_fin_io_susp(pid);

  } else {
    encolar_proceso_en_ready(pcb);
    cambiar_estado(pcb, ESTADO_BLOCKED, ESTADO_READY);
    log_fin_io(pid);
  }



  dictionary_remove(diccionario_contextos_io, clave_pid_actual);
  free(contexto->dispositivo_actual);
  free(contexto);
  free(clave_pid_actual);

  if(!queue_is_empty(dispositivo->cola_bloqueados)){
    t_pcb* siguiente = queue_pop(dispositivo->cola_bloqueados);
    char* clave_pid_siguiente = string_itoa(siguiente->pid);
    t_contexto_io* contexto_siguiente = dictionary_get(diccionario_contextos_io, clave_pid_siguiente);
    if(contexto_siguiente){
      enviar_peticion_io(dispositivo->socket, contexto_siguiente->duracion_io, siguiente->pid);
      dispositivo->ocupado = true;
    } else {
      log_error(logger, "No se encontró el contexto IO para el proceso <%d>", siguiente->pid);
      dispositivo->ocupado = false;
    };
    free(clave_pid_siguiente);
  } else {
    dispositivo->ocupado = false;
  };
};