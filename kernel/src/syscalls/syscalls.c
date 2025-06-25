#include "syscalls.h"

t_syscall* recibir_syscall(int socket_cliente){
  t_list* lista = recibir_paquete(socket_cliente);
  if(!lista || list_size(lista) < 6){
    log_error(logger, "Error al recibir la llamada al sistema: Lista vacía o incompleta");
    return NULL;
  };

  t_syscall* syscall = malloc(sizeof(t_syscall));
  int offset = 0;

  syscall->pid = *(uint32_t*) list_get(lista, offset++);
  syscall->tipo = *(uint32_t*) list_get(lista, offset++);

  uint32_t arg1_len = *(uint32_t*) list_get(lista, offset++);
  syscall->arg1 = NULL;
  if(arg1_len > 0){
    syscall->arg1 = strdup((char*) list_get(lista, offset++));
  };

  uint32_t arg2_len = *(uint32_t*) list_get(lista, offset++);
  syscall->arg2 = NULL;
  if(arg2_len > 0){
    syscall->arg2 = strdup((char*) list_get(lista, offset++));
  };

  syscall->pc = *(uint32_t*) list_get(lista, offset++);

  list_destroy_and_destroy_elements(lista, free);
  return syscall;
};

void syscall_init_proc(t_syscall* syscall){
  t_pcb* pcb_nuevo = crear_pcb();
  char* archivo_pseudocodigo = strdup(syscall->arg1);
  uint32_t tamanio = atoi(syscall->arg2);
  inicializar_proceso(pcb_nuevo, archivo_pseudocodigo, tamanio);
};

void syscall_exit(t_syscall* syscall){
  t_pcb* pcb = obtener_pcb_por_pid(syscall->pid);
  if(pcb == NULL) return;
  cambiar_estado(pcb, ESTADO_EXEC, ESTADO_EXIT);
  liberar_cpu_por_pid(pcb->pid);
  finalizar_proceso(pcb);
};

void syscall_io(t_syscall* syscall){
  t_pcb* pcb = obtener_pcb_por_pid(syscall->pid);
  if(pcb == NULL){
    log_warning(logger, "No existe el PCB");
    return;
  };
  pcb->pc = syscall->pc;

  char* clave_pid = string_itoa(pcb->pid);
  t_temporizadores_estado* tiempos = dictionary_get(diccionario_cronometros, clave_pid);

  if(tiempos && tiempos->cronometros_estado[ESTADO_EXEC]){
    double rafaga_real = temporal_gettime(tiempos->cronometros_estado[ESTADO_EXEC]) / 1000.0; //Obtiene los segundos
    actualizar_estimacion(pcb->pid, rafaga_real);
    log_debug(logger, "PID <%d> - Ráfaga real: %.2f - Estimación actualizada", pcb->pid, rafaga_real);
  } else {
    log_warning(logger, "No se pudo medir la ráfaga real del proceso <%d>", pcb->pid);
  };

  // Obtener el dispositivo directamente, sin chequeo previo
  t_dispositivo_io* dispositivo = dictionary_get(diccionario_dispositivos, syscall->arg1);
  if(!dispositivo){
    log_error(logger, "Dispositivo IO <%s> no encontrado. Proceso <%d> finalizando...", syscall->arg1, pcb->pid);
    cambiar_estado(pcb, ESTADO_EXEC, ESTADO_EXIT);
    liberar_cpu_por_pid(pcb->pid);
    finalizar_proceso(pcb);
    //finalizar_proceso_por_syscall(pcb);
    return;
  }

  t_contexto_io* contexto = malloc(sizeof(t_contexto_io));
  contexto->dispositivo_actual = strdup(syscall->arg1);
  contexto->duracion_io = atoi(syscall->arg2);
  //char* clave_pid = string_itoa(pcb->pid);
  dictionary_put(diccionario_contextos_io, clave_pid, contexto);
  free(clave_pid);
  cambiar_estado(pcb, ESTADO_EXEC, ESTADO_BLOCKED);

  if(dispositivo->ocupado){
    queue_push(dispositivo->cola_bloqueados, pcb);
    log_debug(logger, "Dispositivo <%s> ocupado. Proceso <%d> encolado", syscall->arg1, pcb->pid);
  } else {
    dispositivo->ocupado = true;
    enviar_peticion_io(dispositivo->socket, contexto->duracion_io, pcb->pid);
    log_debug(logger, "Proceso <%d> enviado al dispositivo <%s>", pcb->pid, syscall->arg1);
  };

  liberar_cpu_por_pid(pcb->pid);
};

void syscall_dump_memory(t_syscall* syscall){ // Se pide un volcado de información de un proceso obtenido por el PID.
  t_pcb* pcb = obtener_pcb_por_pid(syscall->pid);
  if(pcb == NULL){
    log_warning(logger, "No existe el PCB");
    return;
  };
  pcb->pc = syscall->pc;
  log_debug(logger, "Solicitando un volcado de información para el proceso <%d>", pcb->pid);

  int socket_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA, MODULO_KERNEL);
  if(handshake_kernel(socket_memoria) != 0){
    log_error(logger, "No se pudo conectar a Memoria para solicitar DUMP_MEMORY");
    return;
  };

  t_paquete* paquete = crear_paquete(SOLICITUD_DUMP_MEMORY);
  agregar_a_paquete(paquete, &(pcb->pid), sizeof(uint32_t));
  enviar_paquete(paquete, socket_memoria);
  cambiar_estado(pcb, ESTADO_EXEC, ESTADO_BLOCKED);
  liberar_cpu_por_pid(pcb->pid);

  int respuesta = recibir_operacion(socket_memoria);
  close(socket_memoria);
  
  if(respuesta == 0){
    log_info(logger, "Dump de Memoria exitoso para proceso <%d>", pcb->pid);
    encolar_proceso_en_ready(pcb);
    cambiar_estado(pcb, ESTADO_BLOCKED, ESTADO_READY);
    sem_post(&semaforo_ready);
  } else {
    log_error(logger, "Error al realizar dump de Memoria para proceso <%d>", pcb->pid);
    cambiar_estado(pcb, ESTADO_BLOCKED, ESTADO_EXIT);
    finalizar_proceso(pcb);
  };
};

void manejar_syscall(t_syscall* syscall, int socket_cpu_dispatch){
  log_syscall_recibida(syscall->pid, syscall->tipo);
  switch(syscall->tipo){
    case SYSCALL_INIT_PROC:
      syscall_init_proc(syscall);
      break;
    case SYSCALL_EXIT:
      syscall_exit(syscall);
      break;
    case SYSCALL_IO:
      syscall_io(syscall);
      break;
    case SYSCALL_DUMP_MEMORY:
      syscall_dump_memory(syscall);
      break;
    default:
      log_warning(logger, "Syscall desconocida");
  };
};

void destruir_syscall(t_syscall* syscall){
  if(!syscall) return;
  if(syscall->arg1) free(syscall->arg1);
  if(syscall->arg2) free(syscall->arg2);
  free(syscall);
};