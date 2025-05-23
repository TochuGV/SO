#include "common.h"

int conexion_cpu_dispatch;
int conexion_cpu_interrupt;
int conexion_io;

void* conectar_cpu_dispatch(void* arg){
  char* puerto = (char*) arg;
  int socket_cpu_dispatch = iniciar_conexion(puerto);
  if (socket_cpu_dispatch == -1)
  pthread_exit(NULL);

  pthread_t hilo_atender_cpu_dispatch;
  pthread_create(&hilo_atender_cpu_dispatch, NULL, atender_cpu_dispatch, &socket_cpu_dispatch);
  pthread_join(hilo_atender_cpu_dispatch, NULL);

  return NULL;
};

void* conectar_io(void* arg){
  char* puerto = (char*) arg;
  int socket_io = iniciar_conexion(puerto);
  if (socket_io == -1)
  pthread_exit(NULL);

  pthread_t hilo_atender_io;
  pthread_create(&hilo_atender_io, NULL, atender_io, &socket_io);
  pthread_join(hilo_atender_io,NULL);

  return NULL;
};

void* atender_cpu_dispatch(void* arg){
  int socket_cpu_dispatch = *(int*)arg;
  if (recibir_handshake_de(CPU,socket_cpu_dispatch) == -1) {
    pthread_exit((void*)EXIT_FAILURE);
  } else {
    t_list* lista;
      while (1) {
        int cod_op = recibir_operacion(socket_cpu_dispatch);
        switch (cod_op) {

        case SYSCALL_EXIT:
          // finalizar proceso
          break;
        case SYSCALL_IO:
          // llamar a io
          break;
        case SYSCALL_INIT_PROC:
          // crear un proceso
          break;
        case SYSCALL_DUMP_MEMORY:
          // por ahora nada
          break;
        case -1:
          log_error(logger, "El cliente se desconectó. Terminando servidor...");
          pthread_exit((void*)EXIT_FAILURE);
        default:
          log_warning(logger,"Operación desconocida. No quieras meter la pata.");
          break;
        };
      };
    return NULL;
  };
};

void* atender_io(void* arg){
  int socket_io = *(int*)arg;
  if (recibir_handshake_de(IO, socket_io) == -1) {
    pthread_exit((void*)EXIT_FAILURE);
  } else {
    t_list* lista;
      while (1) {
        int cod_op = recibir_operacion(socket_io);
        switch (cod_op) {
        case PAQUETE:
          lista = recibir_paquete(socket_io);
          list_iterate(lista, (void*) iterator);
          break;
        case -1:
          log_error(logger, "El cliente se desconectó. Terminando servidor...");
          pthread_exit((void*)EXIT_FAILURE);
        default:
          log_warning(logger,"Operación desconocida. No quieras meter la pata.");
          break;
        };
      };
      return NULL;
    };
};

int recibir_handshake_kernel(int cliente_kernel){
  int32_t handshake;
  int32_t resultado_ok = 0;
  int32_t resultado_error = -1;

  recv(cliente_kernel, &handshake, sizeof(int32_t), MSG_WAITALL);

  switch(handshake){
    case IO:
      int32_t token_io;
      send(cliente_kernel, &resultado_ok, sizeof(int32_t), 0);
      recv(cliente_kernel, &token_io, sizeof(int32_t), MSG_WAITALL);
      return IO;
    case CPU:
      int32_t identificador_cpu;
      send(cliente_memoria, &resultado_ok, sizeof(int32_t), 0);
      recv(cliente_memoria, &identificador_cpu, sizeof(int32_t),MSG_WAITALL);
      log_debug(logger, "CPU %d conectada.", identificador_cpu);
      return CPU;
    default:
      send(cliente_kernel, &resultado_error, sizeof(int32_t), 0);
      break;
  }
  return -1;
}

void* atender_cliente(void* arg){
  int cliente_kernel = *(int*)arg;
  pthread_t hilo_atender;
  int32_t cliente = recibir_handshake_kernel(cliente_kernel);

  if(cliente == IO){
    pthread_create(&hilo_atender, NULL, atender_io, arg);
    pthread_join(hilo_atender);
  } else if (cliente == CPU) {
    pthread_create(&hilo_atender, NULL, atender_cpu_dispatch, arg);
    pthread_join(hilo_atender, NULL);
  } else {
    log_warning(logger, "Handshake desconocido");
  }
  pthread_detach(pthread_self());
  return NULL;
};

void* enviar_proceso_a_memoria(char* archivo_pseudocodigo, uint32_t tamanio_proceso, uint32_t pid, int socket_cliente){
  t_paquete* paquete = crear_paquete(SOLICITUD_MEMORIA);
  uint32_t longitud_archivo_pseudocodigo;
  uint32_t resultado;
  if(archivo_pseudocodigo != NULL){
    longitud_archivo_pseudocodigo = strlen(archivo_pseudocodigo) + 1;
    agregar_a_paquete(paquete, &longitud_archivo_pseudocodigo, sizeof(uint32_t));// Enviar la longitud
    agregar_a_paquete(paquete, archivo_pseudocodigo, longitud_archivo_pseudocodigo);            // Enviar el contenido del path
    agregar_a_paquete(paquete, &tamanio_proceso, sizeof(tamanio_proceso));
    agregar_a_paquete(paquete, &pid, sizeof(uint32_t));

    enviar_paquete(paquete, socket_cliente);

    log_info(logger, "Archivo pseudocodigo enviado a memoria!");

    recv(socket_cliente, &resultado, sizeof(uint32_t), MSG_WAITALL);

    if (resultado == 0)
      return 0;
    else
      return -1;

  } else {
    longitud_archivo_pseudocodigo = 0;
    agregar_a_paquete(paquete, &longitud_archivo_pseudocodigo, sizeof(uint32_t));
    log_warning(logger, "Archivo pseudocodigo es NULL, temporalmente. Se envía como longitud '0'");
    enviar_paquete(paquete, socket_cliente);
  };
  return -1;
};

int32_t handshake_kernel(int conexion_memoria){
  int32_t handshake = KERNEL;
  int32_t resultado;

  log_info(logger, "Enviando handshake a Memoria...");
  send(conexion_memoria, &handshake, sizeof(int32_t), 0);
  log_info(logger, "Esperando respuesta de Memoria...");
  recv(conexion_memoria, &resultado, sizeof(int32_t), 0);
  log_info(logger, "Respuesta recibida de handshake");
  if (resultado == -1) {
    log_error(logger, "Error: La conexión con Memoria falló. Finalizando conexión...");
    return -1;
  };
  return 0;
};

char* crear_cadena_metricas_estado(t_pcb* pcb){
  char* buffer = string_from_format("## (<%d>) - Métricas de estado: ", pcb->pid);
  for(int i = 0; i < CANTIDAD_ESTADOS; i++){
    char* aux = string_from_format("%s (%d) (%d)", obtener_nombre_estado(i), pcb->me[i], pcb->mt[i]);
    string_append(&buffer, aux);
    if(i < CANTIDAD_ESTADOS - 1) string_append(&buffer, ", ");
    free(aux);
  };
  return buffer;
};

char* NOMBRES_SYSCALLS[] = {
  "IO",
  "EXIT"
  "INIT_PROC",
  "DUMP_MEMORY",
};

char* NOMBRES_DISPOSITIVOS_IO[] = {
  "IMPRESORA",
  "TECLADO",
  "MOUSE",
  "AURICULARES",
  "PARLANTE"
};