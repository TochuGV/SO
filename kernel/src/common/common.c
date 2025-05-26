#include "common.h"

int conexion_cpu_dispatch;
int conexion_cpu_interrupt;
int conexion_io;

void* conectar_cpu_dispatch(void* arg){
  char* puerto = (char*) arg;
  int socket_escucha = iniciar_servidor(puerto);
  
  log_info(logger, "Esperando conexiones de CPU Dispatch...");

  while(1){
    int socket_cliente = esperar_cliente(socket_escucha);
    int* socket_ptr = malloc(sizeof(int));
    *socket_ptr = socket_cliente;
    pthread_t hilo_atender_cpu_dispatch;
    pthread_create(&hilo_atender_cpu_dispatch, NULL, atender_cpu_dispatch, socket_ptr);
    pthread_detach(hilo_atender_cpu_dispatch);
  }
  return NULL;
};

void* atender_cpu_dispatch(void* arg){
  int socket_cpu_dispatch = *(int*)arg;
  free(arg);

  if (recibir_handshake_kernel(socket_cpu_dispatch) != CPU) {
    log_error(logger, "Se esperaba un CPU, pero se conectó otro tipo");
    close(socket_cpu_dispatch);
    pthread_exit(NULL);
  };
    
  log_info(logger, "CPU Dispatch conectada correctamente");
  
  while (1) {
    int cod_op = recibir_operacion(socket_cpu_dispatch);
    if (cod_op == -1){
      log_warning(logger, "CPU Dispatch desconectada");
      break;
    };
    log_debug(logger, "Código de operación recibido: %d", cod_op);
  };

  close(socket_cpu_dispatch);
  return NULL;
};

void* conectar_io(void* arg){
  char* puerto = (char*) arg;
  int socket_io = iniciar_conexion(puerto);
  if (socket_io == -1)
  pthread_exit(NULL);

  pthread_t hilo_atender_io;
  pthread_create(&hilo_atender_io, NULL, atender_io, &socket_io);
  pthread_join(hilo_atender_io, NULL);

  return NULL;
};

void* atender_io(void* arg){
  int socket_io = *(int*)arg;
  
  if (recibir_handshake_kernel(socket_io) != IO) {
    log_error(logger, "Se esperaba un IO, pero se conectó otro tipo");
    close(socket_io);
    pthread_exit(NULL);
  };

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
          log_warning(logger, "Operación desconocida. No quieras meter la pata.");
          break;
        };
      };
      return NULL;
};

int recibir_handshake_kernel(int cliente_kernel){
  int32_t handshake;
  int32_t ok = 0;
  int32_t error = -1;

  if (recv(cliente_kernel, &handshake, sizeof(int32_t), MSG_WAITALL) <= 0){
    log_error(logger, "Error recibiendo el tipo de módulo en Handshake");
    return -1;
  };

  switch(handshake){
    case CPU:
      int32_t id_cpu;
      if (recv(cliente_kernel, &id_cpu, sizeof(int32_t), MSG_WAITALL) <= 0 || id_cpu <= 0){
        log_error(logger, "Handshake CPU inválido");
        send(cliente_kernel, &error, sizeof(int32_t), 0);
        return -1;
      };
      send(cliente_kernel, &ok, sizeof(int32_t), 0);
      log_debug(logger, "CPU %d conectada.", id_cpu);
      return CPU;
    case IO:
      int32_t token_io;
      if (recv(cliente_kernel, &token_io, sizeof(int32_t), MSG_WAITALL) <= 0){
        log_error(logger, "Handshake IO inválido");
        send(cliente_kernel, &error, sizeof(int32_t), 0);
        return -1;
      };

      /*
      char* nombre_io = tipo_io_to_string(tipo_io);
      if (nombre_io == NULL) {
      send(socket_cliente, &error, sizeof(int32_t), 0);
      log_warning(logger, "Tipo de IO desconocido");
      return -1;
      }
      */
      send(cliente_kernel, &ok, sizeof(int32_t), 0);
      //log_debug(logger, "Dispositivo %s conectado", nombre_io);
      return IO;
    default:
      send(cliente_kernel, &error, sizeof(int32_t), 0);
      log_warning(logger, "Tipo de módulo desconocido en Handshake: %d", handshake);
      break;
  };
  return -1;
};
/*
void* atender_cliente(void* arg){
  int cliente_kernel = *(int*)arg;
  pthread_t hilo_atender;
  int32_t cliente = recibir_handshake_kernel(cliente_kernel);

  if(cliente == IO){
    pthread_create(&hilo_atender, NULL, atender_io, arg);
    pthread_join(hilo_atender, NULL);
  } else if (cliente == CPU) {
    pthread_create(&hilo_atender, NULL, atender_cpu_dispatch, arg);
    pthread_join(hilo_atender, NULL);
  } else {
    log_warning(logger, "Handshake desconocido");
  }
  pthread_detach(pthread_self());
  return NULL;
};
*/
uint32_t enviar_proceso_a_memoria(char* archivo_pseudocodigo, uint32_t tamanio_proceso, uint32_t pid, int socket_cliente){
  t_paquete* paquete = crear_paquete(SOLICITUD_MEMORIA);
  uint32_t longitud_archivo_pseudocodigo;
  uint32_t resultado;
  if(archivo_pseudocodigo != NULL){
    longitud_archivo_pseudocodigo = strlen(archivo_pseudocodigo) + 1;
    agregar_a_paquete(paquete, &longitud_archivo_pseudocodigo, sizeof(uint32_t));// Enviar la longitud
    agregar_a_paquete(paquete, archivo_pseudocodigo, longitud_archivo_pseudocodigo);
    agregar_a_paquete(paquete, &tamanio_proceso, sizeof(tamanio_proceso));
    agregar_a_paquete(paquete, &pid, sizeof(uint32_t));

    enviar_paquete(paquete, socket_cliente);

    log_info(logger, "Archivo pseudocódigo enviado a memoria!");

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

/*
char* tipo_io_to_string(int tipo) {
    switch (tipo) {
        case IMPRESORA: return "Impresora";
        case TECLADO: return "Teclado";
        case MOUSE: return "Mouse";
        case AURICULARES: return "Auriculares";
        case PARLANTE: return "Parlante";
        default: return NULL;
    }
}
*/