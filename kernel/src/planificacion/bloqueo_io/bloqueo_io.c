#include "bloqueo_io.h"

void manejar_respuesta_io(uint32_t pid){
  t_pcb* pcb = obtener_pcb_por_pid(pid);
  if(!pcb){
    log_warning(logger, "PCB no encontrado para el proceso <%d> al finalizar IO", pid);
    return;
  };
  
  if(!pcb->dispositivo_actual){
    log_error(logger, "Proceso <%d> no tiene un dispositivo asignado");
    return;
  };

  t_dispositivo_io* dispositivo = dictionary_get(diccionario_dispositivos, pcb->dispositivo_actual);
  if (!dispositivo){
    log_error(logger, "Dispositivo IO <%s> no encontrado", pcb->dispositivo_actual);
    return;
  };

  pthread_mutex_lock(&mutex_ready);
  queue_push(cola_ready, pcb);
  pthread_mutex_unlock(&mutex_ready);

  cambiar_estado(pcb, ESTADO_BLOCKED, ESTADO_READY);
  log_fin_io(pid);

  if(!queue_is_empty(dispositivo->cola_bloqueados)){
    t_pcb* siguiente = queue_pop(dispositivo->cola_bloqueados);
    enviar_peticion_io(dispositivo->socket, siguiente->duracion_io, siguiente->pid);
    dispositivo->ocupado = true;
  } else {
    dispositivo->ocupado = false;
  };

  mover_proceso_a_exec();
};