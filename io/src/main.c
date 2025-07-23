#include "main.h"

int main(int argc, char* argv[]){
  inicializar_io();

  int conexion_kernel = crear_conexion(IP_KERNEL, PUERTO_KERNEL, IO);

  if(handshake_io(argv[1], conexion_kernel) == 0){
    while(1){
      int cod_op = recibir_operacion(conexion_kernel);
      if (cod_op == PETICION_IO) {
        atender_interrupcion(conexion_kernel);
      } else if(cod_op == -1){
        log_warning(logger, "Se perdió la conexión con Kernel. Finalizando IO...");
        break;
      } else {
        log_warning(logger, "Código de operación desconocido: %d", cod_op);
      };
    };
  };

  terminar_programa(conexion_kernel, logger, config); //Tiene sentido esto???
  return EXIT_SUCCESS;
};