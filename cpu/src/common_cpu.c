#include "common_cpu.h"
//Conexiones
void* atender_cpu_dispatch(void* arg)
{
  int socket_cpu_dispatch = *(int*)arg;
  if (recibir_handshake_de(CPU_DISPATCH,socket_cpu_dispatch) == -1) {
    pthread_exit((void*)EXIT_FAILURE);
  } else {
    
  t_list* lista;

    while (1) {
      int cod_op = recibir_operacion(socket_cpu_dispatch);
      switch (cod_op) {
      case PAQUETE:
        lista = recibir_paquete(socket_cpu_dispatch);
        list_iterate(lista, (void*) iterator);
        break;
      case -1:
        log_error(logger, "El cliente se desconectó. Terminando servidor...");
        pthread_exit((void*)EXIT_FAILURE);
      default:
        log_warning(logger,"Operación desconocida. No quieras meter la pata.");
        break;
      }
    }
    return NULL;
  }
}

void* atender_cpu_interrupt(void* arg)
{
  int socket_cpu_interrupt = *(int*)arg;
  if (recibir_handshake_de(CPU_INTERRUPT,socket_cpu_interrupt) == -1) {
    pthread_exit((void*)EXIT_FAILURE);
  } else {
    
  t_list* lista;

    while (1) {
      int cod_op = recibir_operacion(socket_cpu_interrupt);
      switch (cod_op) {
      case PAQUETE:
        lista = recibir_paquete(socket_cpu_interrupt);
        list_iterate(lista, (void*) iterator);
        break;
      case -1:
        log_error(logger, "El cliente se desconectó. Terminando servidor...");
        pthread_exit((void*)EXIT_FAILURE);
      default:
        log_warning(logger,"Operación desconocida. No quieras meter la pata.");
        break;
      }
    }
    return NULL;
  }
}

void* atender_memoria(void* arg)
{
  int socket_memoria = *(int*)arg;
  if (recibir_handshake_de(MEMORIA,socket_memoria) == -1) {
    pthread_exit((void*)EXIT_FAILURE);
  } else {
    
  t_list* lista;

    while (1) {
      int cod_op = recibir_operacion(socket_memoria);
      switch (cod_op) {
      case PAQUETE:
        lista = recibir_paquete(socket_memoria);
        list_iterate(lista, (void*) iterator);
        break;
      case -1:
        log_error(logger, "El cliente se desconectó. Terminando servidor...");
        pthread_exit((void*)EXIT_FAILURE);
      default:
        log_warning(logger,"Operación desconocida. No quieras meter la pata.");
        break;
      }
    }
    return NULL;
  }
}

void* conectar_kernel_dispatch(void* arg) 
{
  char* puerto = (char*) arg;
  int socket_cpu_dispatch = iniciar_conexion(puerto);
  if (socket_cpu_dispatch == -1)
  pthread_exit(NULL);

  pthread_t hilo_atender_cpu_dispatch;
  pthread_create(&hilo_atender_cpu_dispatch, NULL, atender_cpu_dispatch, &socket_cpu_dispatch);
  pthread_join(hilo_atender_cpu_dispatch,NULL);

  return NULL;
}

void* conectar_kernel_interrupt(void* arg) 
{
  char* puerto = (char*) arg;
  int socket_cpu_interrupt= iniciar_conexion(puerto);
  if (socket_cpu_interrupt== -1)
  pthread_exit(NULL);

  pthread_t hilo_atender_cpu_dispatch;
  pthread_create(&hilo_atender_cpu_dispatch, NULL, atender_cpu_dispatch, &socket_cpu_interrupt);
  pthread_join(hilo_atender_cpu_dispatch,NULL);

  return NULL;
}

void* conectar_memoria(void* arg) 
{
  char* puerto = (char*) arg;
  int socket_memoria = iniciar_conexion(puerto);
  if (socket_memoria == -1)
  pthread_exit(NULL);

  pthread_t hilo_atender_cpu_dispatch;
  pthread_create(&hilo_atender_cpu_dispatch, NULL, atender_cpu_dispatch, &socket_memoria);
  pthread_join(hilo_atender_cpu_dispatch,NULL);

  return NULL;
}



//Handshakes
int32_t handshake_kernel_dispatch(int32_t identificador, int conexion_kernel_dispatch) {
  int32_t resultado;

  send(conexion_kernel_dispatch, &identificador, sizeof(int32_t), 0);
  recv(conexion_kernel_dispatch, &resultado, sizeof(int32_t), MSG_WAITALL);

  if(resultado == -1){
    log_error(logger, "Error: La conexión con Kernel Dispatch falló. Finalizando conexión...");
    return -1;
  }
  else {
    log_info(logger,"Conexion con Kernel Dispatch exitosa!");
    return 0;
  }
}

int32_t handshake_kernel_interrupt(int32_t identificador, int conexion_kernel_itnerrupt) {
  int32_t resultado;

  send(conexion_kernel_itnerrupt, &identificador, sizeof(int32_t), 0);
  recv(conexion_kernel_itnerrupt, &resultado, sizeof(int32_t), MSG_WAITALL);

  if(resultado == -1){
    log_error(logger, "Error: La conexión con Kernel Interrupt falló. Finalizando conexión...");
    return -1;
  }
  else {
    log_info(logger,"Conexion con Kernel Interrupt exitosa!");
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
}#include "common_cpu.h"
//Conexiones
void* atender_cpu_dispatch(void* arg)
{
  int socket_cpu_dispatch = *(int*)arg;
  if (recibir_handshake_de(CPU_DISPATCH,socket_cpu_dispatch) == -1) {
    pthread_exit((void*)EXIT_FAILURE);
  } else {
    
  t_list* lista;

    while (1) {
      int cod_op = recibir_operacion(socket_cpu_dispatch);
      switch (cod_op) {
      case PAQUETE:
        lista = recibir_paquete(socket_cpu_dispatch);
        list_iterate(lista, (void*) iterator);
        break;
      case -1:
        log_error(logger, "El cliente se desconectó. Terminando servidor...");
        pthread_exit((void*)EXIT_FAILURE);
      default:
        log_warning(logger,"Operación desconocida. No quieras meter la pata.");
        break;
      }
    }
    return NULL;
  }
}

void* atender_cpu_interrupt(void* arg)
{
  int socket_cpu_interrupt = *(int*)arg;
  if (recibir_handshake_de(CPU_INTERRUPT,socket_cpu_interrupt) == -1) {
    pthread_exit((void*)EXIT_FAILURE);
  } else {
    
  t_list* lista;

    while (1) {
      int cod_op = recibir_operacion(socket_cpu_interrupt);
      switch (cod_op) {
      case PAQUETE:
        lista = recibir_paquete(socket_cpu_interrupt);
        list_iterate(lista, (void*) iterator);
        break;
      case -1:
        log_error(logger, "El cliente se desconectó. Terminando servidor...");
        pthread_exit((void*)EXIT_FAILURE);
      default:
        log_warning(logger,"Operación desconocida. No quieras meter la pata.");
        break;
      }
    }
    return NULL;
  }
}

void* atender_memoria(void* arg)
{
  int socket_memoria = *(int*)arg;
  if (recibir_handshake_de(MEMORIA,socket_memoria) == -1) {
    pthread_exit((void*)EXIT_FAILURE);
  } else {
    
  t_list* lista;

    while (1) {
      int cod_op = recibir_operacion(socket_memoria);
      switch (cod_op) {
      case PAQUETE:
        lista = recibir_paquete(socket_memoria);
        list_iterate(lista, (void*) iterator);
        break;
      case -1:
        log_error(logger, "El cliente se desconectó. Terminando servidor...");
        pthread_exit((void*)EXIT_FAILURE);
      default:
        log_warning(logger,"Operación desconocida. No quieras meter la pata.");
        break;
      }
    }
    return NULL;
  }
}

void* conectar_kernel_dispatch(void* arg) 
{
  char* puerto = (char*) arg;
  int socket_cpu_dispatch = iniciar_conexion(puerto);
  if (socket_cpu_dispatch == -1)
  pthread_exit(NULL);

  pthread_t hilo_atender_cpu_dispatch;
  pthread_create(&hilo_atender_cpu_dispatch, NULL, atender_cpu_dispatch, &socket_cpu_dispatch);
  pthread_join(hilo_atender_cpu_dispatch,NULL);

  return NULL;
}

void* conectar_kernel_interrupt(void* arg) 
{
  char* puerto = (char*) arg;
  int socket_cpu_interrupt= iniciar_conexion(puerto);
  if (socket_cpu_interrupt== -1)
  pthread_exit(NULL);

  pthread_t hilo_atender_cpu_dispatch;
  pthread_create(&hilo_atender_cpu_dispatch, NULL, atender_cpu_dispatch, &socket_cpu_interrupt);
  pthread_join(hilo_atender_cpu_dispatch,NULL);

  return NULL;
}

void* conectar_memoria(void* arg) 
{
  char* puerto = (char*) arg;
  int socket_memoria = iniciar_conexion(puerto);
  if (socket_memoria == -1)
  pthread_exit(NULL);

  pthread_t hilo_atender_cpu_dispatch;
  pthread_create(&hilo_atender_cpu_dispatch, NULL, atender_cpu_dispatch, &socket_memoria);
  pthread_join(hilo_atender_cpu_dispatch,NULL);

  return NULL;
}



//Handshakes
int32_t handshake_kernel_dispatch(int32_t identificador, int conexion_kernel_dispatch) {
  int32_t resultado;

  send(conexion_kernel_dispatch, &identificador, sizeof(int32_t), 0);
  recv(conexion_kernel_dispatch, &resultado, sizeof(int32_t), MSG_WAITALL);

  if(resultado == -1){
    log_error(logger, "Error: La conexión con Kernel Dispatch falló. Finalizando conexión...");
    return -1;
  }
  else {
    log_info(logger,"Conexion con Kernel Dispatch exitosa!");
    return 0;
  }
}

int32_t handshake_kernel_interrupt(int32_t identificador, int conexion_kernel_itnerrupt) {
  int32_t resultado;

  send(conexion_kernel_itnerrupt, &identificador, sizeof(int32_t), 0);
  recv(conexion_kernel_itnerrupt, &resultado, sizeof(int32_t), MSG_WAITALL);

  if(resultado == -1){
    log_error(logger, "Error: La conexión con Kernel Interrupt falló. Finalizando conexión...");
    return -1;
  }
  else {
    log_info(logger,"Conexion con Kernel Interrupt exitosa!");
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
