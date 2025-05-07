#include "common_cpu.h"

int32_t handshake_cpu(int32_t identificador, int conexion_cpu){
  int32_t resultado;

  send(conexion_cpu, &identificador, sizeof(int32_t), 0);
  recv(conexion_cpu, &resultado, sizeof(int32_t), MSG_WAITALL);

  if(resultado == -1){
    log_error(logger, "Error: La conexión con Kernel falló. Finalizando conexión...");
    return -1;
  }
  else {
    log_info(logger,"Conexion con Kernel exitosa!");
    return 0;
  }
}

t_pcb* recibir_pcb(int puerto_kernel_dispatch){
  void* buffer = malloc(sizeof(uint32_t)*2);

  int bytes_recibidos= recv(puerto_kernel_dispatch,buffer,sizeof(uint32_t)*2,MSG_WAITALL);
  if (bytes_recibidos <= 0) {
    log_info(logger,"Fallo al recibir instrucción")
    free (buffer);
    return NULL;
  }

  t_pcb* pcb = deserializar_pcb(buffer);
  free (buffer);
  return pcb;
}

t_pcb* deserializar_pcb(buffer) {
  t_pcb* pcb = malloc(sizeof(t_pcb));
  int offset = 0;

  memcpy(&(pcb->pid), buffer + offset, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  memcpy(&(pcb->pc), buffer + offset, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  pcb->me = NULL;
  pcb->mt = NULL;

  return pcb;
}

void* manejar_dispatch(void* arg) {
  int dispatch_socket = *(int*)arg;

  // Paso 1: recibir el PCB desde kernel
  t_pcb* pcb = recibir_pcb(dispatch_socket);

  // Paso 2: ejecutar ciclo de instrucción
  ciclo_de_instruccion(pcb);

  // Paso 3: liberar memoria (si es necesario)
  free(pcb);

  return NULL;
}
