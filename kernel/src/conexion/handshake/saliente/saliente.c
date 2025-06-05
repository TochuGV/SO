#include "saliente.h"

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