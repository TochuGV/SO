#include "io.h"

void* conectar_io(void* arg){
  char* puerto = (char*) arg;
  int socket_escucha = iniciar_servidor(puerto);
  if (socket_escucha == -1){
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
  if (recibir_handshake_kernel(socket_io) != MODULO_IO){
    log_error(logger, "Se esperaba un IO, pero se conectó otro tipo");
    close(socket_io);
    pthread_exit(NULL);
  };
  t_list* lista;
  while (1) {
    int cod_op = recibir_operacion(socket_io);
    switch (cod_op){
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

void registrar_socket_io(char* nombre, int socket){
  if(!diccionario_dispositivos){
    log_error(logger, "El diccionario de dispositivos IO no está inicializado.");
    return;
  };
  t_dispositivo_io* dispositivo = dictionary_get(diccionario_dispositivos, nombre);
  if(!dispositivo){
    log_error(logger, "No existe el dispositivo IO: %s", nombre); //Cambiar texto
    return;
  };
  dispositivo->socket = socket;
  //log_info(logger, "Socket IO registrado para <%s>", nombre);
};

void enviar_peticion_io(int socket_io, uint32_t duracion, uint32_t pid){
  t_paquete* paquete = crear_paquete(PETICION_IO);
  agregar_a_paquete(paquete, &pid, sizeof(uint32_t));
  agregar_a_paquete(paquete, &duracion, sizeof(uint32_t));
  enviar_paquete(paquete, socket_io);
  eliminar_paquete(paquete);
};