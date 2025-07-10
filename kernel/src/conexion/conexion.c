#include "conexion.h"

void aceptar_conexiones(char* puerto, void* (*atender_cliente)(void*), char* nombre_modulo){
  int socket_escucha = iniciar_servidor(puerto);
  if(socket_escucha == -1) pthread_exit(NULL);
  log_debug(logger, "Esperando conexiones de %s...", nombre_modulo);
  while(1){
    int socket_cliente = esperar_cliente(socket_escucha);
    if(socket_cliente == -1){
      log_error(logger, "Error al aceptar conexión desde %s", nombre_modulo);
      continue;
    };
    int* socket_ptr = malloc(sizeof(int));
    *socket_ptr = socket_cliente;
    pthread_t hilo_atencion;
    pthread_create(&hilo_atencion, NULL, atender_cliente, socket_ptr);
    pthread_detach(hilo_atencion);
  };
};

bool validar_handshake_cliente(int socket, int modulo_esperado, char* nombre_modulo){
  if(recibir_handshake_kernel(socket) != modulo_esperado){
    log_error(logger, "Se esperaba un módulo %s, pero se conectó otro tipo", nombre_modulo);
    close(socket);
    return false;
  };
  log_debug(logger, "%s conectado correctamente", nombre_modulo);
  return true;
};