#include "bloqueo_io.h"

void manejar_respuesta_io(uint32_t pid, int socket_io){
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

  t_list* lista_instancias = dictionary_get(diccionario_dispositivos, contexto->dispositivo_actual);
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
    log_error(logger, "No se encontró ninguna instancia ocupada para el dispositivo <%s>", contexto->dispositivo_actual);
    free(clave_pid_actual);
    return;
  }

  int index_susp = esta_suspendido(pcb);

  if(index_susp != -1) {
    list_remove(lista_susp_blocked, index_susp);
    encolar_proceso_en_susp_ready(pcb);
    sem_post(&semaforo_revisar_susp_ready);

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

  if(!queue_is_empty(instancia_ocupada->cola_bloqueados)){
    t_pcb* siguiente = queue_pop(instancia_ocupada->cola_bloqueados);
    char* clave_pid_siguiente = string_itoa(siguiente->pid);
    t_contexto_io* contexto_siguiente = dictionary_get(diccionario_contextos_io, clave_pid_siguiente);
    if(contexto_siguiente){
      enviar_peticion_io(instancia_ocupada->socket, contexto_siguiente->duracion_io, siguiente->pid);
      instancia_ocupada->ocupado = true;
    } else {
      log_error(logger, "No se encontró el contexto IO para el proceso <%d>", siguiente->pid);
      instancia_ocupada->ocupado = false;
    };
    free(clave_pid_siguiente);
  } else {
    instancia_ocupada->ocupado = false;
  };
};