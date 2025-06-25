#include "interrupt.h"

void* conectar_cpu_interrupt(void* arg){
  aceptar_conexiones((char*) arg, atender_cpu_interrupt, "CPU Interrupt");
  return NULL;
};

void* atender_cpu_interrupt(void* arg){
  int socket_cpu_interrupt = *(int*) arg;
  free(arg);

  if(!validar_handshake_cliente(socket_cpu_interrupt, MODULO_CPU, "CPU Interrupt"))
    pthread_exit(NULL);

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