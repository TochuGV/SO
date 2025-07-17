#include "saliente.h"

int32_t handshake_kernel(int conexion_memoria){
  int32_t handshake = MODULO_KERNEL;
  int32_t resultado;
  send(conexion_memoria, &handshake, sizeof(int32_t), 0);
  recv(conexion_memoria, &resultado, sizeof(int32_t), 0);
  if (resultado == -1) {
    log_error(logger, "Error: La conexión con Memoria falló. Finalizando conexión...");
    return -1;
  };
  return 0;
};