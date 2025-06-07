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
  mover_proceso_a_ready(syscall->arg1, atoi(syscall->arg2));
};

void syscall_exit(t_syscall* syscall){
  t_pcb* pcb = obtener_pcb_por_pid(syscall->pid);
  if(pcb == NULL) return;
  cambiar_estado(pcb, ESTADO_EXEC, ESTADO_EXIT);
  finalizar_proceso(pcb);
  //Liberar la memoria
  liberar_cpu_por_pid(pcb->pid);
};

void syscall_io(t_syscall* syscall){
  t_pcb* pcb = obtener_pcb_por_pid(syscall->pid);
  if(pcb == NULL) return;

  nombre_dispositivo_io dispositivo = obtener_dispositivo_io(syscall->arg1);
  if(dispositivo == -1){
    cambiar_estado(pcb, ESTADO_EXEC, ESTADO_EXIT);
    finalizar_proceso(pcb);
    return;
  };

  cambiar_estado(pcb, ESTADO_EXEC, ESTADO_BLOCKED);
  log_motivo_bloqueo(pcb->pid, dispositivo);

  //Revisar si algún proceso ya estaba en IO
};

void syscall_dump_memory(t_syscall* syscall){
  t_pcb* pcb = obtener_pcb_por_pid(syscall->pid);
  if(!pcb) return;

  //Enviar a Memoria una orden de DUMP con el PID
  //Bloquear el proceso hasta recibir respuesta (o usar una estructura que lo maneje)

  //Supongamos que recibís OK:
  cambiar_estado(pcb, ESTADO_EXEC, ESTADO_READY);
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