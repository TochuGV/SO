#include "syscalls.h"

t_syscall* recibir_syscall(int socket_cliente){
  t_list* lista = recibir_paquete(socket_cliente);
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
  inicializar_proceso(pcb_nuevo);
  mover_proceso_a_ready(syscall->arg1, atoi(syscall->arg2)); //En el futuro, intentar_ingresar_procesos_a_ready()
};

void syscall_exit(t_syscall* syscall){
  t_pcb* pcb = obtener_pcb_por_pid(syscall->pid);
  if(pcb == NULL) return;
  cambiar_estado(pcb, ESTADO_EXEC, ESTADO_EXIT);
  liberar_cpu_por_pid(pcb->pid);
  finalizar_proceso(pcb);
  mover_proceso_a_exec(); //REVISAR - Justo después de que se libere la CPU, tendría que entrar otro
};

void syscall_io(t_syscall* syscall){
  printf("Hola");
  t_pcb* pcb = obtener_pcb_por_pid(syscall->pid);
  if(pcb == NULL){
    log_warning(logger, "No existe el PCB");
    return;
  }

  // Obtener el dispositivo directamente, sin chequeo previo
  t_dispositivo_io* dispositivo = dictionary_get(diccionario_dispositivos, syscall->arg1);
    
  // Si el dispositivo no existe, finaliza el proceso
  if(!dispositivo){
    log_error(logger, "Dispositivo IO <%s> no encontrado. Proceso <%d> finalizando...", syscall->arg1, pcb->pid);
    cambiar_estado(pcb, ESTADO_EXEC, ESTADO_EXIT);
    //finalizar_proceso_por_syscall(pcb);
    return;
  }

  // Mover el proceso a bloqueado antes de evaluar disponibilidad
  pcb->dispositivo_actual = strdup(syscall->arg1);
  pcb->duracion_io = atoi(syscall->arg2);
  cambiar_estado(pcb, ESTADO_EXEC, ESTADO_BLOCKED);

  if(dispositivo->ocupado){
    queue_push(dispositivo->cola_bloqueados, pcb);
    log_debug(logger, "Dispositivo <%s> ocupado. Proceso <%d> encolado", syscall->arg1, pcb->pid);
  } else {
    dispositivo->ocupado = true;
    enviar_peticion_io(dispositivo->socket, atoi(syscall->arg2), pcb->pid);
    log_debug(logger, "Proceso <%d> enviado al dispositivo <%s>", pcb->pid, syscall->arg1);
  };

  liberar_cpu_por_pid(pcb->pid);
  mover_proceso_a_exec();
};

void syscall_dump_memory(t_syscall* syscall){ // Se pide un volcado de información de un proceso obtenido por el PID.
  t_pcb* pcb = obtener_pcb_por_pid(syscall->pid);
  if(!pcb) return;
  log_debug(logger, "Solicitando volcado de información para el proceso <%d>", pcb->pid);

  t_paquete* paquete = crear_paquete(SOLICITUD_DUMP_MEMORY);
  agregar_a_paquete(paquete, &(pcb->pid), sizeof(uint32_t));
  enviar_paquete(paquete, conexion_memoria);
  cambiar_estado(pcb, ESTADO_EXEC, ESTADO_BLOCKED);

  // Recibir la respuesta de Memoria
  int respuesta = recibir_operacion(conexion_memoria);
  if(respuesta == 0){ //Supongamos que recibís OK, pasa el estado a READY despues de hacer el dump
    log_info(logger, "Dump de Memoria exitoso para proceso <%d>", pcb->pid);
    cambiar_estado(pcb, ESTADO_BLOCKED, ESTADO_READY);
  } else { // Si no se puede hacere el dump, el proceso se finaliza.
    log_error(logger, "Error al realizar dump de Memoria para proceso <%d>", pcb->pid);
    cambiar_estado(pcb, ESTADO_BLOCKED, ESTADO_EXIT);
    finalizar_proceso(pcb);
  };
  eliminar_paquete(paquete);
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