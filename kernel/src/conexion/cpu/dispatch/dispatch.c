#include "dispatch.h"

void* conectar_cpu_dispatch(void* arg){
  aceptar_conexiones((char*) arg, atender_cpu_dispatch, "CPU Dispatch");
  return NULL;
};

void* atender_cpu_dispatch(void* arg){
  int socket_cpu_dispatch = *(int*) arg;
  free(arg);

  if(!validar_handshake_cliente(socket_cpu_dispatch, MODULO_CPU, "CPU Dispatch"))
    pthread_exit(NULL);

  while(1){
    int cod_op = recibir_operacion(socket_cpu_dispatch);
    if(cod_op == -1){
      log_warning(logger, "CPU Dispatch desconectada");
      break;
    };
    switch(cod_op){
      case SYSCALL:
        t_syscall* syscall = recibir_syscall(socket_cpu_dispatch);
        if(!syscall){
          log_error(logger, "Fallo al recibir la llamada al sistema. Cerrando la conexión con CPU Dispatch...");
          break;
        }
        manejar_syscall(syscall, socket_cpu_dispatch);
        destruir_syscall(syscall);
        break;
      
      case DESALOJO:
        t_list* lista = recibir_paquete(socket_cpu_dispatch);
        if(!lista || list_size(lista) < 2){
          log_error(logger, "Fallo al recibir PCB desalojado");
          break;
        } else {
          uint32_t pid = *(uint32_t*) list_get(lista, 0);
          uint32_t pc = *(uint32_t*) list_get(lista, 1);

          list_destroy_and_destroy_elements(lista, free);
          
          log_desalojo_sjf_srt(pid);

          t_pcb* pcb = obtener_pcb_por_pid(pid);
          pcb->pc = pc;

          pthread_mutex_lock(&mutex_ready);
          queue_push(cola_ready, pcb);
          pthread_mutex_unlock(&mutex_ready);
          sem_post(&semaforo_ready);

          cambiar_estado(pcb, ESTADO_EXEC, ESTADO_READY);

          liberar_cpu_por_pid(pcb->pid);
        };
          break;
      default:
        log_warning(logger, "Operación desconocida desde CPU Dispatch: %d", cod_op);
        break;
    };
  };
  close(socket_cpu_dispatch);
  return NULL;
};