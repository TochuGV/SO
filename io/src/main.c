#include "main.h"

int main(int argc, char* argv[]){
  inicializar_io();

  char* nombre_io = argv[1];
  int conexion_kernel = crear_conexion(IP_KERNEL, PUERTO_KERNEL, IO);

  if (handshake_io(nombre_io, conexion_kernel) == 0) {
    atender_interrupcion(conexion_kernel);
  };

  terminar_programa(conexion_kernel, logger, config);
  return EXIT_SUCCESS;
};

