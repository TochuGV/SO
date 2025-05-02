#include "common_io.h"

int32_t handshake_io(char* nombre, int conexion_kernel)
{
  int32_t token_io;
  int32_t resultado;

  if (strcasecmp(nombre, "impresora") == 0)
    token_io = IMPRESORA;
  else if (strcasecmp(nombre, "teclado") == 0)
    token_io = TECLADO;
  else if (strcasecmp(nombre, "mouse") == 0)
    token_io = MOUSE;
  else if (strcasecmp(nombre, "auriculares") == 0)
    token_io = AURICULARES;
  else if (strcasecmp(nombre, "parlante") == 0)
    token_io = PARLANTE;

  send(conexion_kernel, &token_io, sizeof(int32_t), 0);
  recv(conexion_kernel, &resultado, sizeof(int32_t), MSG_WAITALL);

  if(resultado == -1) {
    log_error(logger, "Error: La conexión con Kernel falló. Finalizando conexión...");
    return -1;
  }
  else {
    log_info(logger," %s se ha conectado a Kernel exitosamente!",nombre);
    return 0;
  }
}
/*
void atender_interrupcion(int conexion_kernel)
{
  int32_t interrupcion; //esto debe ser un struct con nombre de dispositivo y cantidad de ms
  recv(conexion_kernel, &interrupcion, sizeof(int32_t), MSG_WAITALL);
  // ...
}*/