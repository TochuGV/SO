#include "dispatch.h"

void* conectar_cpu_dispatch(void* arg){
  char* puerto = (char*) arg;
  int socket_escucha = iniciar_servidor(puerto);
  log_info(logger, "Esperando conexiones de CPU Dispatch...");
  while(1){
    int socket_cliente = esperar_cliente(socket_escucha);
    if(socket_cliente == -1){
      log_error(logger, "Error al aceptar conexión desde CPU Dispatch");
      continue;
    };
    int* socket_ptr = malloc(sizeof(int));
    *socket_ptr = socket_cliente;
    pthread_t hilo_atender_cpu_dispatch;
    pthread_create(&hilo_atender_cpu_dispatch, NULL, atender_cpu_dispatch, socket_ptr);
    pthread_detach(hilo_atender_cpu_dispatch);
  };
  return NULL;
};

void* atender_cpu_dispatch(void* arg){
  int socket_cpu_dispatch = *(int*) arg;
  free(arg);
  if(recibir_handshake_kernel(socket_cpu_dispatch) != MODULO_CPU){
    log_error(logger, "Se esperaba un CPU, pero se conectó otro tipo");
    close(socket_cpu_dispatch);
    pthread_exit(NULL);
  };
  log_info(logger, "CPU Dispatch conectada correctamente");
  while(1){
    int cod_op = recibir_operacion(socket_cpu_dispatch);
    if(cod_op == -1){
      log_warning(logger, "CPU Dispatch desconectada");
      break;
    };
    log_debug(logger, "Código de operación recibido: %d", cod_op);
    
    t_syscall* syscall = recibir_syscall(socket_cpu_dispatch);
    log_debug(logger, "PID: %d", syscall->pid);
    log_debug(logger, "TIPO: %d", syscall->tipo);
    log_debug(logger, "ARG1: %s", syscall->arg1);
    log_debug(logger, "ARG2: %s", syscall->arg2);
    log_debug(logger, "PC: %d", syscall->pc);
    manejar_syscall(syscall, socket_cpu_dispatch);
    destruir_syscall(syscall);
  };
  close(socket_cpu_dispatch);
  return NULL;
};