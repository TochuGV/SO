#include "common_cpu.h"
//Conexiones
void* conexiones_modulos(void* arg) {
  t_cpu_args* args = (t_cpu_args*) arg;

  args-> ip_kernel = config_get_string_value(config, "IP_KERNEL");
  args-> ip_memoria = config_get_string_value(config, "IP_MEMORIA");

  args-> puerto_kernel_dispatch = config_get_string_value(config, "PUERTO_KERNEL_DISPATCH");
  //char* puerto_kernel_interrupt = config_get_string_value(config, "PUERTO_KERNEL_INTERRUPT");
  args-> puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");

  args->socket_dispatch = crear_conexion(args->ip_kernel,args-> puerto_kernel_dispatch, CPU);
  //args->socket_interrupt = crear_conexion(ip_kernel, puerto_kernel_interrupt CPU);
  args->socket_memoria = crear_conexion(args->ip_memoria, args->puerto_memoria, CPU);

  free (args);

  return NULL;
}

//Handshakes
int32_t handshake_cpu(int32_t identificador, int conexion_cpu) {
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

int32_t handshake_memoria(int32_t identificador, int conexion_memoria) {
  int32_t resultado;

  send(conexion_memoria, &identificador, sizeof(int32_t), 0);
  recv(conexion_memoria, &resultado, sizeof(int32_t), MSG_WAITALL);

  if(resultado == -1){
    log_error(logger, "Error: La conexión con Memoria falló. Finalizando conexión...");
    return -1;
  }
  else {
    log_info(logger,"Conexion con Memoria exitosa!");
    return 0;
  }
}

void* manejar_dispatch(t_cpu_args* args) {
  // Paso 1: recibir el PCB desde kernel
  t_pcb* pcb = recibir_pcb(args->socket_dispatch);
  
  // Paso 2: obtener la dirección de memoria de la instrucción
  void* buffer = enviar_pc_a_memoria(pcb, args->socket_memoria);
  // Paso 3: ejecutar instrucción
  //ciclo_de_instruccion(pcb);

  // Paso 5: liberar memoria (si es necesario)
  free(pcb);
  free(buffer);

  return NULL;
}

//Manejo del PCB
t_pcb* recibir_pcb(int socket_dispatch) {
  void* buffer = malloc(sizeof(uint32_t)*2);

  int bytes_recibidos= recv(socket_dispatch,buffer,sizeof(uint32_t)*2,MSG_WAITALL);
  if (bytes_recibidos <= 0) {
    log_info(logger,"Fallo al recibir instrucción");
    free (buffer);
    return NULL;
  }

  t_pcb* pcb = deserializar_pcb(buffer);
  free (buffer);
  return pcb;
}

t_pcb* deserializar_pcb(void* buffer) {
  t_pcb* pcb = malloc(sizeof(t_pcb));
  int offset = 0;

  memcpy(&(pcb->pid), buffer + offset, sizeof(uint32_t));
  offset += sizeof(uint32_t);

  memcpy(&(pcb->pc), buffer + offset, sizeof(uint32_t));
  offset += sizeof(uint32_t);
  
  return pcb;
}

void* enviar_pc_a_memoria(t_pcb* pcb, int conexion_memoria) {
  void* buffer = malloc(sizeof(uint32_t));
  int offset = 0;

  memcpy(buffer + offset, &(pcb->pc), sizeof(uint32_t));

  send(conexion_memoria, buffer, sizeof(uint32_t), 0);

  free (buffer);

  return NULL;
}