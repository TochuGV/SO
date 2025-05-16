#include "common_cpu.h"

 char* ip_kernel;
 char* ip_memoria;
 char* puerto_kernel_dispatch;
 char* puerto_kernel_interrupt;
 char* puerto_memoria;
 datos_conexion_t* datos_dispatch;
 datos_conexion_t* datos_interrupt;
 datos_conexion_t* datos_memoria;

void iniciar_cpu() {

  ip_kernel = config_get_string_value(config, "IP_KERNEL");
  ip_memoria = config_get_string_value(config, "IP_MEMORIA");

  puerto_kernel_dispatch = config_get_string_value(config, "PUERTO_KERNEL_DISPATCH");
  puerto_kernel_interrupt = config_get_string_value(config, "PUERTO_KERNEL_INTERRUPT");
  puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");

  datos_dispatch = malloc(sizeof(datos_conexion_t));
  datos_dispatch->ip = ip_kernel;
  datos_dispatch->puerto = puerto_kernel_dispatch;

  datos_interrupt = malloc(sizeof(datos_conexion_t));
  datos_interrupt->ip = ip_kernel;
  datos_interrupt->puerto = puerto_kernel_interrupt;
  
  datos_memoria = malloc(sizeof(datos_conexion_t));
  datos_memoria->ip = ip_memoria;
  datos_memoria->puerto = puerto_memoria;
}

//Conexiones
void* conectar(void* arg) {
  datos_conexion_t* datos = (datos_conexion_t*) arg;
    int socket = crear_conexion(datos->ip, datos->puerto, CPU);
    if (socket == -1) {
        log_error(logger, "Fallo al conectar");
        pthread_exit(NULL);
    }

    int32_t handshake_header = CPU;
    int32_t respuesta;
    send(socket, &handshake_header, sizeof(int32_t), 0);
    recv(socket, &respuesta, sizeof(int32_t), MSG_WAITALL);

    if (respuesta <= 0) {
      log_error(logger, "Fallo al recibir respuesta del handshake");
      close(socket);
      pthread_exit(NULL);
    }

    log_info(logger, "Conexión exitosa");

    free(datos);

    return NULL;
}

void* manejar_dispatch(conexion_kernel_dispatch,conexion_memoria) {
  // Paso 1: recibir el PCB desde kernel
  t_pcb* pcb = recibir_pcb(conexion_kernel_dispatch);
  
  // Paso 2: obtener la dirección de memoria de la instrucción
  void* buffer = recibir_instruccion(pcb, conexion_memoria);
  // Paso 3: ejecutar instrucción
  //ciclo_de_instruccion(pcb);

  // Paso 5: liberar memoria (si es necesario)
  free(pcb);
  free(buffer);

  return NULL;
}

//Recibir información del PCB desde Kernel
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

//Solicitar instrucción a Memoria
void* recibir_instruccion(t_pcb* pcb, int conexion_memoria) {

  send(conexion_memoria, &(pcb->pc), sizeof(uint32_t), 0);

  t_instruccion instruccion_recibida;
  recv(conexion_memoria, &instruccion_recibida, sizeof(t_instruccion), 0);

  return NULL;
}




