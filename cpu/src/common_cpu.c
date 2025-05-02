#include "common_cpu.h"

int32_t handshake_cpu(int32_t identificador, int conexion_cpu){
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