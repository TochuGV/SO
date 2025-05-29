#include "common.h"

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
  int socket_cpu_interrupt = *(int*)arg;
  free(arg);

  if (recibir_handshake_kernel(socket_cpu_interrupt) != CPU) {
    log_error(logger, "Se esperaba un CPU, pero se conectó otro tipo");
    close(socket_cpu_interrupt);
    pthread_exit(NULL);
  };
    
  log_info(logger, "CPU Interrupt conectada correctamente");
  
  while (1) {
    int cod_op = recibir_operacion(socket_cpu_interrupt);
    if (cod_op == -1){
      log_warning(logger, "CPU Interrupt desconectada");
      break;
    };
    log_debug(logger, "Se recibió la interrupción. Código: %d", cod_op);

  };

  close(socket_cpu_interrupt);
  return NULL;
};

void* conectar_io(void* arg){
  char* puerto = (char*) arg;
  int socket_escucha = iniciar_servidor(puerto);
  if (socket_escucha == -1) {
    pthread_exit(NULL);
  };

  log_info(logger, "Esperando conexiones de IO...");

  while(1){
    int socket_cliente = esperar_cliente(socket_escucha);
    if(socket_cliente == -1){
      log_error(logger, "Error al aceptar conexión desde dispositivo IO");
      continue;
    };
    int* socket_ptr = malloc(sizeof(int));
    *socket_ptr = socket_cliente;
    pthread_t hilo_atender_io;
    pthread_create(&hilo_atender_io, NULL, atender_io, socket_ptr);
    pthread_detach(hilo_atender_io);
  };
  return NULL;
};

void* atender_io(void* arg){
  int socket_io = *(int*)arg;
  free(arg);

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
        log_warning(logger, "Dispositivo IO desconectado");
        close(socket_io);
        pthread_exit(NULL);
      default:
        log_warning(logger, "Operación desconocida desde IO: %d", cod_op);
        break;
    };
  };

  close(socket_io);
  pthread_exit(NULL);
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
      int32_t tipo_conexion; //Dispatch = 0 | Interrupt = 1


      if (recv(cliente_kernel, &id_cpu, sizeof(int32_t), MSG_WAITALL) <= 0 || 
        recv(cliente_kernel, &tipo_conexion, sizeof(int32_t), MSG_WAITALL) <= 0 ||
        id_cpu <= 0 || (tipo_conexion != CPU_DISPATCH && tipo_conexion != CPU_INTERRUPT)){
        log_error(logger, "Handshake CPU inválido");
        send(cliente_kernel, &error, sizeof(int32_t), 0);
        return -1;
      };

      bool misma_cpu(void* elem){
        return ((t_cpu*)elem)->id_cpu == id_cpu;
      };

      t_cpu* cpu = list_find(lista_cpus, misma_cpu);
      if(cpu == NULL){
        cpu = malloc(sizeof(t_cpu));
        cpu->id_cpu = id_cpu;
        cpu->socket_dispatch = -1;
        cpu->socket_interrupt = -1;
        cpu->disponible = true;
        list_add(lista_cpus, cpu);
      };

      if(tipo_conexion == CPU_DISPATCH){
        if(cpu->socket_dispatch != -1){
          log_error(logger, "CPU %d ya tiene conexión Dispatch", id_cpu);
          send(cliente_kernel, &error, sizeof(int32_t), 0);
          return -1;
        };
        cpu->socket_dispatch = cliente_kernel;
        log_info(logger, "CPU %d conectó Dispatch", id_cpu);
      } else {
        if(cpu->socket_interrupt != -1){
          log_error(logger, "CPU %d ya tiene una conexión Interrupt", id_cpu);
          send(cliente_kernel, &error, sizeof(int32_t), 0);
          return -1;
        };
        cpu->socket_interrupt = cliente_kernel;
        log_info(logger, "CPU %d conectó Interrupt", id_cpu);
      };
      
      send(cliente_kernel, &ok, sizeof(int32_t), 0);
      //log_info(logger, "CPU %d conectada.", id_cpu); //Agregar validación para cuando se conecten Dispatch e Interrupt
      return CPU;
    case IO:
      log_debug(logger, "Handshake recibido: IO");
      int32_t token_io;
      if (recv(cliente_kernel, &token_io, sizeof(int32_t), MSG_WAITALL) <= 0){
        log_error(logger, "Handshake IO inválido");
        send(cliente_kernel, &error, sizeof(int32_t), 0);
        return -1;
      };
      log_debug(logger, "Token IO recibido: %d", token_io);
      char* nombre_io = token_io_to_string(token_io);
      if (nombre_io == NULL) {
        log_warning(logger, "Tipo de IO desconocido");
        send(cliente_kernel, &error, sizeof(int32_t), 0);
        return -1;
      };
      send(cliente_kernel, &ok, sizeof(int32_t), 0);
      log_info(logger, "Dispositivo '%s' conectado", nombre_io);
      return IO;
    default:
      send(cliente_kernel, &error, sizeof(int32_t), 0);
      log_warning(logger, "Tipo de módulo desconocido en Handshake: %d", handshake);
      break;
  };
  return -1;
};

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
  log_info(logger, "Kernel se conectó exitosamente a Memoria");
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

char* token_io_to_string(int32_t token) {
  switch (token) {
    case IMPRESORA: return "Impresora";
    case TECLADO: return "Teclado";
    case MOUSE: return "Mouse";
    case AURICULARES: return "Auriculares";
    case PARLANTE: return "Parlante";
    default: return NULL;
  };
};
