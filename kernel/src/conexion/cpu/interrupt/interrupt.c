#include "interrupt.h"

void* conectar_cpu_interrupt(void* arg){
  char* puerto = (char*) arg;
  int socket_escucha = iniciar_servidor(puerto);
  log_info(logger, "Esperando conexiones de CPU Interrupt...");
  while(1){
    int socket_cliente = esperar_cliente(socket_escucha);
    if(socket_cliente == -1){
      log_error(logger, "Error al aceptar conexión desde CPU Interrupt");
      continue;
    };
    int* socket_ptr = malloc(sizeof(int));
    *socket_ptr = socket_cliente;
    pthread_t hilo_atender_cpu_interrupt;
    pthread_create(&hilo_atender_cpu_interrupt, NULL, atender_cpu_interrupt, socket_ptr);
    pthread_detach(hilo_atender_cpu_interrupt);
  };
  return NULL;
};

void* atender_cpu_interrupt(void* arg){
  int socket_cpu_interrupt = *(int*) arg;
  free(arg);
  if(recibir_handshake_kernel(socket_cpu_interrupt) != MODULO_CPU){
    log_error(logger, "Se esperaba un CPU, pero se conectó otro tipo");
    close(socket_cpu_interrupt);
    pthread_exit(NULL);
  };
  log_info(logger, "CPU Interrupt conectada correctamente");
  while(1){
    int cod_op = recibir_operacion(socket_cpu_interrupt);
    if(cod_op == -1){
      log_warning(logger, "CPU Interrupt desconectada");
      break;
    };
    log_debug(logger, "Se recibió la interrupción. Código: %d", cod_op);
  };
  close(socket_cpu_interrupt);
  return NULL;
};